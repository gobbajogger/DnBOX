"""Score each folded delay on the original note, overlap and release schedule."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import os
from pathlib import Path
import subprocess

import numpy as np


def digest(words):
    return hashlib.sha256(words.tobytes()).hexdigest()


def spectrum(signal):
    blocks = np.lib.stride_tricks.sliding_window_view(signal, 512)[::128]
    return np.sqrt(np.mean(abs(np.fft.rfft(blocks * np.hanning(512), axis=1)) ** 2, axis=0))


def score(ref, ref_spec, trial):
    if np.array_equal(ref, trial):
        return dict(level_db=0., spectral_db=0., peak_error=0., passed=True)
    test_spec = spectrum(trial)
    floor = max(ref_spec.max() * 1e-4, 1e-12)
    keep = (ref_spec > floor) | (test_spec > floor)
    ratios = np.maximum(test_spec[keep], floor) / np.maximum(ref_spec[keep], floor)
    error = np.sqrt(np.mean((20 * np.log10(ratios)) ** 2)) if keep.any() else 0.
    level = 10 * np.log10(max(1e-20, np.mean(trial * trial)) / max(1e-20, np.mean(ref * ref)))
    return dict(level_db=float(level), spectral_db=float(error),
                peak_error=float(np.max(abs(ref - trial))), passed=bool(abs(level) <= 1 and error <= 3))


def main():
    cli = argparse.ArgumentParser(description=__doc__)
    for name in ('build', 'probe', 'output'):
        cli.add_argument(name, type=Path)
    cli.add_argument('--indices', nargs='*', type=int)
    cli.add_argument('--jobs', type=int, default=2)
    args = cli.parse_args()
    factory = args.build.resolve()
    assets = factory / 'factory-inputs'
    args.output.mkdir(parents=True, exist_ok=True)
    cases = json.loads((factory / 'commands/ORIGINAL-PRESET-COMMANDS.json').read_text())['cases']
    def run(index):
        row = cases[index]
        result = args.output / f'{index:03}.json'
        if result.exists():
            return json.loads(result.read_text())
        fd = os.memfd_create('factory-delay-pair')
        with os.fdopen(fd, 'w+b') as pcm:
            subprocess.run([args.probe, assets / 'public-morphs.f32', assets / 'compact.bin',
                            factory / 'commands/preset-commands.bin', str(index), f'/proc/self/fd/{fd}'],
                           check=True, pass_fds=(fd,))
            pcm.seek(0)
            data = np.frombuffer(pcm.read(), dtype='<f4').reshape(132300, 3, 4)
        if not np.isfinite(data).all():
            raise ValueError(f'Nonfinite samples on program {index}')
        controls = dict(row['commands'])
        original = data[:, 0, :]
        if not np.all(np.any(original[:, 2:] != 0.0, axis=0)):
            raise ValueError(f'Silent original program {index}')
        rec = dict(index=index, bank=row['bank'], position=row['preset_index'], name=row['name'],
                   wet_target=controls[41] * controls[44], original_wet_sha256=digest(original[:, :2]),
                   original_output_sha256=digest(original[:, 2:]), modes={})
        ref = original.astype(float)
        spectra = [spectrum(ch) for ch in ref.T]
        for k, mode in ((1, 'float32'), (2, 'binary16')):
            v = data[:, k, :]
            rec['modes'][mode] = {label: score(ref[:, i], spectra[i], v[:, i].astype(float))
                                  for i, label in enumerate(('wet_left', 'wet_right', 'left', 'right'))}
            rec['modes'][mode].update(wet_sha256=digest(v[:, :2]), output_sha256=digest(v[:, 2:]))
        result.write_text(json.dumps(rec) + '\n')
        return rec
    selected = args.indices or range(len(cases))
    with ThreadPoolExecutor(max_workers=args.jobs) as workers:
        rows = list(workers.map(run, selected))
    for mode in ('float32', 'binary16'):
        for pair in (('left', 'right'), ('wet_left', 'wet_right')):
            print(mode, pair, sum(all(row['modes'][mode][side]['passed'] for side in pair) for row in rows), len(rows))


if __name__ == '__main__':
    main()
