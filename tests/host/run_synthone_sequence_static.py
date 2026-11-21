#!/usr/bin/env python3
"""Instrument actual static validation calls; no timing or app qualification."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--fallback-smoothed-sequence', action='store_true', help='Synthetic valid future port map must retain per-sample checks')
    args = parser.parse_args()
    repo, out = Path(__file__).resolve().parents[2], args.output.resolve()
    source_bytes = (repo / 'tests/host/synthone_sequence_static_test.cpp').read_bytes()
    runner_bytes = Path(__file__).read_bytes()
    tool_paths = [repo / 'tests/host' / name for name in ('run_synthone_renderer.py', 'run_synthone_factory.py')]
    tool_bytes = {p: p.read_bytes() for p in tool_paths}
    subprocess.run([sys.executable, tool_paths[0], '--source-build', args.source_build.resolve(), '--output', out], check=True)
    for p, data in tool_bytes.items():
        if p.read_bytes() != data: raise ValueError('orchestration changed')
    import types
    helper = types.ModuleType('factory'); helper.__file__ = str(tool_paths[1])
    exec(compile(tool_bytes[tool_paths[1]], helper.__file__, 'exec'), helper.__dict__)
    allowed = {}
    for manifest, directory in [('TESTED-SOURCES.json', 'closure'), ('ADAPTER-SOURCES.json', 'adapter-inputs')]:
        for name, digest in json.loads((out / manifest).read_bytes()).items():
            allowed[out / directory / helper.relative(name)] = digest
    source = out / 'sequence-static-test.cpp'; source.write_bytes(source_bytes)
    allowed[source] = hashlib.sha256(source_bytes).hexdigest()
    runner_path = out / 'sequence-static-runner.py'; runner_path.write_bytes(runner_bytes)
    allowed[runner_path] = hashlib.sha256(runner_bytes).hexdigest()
    for p, data in tool_bytes.items():
        staged = out / ('orchestration-' + p.name); staged.write_bytes(data)
        allowed[staged] = hashlib.sha256(data).hexdigest()
    for name, size, digest in [('public-morphs.f32', 852176, '0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5'),
                               ('compact.bin', 165292, 'a5cf8182b8bc9ca0a1b034a6b63f55d2cdc1087df38d6b1433da4fb56e5eccd5')]:
        path = args.source_build.resolve() / name; data = path.read_bytes()
        if len(data) != size or hashlib.sha256(data).hexdigest() != digest: raise ValueError('asset identity mismatch')
        allowed[path] = digest
    commands = json.loads((out / 'commands.json').read_bytes())
    scheduler = [c for c in commands if '-c' in c and c[c.index('-c') + 1].endswith('/source_scheduler.cpp')]
    harness = [c for c in commands if '-c' in c and c[c.index('-c') + 1].endswith('/synthone_renderer_test.cpp')]
    if len(scheduler) != 1 or len(harness) != 1: raise ValueError('ambiguous compile inputs')
    replacements = {}; new_commands = []
    for index, original in enumerate((scheduler[0], harness[0])):
        command = list(original); old = command[command.index('-o') + 1]
        obj, dep = out / ('instrumented-' + str(index) + '.o'), out / ('instrumented-' + str(index) + '.d')
        replacements[old] = str(obj); command[command.index('-o') + 1] = str(obj)
        command[command.index('-MF') + 1] = str(dep)
        if index == 0: command.insert(1, '-finstrument-functions')
        else: command[command.index('-c') + 1] = str(source)
        helper.verify_files(allowed); subprocess.run(command, check=True)
        helper.verify_dependencies(dep, allowed); new_commands.append(command)
    if args.fallback_smoothed_sequence:
        maps = [c for c in commands if '-c' in c and c[c.index('-c') + 1].endswith('/generated/parameter_map.cpp')]
        if len(maps) != 1: raise ValueError('expected one port map compile')
        command = list(maps[0]); original_source = Path(command[command.index('-c') + 1])
        text = original_source.read_text()
        import re
        indices = re.search(r'selected_indices\[selected_count\] = \{([^}]+)\}', text)
        slots = re.search(r'selected_slots\[parameter_count\] = \{([^}]+)\}', text)
        if not indices or not slots: raise ValueError('port map shape changed')
        ids = [int(x) for x in indices.group(1).split(',')]
        mapping = [int(x) for x in slots.group(1).split(',')]
        if len(ids) != 46 or len(mapping) != 150 or ids[0] != 0 or mapping[67] != 255:
            raise ValueError('unexpected synthetic-map base')
        # Reassign one existing smoothing slot to sequence pattern00 (ID67).
        # This is a test-only alternative map, never a production edit.
        ids[0] = 67; mapping[0] = 255; mapping[67] = 0
        text = text[:slots.start(1)] + ','.join(map(str, mapping)) + text[slots.end(1):]
        text = text[:indices.start(1)] + ','.join(map(str, ids)) + text[indices.end(1):]
        synthetic = out / 'synthetic-parameter-map.cpp'; synthetic.write_text(text)
        allowed[synthetic] = hashlib.sha256(synthetic.read_bytes()).hexdigest()
        old_obj = command[command.index('-o') + 1]; obj = out / 'synthetic-parameter-map.o'; dep = out / 'synthetic-parameter-map.d'
        replacements[old_obj] = str(obj)
        command[command.index('-c') + 1] = str(synthetic)
        command[command.index('-o') + 1] = str(obj); command[command.index('-MF') + 1] = str(dep)
        command.insert(1, '-I' + str(original_source.parent))
        subprocess.run(command, check=True); helper.verify_dependencies(dep, allowed)
        new_commands.append(command)
    link = [replacements.get(x, x) for x in commands[-1]]
    link[link.index('-o') + 1] = str(out / 'sequence-static-test'); subprocess.run(link, check=True)
    helper.verify_files(allowed)
    new_commands.append(link); (out / 'SEQUENCE-STATIC-COMMANDS.json').write_text(json.dumps(new_commands, indent=2) + '\n')
    executable_hash = hashlib.sha256((out / 'sequence-static-test').read_bytes()).hexdigest()
    manifest = {str(p.relative_to(out)) if p.is_relative_to(out) else 'assets/' + p.name: h for p,h in sorted(allowed.items())}
    (out / 'SEQUENCE-STATIC-INPUTS.json').write_text(json.dumps(manifest, indent=2) + '\n')
    run = subprocess.run([str(out / 'sequence-static-test'), str(args.source_build.resolve() / 'public-morphs.f32'),
                          str(args.source_build.resolve() / 'compact.bin'), '129' if args.fallback_smoothed_sequence else '1'], text=True, capture_output=True)
    (out / 'sequence-static.log').write_text(run.stdout + run.stderr)
    helper.verify_files(allowed)
    for p, data in tool_bytes.items():
        if p.read_bytes() != data: raise ValueError('orchestration changed')
    if hashlib.sha256((out / 'sequence-static-test').read_bytes()).hexdigest() != executable_hash:
        raise ValueError('instrumented executable changed')
    (out / 'SEQUENCE-STATIC-RESULT.json').write_text(json.dumps({
        'scope': 'Actual instrumented static validation count and boundary behavior only, not timing qualification',
        'synthetic_smoothed_map': args.fallback_smoothed_sequence, 'expected_calls_per_128_frames': 129 if args.fallback_smoothed_sequence else 1,
        'process_exit': run.returncode, 'executable_sha256': executable_hash,
        'inputs_sha256': hashlib.sha256((out / 'SEQUENCE-STATIC-INPUTS.json').read_bytes()).hexdigest(),
        'log_sha256': hashlib.sha256((out / 'sequence-static.log').read_bytes()).hexdigest()
    }, indent=2) + '\n')
    print(run.stdout + run.stderr)
    raise SystemExit(run.returncode)


if __name__ == '__main__': main()
