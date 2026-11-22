"""Run two independent owners through live tap, feedback and reset changes for 15 s."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import numpy as np
from music import digest, score, spectrum

here = Path(__file__).resolve().parent
cli = argparse.ArgumentParser(description=__doc__)
cli.add_argument('factory', type=Path)
cli.add_argument('output', type=Path)
args = cli.parse_args()
base, out = args.factory.resolve(), args.output.resolve()
out.mkdir(parents=True, exist_ok=True)
cmds = json.loads((base / 'factory-build-commands.json').read_text())
cc = next(row.copy() for row in cmds if any(a.endswith('/preset_audio.cpp') for a in row))
cc[cc.index('-c') + 1] = str(here / 'live.cpp')
cc[cc.index('-o') + 1] = str(out / 'live.o')
cc[cc.index('-MF') + 1] = str(out / 'live.d')
subprocess.run(cc, check=True)
link = cmds[-1].copy()
link[link.index(str(base / 'factory-6.o'))] = str(out / 'live.o')
link[link.index('-o') + 1] = str(out / 'live')
subprocess.run(link, check=True)
fd = os.memfd_create('live-delay-history')
with os.fdopen(fd, 'w+b') as recording:
    subprocess.run([out / 'live', f'/proc/self/fd/{fd}'], pass_fds=(fd,), check=True)
    recording.seek(0)
    data = np.frombuffer(recording.read(), dtype='<f4').reshape(15 * 44100, 2, 3, 2)
assert np.isfinite(data).all() and not np.array_equal(data[:, 0], data[:, 1])
steps = [0, 5, 7, 10, 11, 12, 15]
rows = []
for a, b in zip(steps, steps[1:]):
    for owner in (0, 1):
        block = data[a * 44100:b * 44100, owner]
        ref = block[:, 0, :].astype(float)
        specs = [spectrum(ref[:, c]) for c in (0, 1)]
        row = dict(owner=owner, start_s=a, end_s=b, original_sha256=digest(block[:, 0, :]), modes={})
        for k, mode in ((1, 'float32'), (2, 'binary16')):
            row['modes'][mode] = [score(ref[:, c], specs[c], block[:, k, c].astype(float)) for c in (0, 1)]
        rows.append(row)
(out / 'live-scores.json').write_text(json.dumps(rows, indent=2) + '\n')
for mode in ('float32', 'binary16'):
    print(mode, sum(all(c['passed'] for c in r['modes'][mode]) for r in rows), len(rows))
