#!/usr/bin/env python3
"""Rebuild the original Linux DSP reference fixture using only the owner's AU."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import zipfile

AU_HASH = 'abe2f2e089306770489fb30c8870f608dc2f2e0804541f624337523679638a28'
EXEC_HASH = 'a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
BENCH_HASH = 'c1b7e99bf4f3054f114557a66c5ad8060525a5f8065e3ff3c7d208dbe2d9bf9c'
MATRIX_HASH = '08b912309b24edceebb159b8e3b1be1b3befc706201602212885ad73dd6cd3e0'
PARSER_PATH = 'superj6-static-behavior-20261115/static_routes.py'
BENCH_DIR = 'superj6-reference-bench-20261115'


def checked_zip(path, digest):
    actual = hashlib.sha256(path.read_bytes()).hexdigest()
    if actual != digest:
        raise ValueError(f'{path.name} digest {actual} does not match {digest}')
    return zipfile.ZipFile(path)


def restore(au, bench, matrix, root):
    # Substitute only the checked archive member supplying the original bytes.
    # Both the upstream binary hash assertion and all component guards remain.
    root.mkdir(exist_ok=True, parents=True)
    with checked_zip(matrix, MATRIX_HASH) as archive:
        parser = archive.read(PARSER_PATH).decode()
    first = parser.index('  raw=ARCHIVE.read_bytes()')
    last = parser.index(';assert hashlib.sha256(self.b)', first)
    parser = (parser[:first] +
              "  self.b=(ROOT.parent/'original-au.bin').read_bytes()" +
              parser[last:])
    target = root / PARSER_PATH
    target.parent.mkdir(exist_ok=True)
    target.write_text(parser)
    with checked_zip(au, AU_HASH) as archive:
        binary = archive.read('SuperJ6 AU.appex/SuperJ6 AU')
    if hashlib.sha256(binary).hexdigest() != EXEC_HASH:
        raise ValueError('Unexpected Mach-O identity')
    (root / 'original-au.bin').write_bytes(binary)
    work = root / BENCH_DIR
    work.mkdir(exist_ok=True)
    with checked_zip(bench, BENCH_HASH) as archive:
        for s in ['safe_arm64.py', 'original_chorus_audio.py', 'original_synth_smoke.py',
                  'original_sampler_smoke.py', 'macho_dsp.py', 'sampler_dsp.py', 'audio_utils.py',
                  'STATIC-DSP-DEPENDENCY-CLOSURE.private.json','SAMPLER-DEPENDENCY-CLOSURE.private.json']:
            (work / s).write_bytes(archive.read(s))
        expected = {n: hashlib.sha256(archive.read(n)).hexdigest()
                    for n in archive.namelist() if n.endswith('.f32le-stereo.private.pcm')}
    return work, expected


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for arg in ['au', 'bench', 'matrix', 'out']:
        p.add_argument('--' + arg, type=Path, required=True)
    args = p.parse_args()
    if args.out.exists():
        p.error("choose a new output directory so every PCM file is generated in this run")
    work, expected = restore(args.au, args.bench, args.matrix, args.out)
    for script in ['safe_arm64.py', 'original_chorus_audio.py', 'original_synth_smoke.py', 'original_sampler_smoke.py']:
        log = work / (script + '.log')
        with log.open('w') as out:
            subprocess.run([sys.executable, str((work / script).resolve())],
                           stdout=out, stderr=subprocess.STDOUT, check=True)
    actual_rows = {name: hashlib.sha256((work / name).read_bytes()).hexdigest() for name in expected}
    status = actual_rows == expected
    receipt = {
        'original_au_sha256': EXEC_HASH,
        'unicorn_version': json.loads((work / 'SYNTHETIC-HARNESS-RESULT.json').read_text())['unicorn_version'],
        'scenes': actual_rows,
        'matches_prior_components': status, 'complete_preset_renders': 0,
    }
    (args.out / 'component-replay.json').write_text(json.dumps(receipt, indent=2)+'\n')
    print(json.dumps(receipt))
    return 0 if status else 1


if __name__ == '__main__':
    sys.exit(main())
