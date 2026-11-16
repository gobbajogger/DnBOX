#!/usr/bin/env python3
"""Rebuild all factory source comparisons; requires the verified original closure.

This is a host source-derived renderer/FX comparison, not Apple-app/device,
native-memory, tuning/hold/modwheel, or callback-xrun qualification.
The predeclared comparison threshold is zero differing stereo float words and
zero nonfinite words, enforced by preset_audio.cpp before it produces metrics.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys


def sha(data):
    return hashlib.sha256(data).hexdigest()


def run(command):
    subprocess.run([str(x) for x in command], check=True)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source-build', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    repo = Path(__file__).resolve().parents[2]
    out = args.output.resolve()
    if out.exists():
        raise ValueError('output must be fresh')
    source = args.source_build.resolve()
    fixture = repo / 'tests/fixtures/synthone-factory'
    frontend, fx = fixture / 'frontend', fixture / 'fullfx'
    corpus = repo / 'assets/factory_banks/synthone'
    fixture_manifest = (fixture / 'SOURCE-MANIFEST.json').read_bytes()
    corpus_manifest = (corpus / 'MANIFEST.json').read_bytes()
    for item in json.loads(fixture_manifest)['files']:
        data = (fixture / item['path']).read_bytes()
        if len(data) != item['bytes'] or sha(data) != item['sha256']:
            raise ValueError('fixture hash mismatch: ' + item['path'])
    for item in json.loads(corpus_manifest)['files']:
        data = (corpus / item['name']).read_bytes()
        blob = hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()
        if len(data) != item['size'] or sha(data) != item['sha256'] or blob != item['sha']:
            raise ValueError('factory original hash mismatch: ' + item['name'])
    # Existing strict closure verifier recompiles every dependency, protects the
    # oracle from runtime overlays, and runs its C/C++/source boundary comparison.
    run([sys.executable, repo / 'tests/host/run_synthone_renderer.py',
         '--source-build', source, '--output', out])
    run([sys.executable, repo / 'tools/compile-synthone-factory.py', '--out', out / 'commands'])
    cmds = json.loads((out / 'commands.json').read_text())
    harness = next(c for c in cmds if any(x.endswith('/synthone_renderer_test.cpp') for x in c))
    harness_obj = harness[harness.index('-o') + 1]
    includes = [x for x in harness if x.startswith('-I')]
    includes += ['-I' + str(x) for x in [frontend, fx, fx / 'compat',
                 fx / 'upstream/AudioKit/Core/Soundpipe/lib/faust']]
    flags = ['-O2', '-fno-builtin', '-ffunction-sections', '-fdata-sections',
             '-ffp-contract=off', '-fno-fast-math', '-DNO_LIBSNDFILE']
    commands, objects = [], []
    for name in ['pan2', 'phaser', 'vdelay', 'delay', 'revsc', 'compressor']:
        src = fx / 'upstream/AudioKit/Core/Soundpipe/modules' / (name + '.c')
        obj = out / (name + '.o')
        cmd = ['cc', '-std=gnu99', *flags, *includes, '-c', str(src), '-o', str(obj)]
        commands.append(cmd)
        run(cmd)
        objects.append(str(obj))
    # Only the freshly rebuilt verified recipe objects; never old donor .o files.
    objects += [x for x in cmds[-1] if x.endswith('.o') and x != harness_obj
                and not x.endswith('/c-wrapper.o')]
    cmd = ['c++', '-std=c++17', *flags, *includes,
           *[str(frontend / name) for name in ['preset_audio.cpp', 'source_controls.cpp', 'oracle_controls.cpp']],
           *objects, '-Wl,--gc-sections', '-Wl,--wrap=s1c_read', '-Wl,--wrap=malloc',
           '-Wl,--wrap=calloc', '-Wl,--wrap=realloc', '-Wl,--wrap=free', '-lm', '-o', str(out / 'factory-test')]
    commands.append(cmd)
    (out / 'factory-build-commands.json').write_text(json.dumps(commands, indent=2) + '\n')
    run(cmd)
    (out / 'pcm').mkdir()
    run([out / 'factory-test', source / 'public-morphs.f32', source / 'compact.bin',
         out / 'commands/preset-commands.bin', out / 'pcm'])
    progress = json.loads((out / 'pcm/progress.json').read_text())
    if progress['completed_presets'] != 694 or progress['total_presets'] != 694:
        raise ValueError('incomplete factory corpus')
    if fixture_manifest != (fixture / 'SOURCE-MANIFEST.json').read_bytes() or corpus_manifest != (corpus / 'MANIFEST.json').read_bytes():
        raise ValueError('input manifests changed during run')
    (out / 'FACTORY-RESULT.json').write_text(json.dumps({
        'scope': __doc__, 'presets': 694, 'frames': progress['frames'],
        'differing_words': 0, 'nonfinite_words': 0,
        'fixture_manifest_sha256': sha(fixture_manifest),
        'corpus_manifest_sha256': sha(corpus_manifest),
        'factory_commands_sha256': sha((out / 'factory-build-commands.json').read_bytes()),
        'frontend_commands_sha256': sha((out / 'commands/preset-commands.bin').read_bytes())
    }, indent=2) + '\n')


if __name__ == '__main__':
    main()
