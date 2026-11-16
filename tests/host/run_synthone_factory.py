#!/usr/bin/env python3
"""Fresh source-derived factory/frontend/FX comparison, not Apple-app/native/xrun qualification.
Predeclared threshold: zero differing stereo float words and zero nonfinite words.
Common original FX is not an independent FX oracle; two-second scenes do not cover all tails.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess
import sys


def sha(data):
    return hashlib.sha256(data).hexdigest()


def run(command):
    subprocess.run([str(x) for x in command], check=True)


def relative(name):
    p = Path(name)
    if p.is_absolute() or '..' in p.parts:
        raise ValueError('escaping input path')
    return p


def collect_factory_inputs(repo):
    """Snapshot bytes once. Never compile or generate from the mutable checkout."""
    fixture = Path('tests/fixtures/synthone-factory')
    corpus = Path('assets/factory_banks/synthone')
    data = {p: (repo / p).read_bytes() for p in [fixture / 'SOURCE-MANIFEST.json', corpus / 'MANIFEST.json',
            Path('tools/compile-synthone-factory.py'), Path('firmware/src/dsp/synthone_controls_data.inc')]}
    for item in json.loads(data[fixture / 'SOURCE-MANIFEST.json'])['files']:
        path = fixture / relative(item['path'])
        b = (repo / path).read_bytes()
        if len(b) != item['bytes'] or sha(b) != item['sha256']:
            raise ValueError('fixture hash mismatch: ' + str(path))
        if path in data:
            raise ValueError('duplicate input path')
        data[path] = b
    for item in json.loads(data[corpus / 'MANIFEST.json'])['files']:
        path = corpus / relative(item['name'])
        b = (repo / path).read_bytes()
        blob = hashlib.sha1(b'blob ' + str(len(b)).encode() + b'\0' + b).hexdigest()
        if len(b) != item['size'] or sha(b) != item['sha256'] or blob != item['sha']:
            raise ValueError('factory original hash mismatch: ' + str(path))
        if path in data:
            raise ValueError('duplicate input path')
        data[path] = b
    return data


def stage_inputs(data, stage):
    allowed = {}
    for path, b in data.items():
        dest = stage / relative(str(path))
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(b)
        allowed[dest.resolve()] = sha(b)
    return allowed


def verify_files(allowed):
    for path, digest in allowed.items():
        if sha(path.read_bytes()) != digest:
            raise ValueError('bound input changed: ' + str(path))


def verify_dependencies(depfile, allowed):
    content = depfile.read_text().replace('\\\n', ' ')
    for name in shlex.split(content.split(':', 1)[1]):
        path = Path(name).resolve()
        if path not in allowed:
            raise ValueError('unlisted include/source: ' + str(path))
        if sha(path.read_bytes()) != allowed[path]:
            raise ValueError('bound dependency changed: ' + str(path))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source-build', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--build-only', action='store_true', help='Compile only; emits no audio PASS result')
    args = p.parse_args()
    repo, out, source = Path(__file__).resolve().parents[2], args.output.resolve(), args.source_build.resolve()
    if out.exists():
        raise ValueError('output must be fresh')
    snapshot = collect_factory_inputs(repo)
    # Strict source closure verifier rebuilds all runtime objects and protects the oracle.
    run([sys.executable, repo / 'tests/host/run_synthone_renderer.py', '--source-build', source, '--output', out])
    stage = out / 'factory-inputs'
    allowed = stage_inputs(snapshot, stage)
    runtime_manifest = (out / 'TESTED-SOURCES.json').read_bytes()
    for name, digest in json.loads(runtime_manifest).items():
        allowed[(out / 'closure' / relative(name)).resolve()] = digest
    assets = {'public-morphs.f32': (852176, '0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5'),
              'compact.bin': (165292, 'a5cf8182b8bc9ca0a1b034a6b63f55d2cdc1087df38d6b1433da4fb56e5eccd5')}
    for name, (size, digest) in assets.items():
        b = (source / name).read_bytes()
        if len(b) != size or sha(b) != digest:
            raise ValueError('original binary fixture mismatch: ' + name)
        dest = stage / name
        dest.write_bytes(b)
        allowed[dest.resolve()] = sha(b)
    verify_files(allowed)
    receipt = {str(path.relative_to(out)): digest for path, digest in sorted(allowed.items())}
    (out / 'FACTORY-INPUTS.json').write_text(json.dumps(receipt, indent=2) + '\n')
    run([sys.executable, stage / 'tools/compile-synthone-factory.py', '--out', out / 'commands'])
    command_stream = out / 'commands/preset-commands.bin'
    allowed[command_stream.resolve()] = sha(command_stream.read_bytes())
    fixture = stage / 'tests/fixtures/synthone-factory'
    frontend, fx = fixture / 'frontend', fixture / 'fullfx'
    cmds = json.loads((out / 'commands.json').read_text())
    harnesses = [c for c in cmds if any(x.endswith('/synthone_renderer_test.cpp') for x in c)]
    if len(harnesses) != 1:
        raise ValueError('expected exactly one verified renderer harness')
    harness = harnesses[0]
    harness_obj = harness[harness.index('-o') + 1]
    includes = ['-I' + str(x) for x in [frontend, fx, fx / 'compat', fx / 'upstream/AudioKit/Core/Soundpipe/lib/faust']]
    for arg in harness:
        if not arg.startswith('-I'):
            continue
        path = Path(arg[2:]).resolve()
        if path == (repo / 'firmware/src/dsp').resolve():
            continue  # C ABI include root is unused by this source-only fixture.
        if not path.is_relative_to(out / 'closure'):
            raise ValueError('unbound include root: ' + str(path))
        includes.append(arg)
    flags = ['-O2', '-fno-builtin', '-ffunction-sections', '-fdata-sections', '-ffp-contract=off', '-fno-fast-math', '-DNO_LIBSNDFILE']
    commands, objects = [], []
    units = [('cc', '-std=gnu99', fx / 'upstream/AudioKit/Core/Soundpipe/modules' / (name + '.c'))
             for name in ['pan2', 'phaser', 'vdelay', 'delay', 'revsc', 'compressor']]
    units += [('c++', '-std=c++17', frontend / name) for name in ['preset_audio.cpp', 'source_controls.cpp', 'oracle_controls.cpp']]
    for index, (compiler, standard, src) in enumerate(units):
        obj, dep = out / ('factory-%d.o' % index), out / ('factory-%d.d' % index)
        verify_files(allowed)
        cmd = [compiler, standard, *flags, *includes, '-MMD', '-MF', str(dep), '-c', str(src), '-o', str(obj)]
        commands.append(cmd)
        run(cmd)
        verify_dependencies(dep, allowed)
        verify_files(allowed)
        objects.append(str(obj))
    objects += [x for x in cmds[-1] if x.endswith('.o') and x != harness_obj and not x.endswith('/c-wrapper.o')]
    cmd = ['c++', *objects, '-Wl,--gc-sections', '-Wl,--wrap=s1c_read', '-Wl,--wrap=malloc', '-Wl,--wrap=calloc',
           '-Wl,--wrap=realloc', '-Wl,--wrap=free', '-lm', '-o', str(out / 'factory-test')]
    commands.append(cmd)
    (out / 'factory-build-commands.json').write_text(json.dumps(commands, indent=2) + '\n')
    run(cmd)
    verify_files(allowed)
    if args.build_only:
        (out / 'FACTORY-BUILD-ONLY.json').write_text(json.dumps({'audio': 'NOT_RUN', 'factory_inputs_sha256': sha((out / 'FACTORY-INPUTS.json').read_bytes())}) + '\n')
        return
    (out / 'pcm').mkdir()
    run([out / 'factory-test', stage / 'public-morphs.f32', stage / 'compact.bin', command_stream, out / 'pcm'])
    verify_files(allowed)
    progress = json.loads((out / 'pcm/progress.json').read_text())
    if progress['completed_presets'] != 694 or progress['total_presets'] != 694:
        raise ValueError('incomplete factory corpus')
    (out / 'FACTORY-RESULT.json').write_text(json.dumps({
        'scope': __doc__, 'presets': 694, 'frames': progress['frames'], 'differing_words': 0, 'nonfinite_words': 0,
        'factory_inputs_sha256': sha((out / 'FACTORY-INPUTS.json').read_bytes()),
        'runtime_tested_sources_sha256': sha(runtime_manifest),
        'factory_commands_sha256': sha((out / 'factory-build-commands.json').read_bytes()),
        'frontend_commands_sha256': allowed[command_stream.resolve()]
    }, indent=2) + '\n')


if __name__ == '__main__':
    main()
