#!/usr/bin/env python3
"""Fresh source-bound host callback deadlines; no device XRUN/app claim."""
import argparse
import csv
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import subprocess
import sys


def sha(data):
    return hashlib.sha256(data).hexdigest()


def summarize(csv_path, programs):
    totals = {'callbacks': 0, 'frames': 0, 'wall_misses': 0, 'cpu_misses': 0,
              'max_wall_ns': 0, 'max_cpu_ns': 0}
    with csv_path.open() as stream:
        for index, row in enumerate(csv.DictReader(stream)):
            if int(row['preset']) != index // 750 or int(row['block']) != index % 750:
                raise ValueError('callback trace identity/order mismatch')
            frames, rate = int(row['frames']), int(row['rate'])
            if (frames, rate) != (128, 48000): raise ValueError('deadline changed')
            totals['callbacks'] += 1
            totals['frames'] += frames
            for clock in ('wall', 'cpu'):
                value = int(row[clock + '_ns'])
                if value < 0: raise ValueError('negative elapsed time')
                miss = value * rate > frames * 1000000000
                if int(row[clock + '_miss']) != int(miss): raise ValueError('deadline count mismatch')
                totals[clock + '_misses'] += int(miss)
                totals['max_' + clock + '_ns'] = max(totals['max_' + clock + '_ns'], value)
    if totals['callbacks'] != programs * 750:
        raise ValueError('incomplete callback trace')
    return totals


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source-build', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--programs', type=int, default=694)
    p.add_argument('--build-only', action='store_true')
    p.add_argument('--diagnostic-ftz', action='store_true', help='Host-only denormal diagnostic; never qualification')
    args = p.parse_args()
    if not 1 <= args.programs <= 694: raise ValueError('invalid program count')
    repo, out = Path(__file__).resolve().parents[2], args.output.resolve()
    # Snapshot new benchmark before starting any compilation.
    runner_bytes = Path(__file__).read_bytes()
    benchmark = (repo / 'tests/fixtures/synthone-callback/bench.cpp').read_bytes()
    contract = (repo / 'tests/fixtures/synthone-callback/CONTRACT.md').read_bytes()
    subprocess.run([sys.executable, repo / 'tests/host/run_synthone_factory.py', '--source-build',
                    args.source_build.resolve(), '--output', out, '--build-only'], check=True)
    spec = importlib.util.spec_from_file_location('factory', repo / 'tests/host/run_synthone_factory.py')
    factory = importlib.util.module_from_spec(spec); spec.loader.exec_module(factory)
    allowed = {}
    for manifest, root in [('TESTED-SOURCES.json', 'closure'), ('ADAPTER-SOURCES.json', 'adapter-inputs')]:
        for name, digest in json.loads((out / manifest).read_bytes()).items():
            allowed[(out / root / factory.relative(name)).resolve()] = digest
    for name, digest in json.loads((out / 'FACTORY-INPUTS.json').read_bytes()).items():
        allowed[(out / factory.relative(name)).resolve()] = digest
    source, contract_path = out / 'callback-bench.cpp', out / 'CALLBACK-CONTRACT.md'
    source.write_bytes(benchmark); contract_path.write_bytes(contract)
    (out / 'callback-runner.py').write_bytes(runner_bytes)
    allowed[(out / 'callback-runner.py').resolve()] = sha(runner_bytes)
    allowed[source] = sha(benchmark); allowed[contract_path] = sha(contract)
    commands = json.loads((out / 'factory-build-commands.json').read_bytes())
    originals = [c for c in commands if '-c' in c and c[c.index('-c') + 1].endswith('/preset_audio.cpp')]
    if len(originals) != 1: raise ValueError('expected one factory harness compile')
    compile_command = list(originals[0]); old_obj = compile_command[compile_command.index('-o') + 1]
    obj = str(out / 'callback-bench.o'); dep = out / 'callback-bench.d'
    compile_command[compile_command.index('-c') + 1] = str(source)
    compile_command[compile_command.index('-o') + 1] = obj
    compile_command[compile_command.index('-MF') + 1] = str(dep)
    link_command = [obj if x == old_obj else x for x in commands[-1]]
    if link_command.count(obj) != 1: raise ValueError('expected one callback harness link')
    link_command[link_command.index('-o') + 1] = str(out / 'callback-bench')
    factory.verify_files(allowed)
    subprocess.run(compile_command, check=True)
    factory.verify_dependencies(dep, allowed); factory.verify_files(allowed)
    subprocess.run(link_command, check=True); factory.verify_files(allowed)
    (out / 'CALLBACK-COMMANDS.json').write_text(json.dumps([compile_command, link_command], indent=2) + '\n')
    stream = out / 'commands/preset-commands.bin'
    stream_hash = sha(stream.read_bytes())
    allowed[stream] = stream_hash
    executable_hash = sha((out / 'callback-bench').read_bytes())
    inputs = {str(path.relative_to(out)): digest for path, digest in sorted(allowed.items())}
    (out / 'CALLBACK-INPUTS.json').write_text(json.dumps(inputs, indent=2) + '\n')
    if args.build_only:
        (out / 'CALLBACK-RESULT.json').write_text(json.dumps({'status': 'NOT_RUN', 'reason': 'build only'}) + '\n')
        return
    status = Path('/proc/self/status').read_text() if Path('/proc/self/status').exists() else 'unavailable'
    env = {'platform': platform.platform(), 'cpu_count': os.cpu_count(),
           'affinity': sorted(os.sched_getaffinity(0)) if hasattr(os, 'sched_getaffinity') else None,
           'tracer_pid': next((line.split(':', 1)[1].strip() for line in status.splitlines() if line.startswith('TracerPid:')), 'unknown')}
    (out / 'CALLBACK-ENVIRONMENT.json').write_text(json.dumps(env, indent=2) + '\n')
    run = subprocess.run([str(out / 'callback-bench'), str(out / 'factory-inputs/public-morphs.f32'),
        str(out / 'factory-inputs/compact.bin'), str(stream), str(out / 'callback-trace.csv'),
        str(out / 'callback-programs.jsonl'), str(args.programs), '1' if args.diagnostic_ftz else '0'], text=True, capture_output=True)
    (out / 'callback-run.log').write_text(run.stdout + run.stderr)
    factory.verify_files(allowed)
    if sha((out / 'callback-bench').read_bytes()) != executable_hash:
        raise ValueError('benchmark executable changed')
    summary = summarize(out / 'callback-trace.csv', args.programs)
    programs = [json.loads(line) for line in (out / 'callback-programs.jsonl').read_text().splitlines()]
    if len(programs) != args.programs or [x['preset'] for x in programs] != list(range(args.programs)):
        raise ValueError('incomplete program result')
    for clock in ('wall', 'cpu'):
        if sum(x[clock + '_misses'] for x in programs) != summary[clock + '_misses']:
            raise ValueError('program/trace miss counts disagree')
        if max(x['max_' + clock + '_ns'] for x in programs) != summary['max_' + clock + '_ns']:
            raise ValueError('program/trace maxima disagree')
    nonfinite = sum(x['nonfinite_words'] for x in programs)
    expected_exit = int(bool(summary['wall_misses'] or summary['cpu_misses'] or nonfinite))
    if run.returncode != expected_exit: raise ValueError('unexpected benchmark exit')
    receipt = {'scope': __doc__, 'programs': args.programs, 'corpus_complete': args.programs == 694, 'diagnostic_ftz': args.diagnostic_ftz,
        'qualification': 'NOT_QUALIFIED: diagnostic floating mode' if args.diagnostic_ftz else 'host measurement only',
        'summary': summary, 'nonfinite_words': nonfinite, 'process_exit': run.returncode,
        'status': 'DEADLINE_FAIL' if expected_exit else 'ZERO_MISSES_IN_THIS_RUN',
        'executable_sha256': executable_hash, 'stream_sha256': stream_hash, 'inputs_sha256': sha((out / 'CALLBACK-INPUTS.json').read_bytes()),
        'files': {name: sha((out / name).read_bytes()) for name in
                  ['callback-trace.csv', 'callback-programs.jsonl', 'CALLBACK-COMMANDS.json', 'CALLBACK-ENVIRONMENT.json', 'callback-run.log']}}
    (out / 'CALLBACK-RESULT.json').write_text(json.dumps(receipt, indent=2) + '\n')
    print(json.dumps(receipt))
    raise SystemExit(expected_exit)


if __name__ == '__main__': main()
