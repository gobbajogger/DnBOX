#!/usr/bin/env python3
"""Rebuild the pack-free harmonic provider and score its stereo factory captures."""
import argparse
import gzip
import hashlib
import json
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent / 'sj6-factory'))
from compare import channel_score


def sha(data):
    return hashlib.sha256(data).hexdigest()


def rebuild(base, provider, out):
    overlay = json.loads((provider / 'OVERLAY.json').read_text())
    for folder in ('closure', 'adapter-inputs', 'factory-inputs'):
        shutil.copytree(base / folder, out / folder)
    for rel, expected in overlay.items():
        src = provider / 'overlay' / rel
        if sha(src.read_bytes()) != expected:
            raise ValueError(f'provider input changed: {rel}')
        dst = (out / 'closure' / rel.removeprefix('synthone/') if rel.startswith('synthone/')
               else out / 'adapter-inputs/firmware/src/dsp' / rel)
        shutil.copyfile(src, dst)
    commands = json.loads((base / 'commands.json').read_text())
    factory = json.loads((base / 'factory-build-commands.json').read_text())
    for original in commands + factory:
        if '-c' not in original:
            continue
        cmd = [s.replace(str(base), str(out)) for s in original]
        cmd.insert(1, '-DS1_HARMONIC_ONLY=1')
        i = cmd.index('-c') + 1
        if cmd[i].endswith('/preset_audio.cpp'):
            cmd[i] = str(provider / 'preset_audio.cpp')
            cmd.insert(1, '-DS1_DEFINE_PARAMETER_BOUNDS=1')
            cmd.insert(1, '-I' + str(provider))
        subprocess.run(cmd, check=True)
    link = [s.replace(str(base), str(out)) for s in factory[-1]]
    link[link.index('-o') + 1] = str(out / 'pair')
    subprocess.run(link, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for argument in ('base', 'provider', 'out'):
        parser.add_argument(argument, type=Path)
    args = parser.parse_args()
    base, provider, out = (p.resolve() for p in (args.base, args.provider, args.out))
    out.mkdir(exist_ok=False)
    rebuild(base, provider, out)
    gold = json.loads(gzip.decompress((HERE / 'historical-oscillator-v3.json.gz').read_bytes()))
    rows = gold['rows']
    stream = (base / 'commands/preset-commands.bin').read_bytes()
    count, stride = 694, 4 + 144 * 8
    if struct.unpack_from('<II', stream) != (0x53315043, count) or len(stream) != 8 + count * stride or len(rows) != count:
        raise ValueError('unexpected factory corpus')
    assets = base / 'factory-inputs'
    passes = 0
    with (out / 'results.jsonl').open('w') as results, (out / 'pair.log').open('w') as log:
        for first in range(0, count, 4):
            n = min(4, count - first)
            with tempfile.TemporaryDirectory(dir=out) as tmp:
                tmp = Path(tmp)
                inp = tmp / 'commands.bin'
                inp.write_bytes(struct.pack('<II', 0x53315043, n) + stream[8+first*stride:8+(first+n)*stride])
                subprocess.run([out / 'pair', assets / 'public-morphs.f32', tmp / 'absent-pack',
                                inp, tmp], check=True, stdout=log, stderr=subprocess.STDOUT)
                for i in range(n):
                    idx = first + i
                    captured = [(tmp / f'preset-{i:02}-{arm}.f32le.pcm').read_bytes()
                                for arm in ('original', 'candidate')]
                    hashes = [sha(c) for c in captured]
                    if hashes != [rows[idx][s + '_sha256'] for s in ('original', 'candidate')]:
                        raise ValueError(f'PCM disagrees with recovered pair for {idx}')
                    audio = [np.frombuffer(c, '<f4').reshape(96000, 2) for c in captured]
                    if not all(np.isfinite(samples).all() for samples in audio):
                        raise ValueError(f'nonfinite output for {idx}')
                    metrics = [channel_score(audio[0][:, ch], audio[1][:, ch], 48000, (20, 16000))
                               for ch in (0, 1)]
                    ok = all(l is not None and abs(l) <= 1 and s is not None and s <= 3 for l, s in metrics)
                    passes += ok
                    row = dict(program=idx, original=hashes[0], candidate=hashes[1], left=metrics[0],
                               right=metrics[1], passed=bool(ok))
                    print(json.dumps(row, allow_nan=False), file=results, flush=True)
            print(f'{first+n}/{count}', flush=True)
    return passes != count


if __name__ == '__main__':
    sys.exit(main())
