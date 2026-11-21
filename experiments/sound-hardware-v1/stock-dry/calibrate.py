#!/usr/bin/env python3
"""Reproduce the independent black-box impulse fit used for the stock dry voice."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import numpy as np
from scipy.optimize import brentq

here = Path(__file__).resolve().parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("recovery", type=Path)
parser.add_argument("output", type=Path)
args = parser.parse_args()
seed = args.recovery.resolve() / "comparison_only/reference_filter_inputs/full_dry_preparation"
lib = seed / "staged_reference/closure/experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe"
obj = seed / "build/dependency-007.o"
pins = json.loads((here / "comparison-inputs.json").read_text())
assert hashlib.sha256(obj.read_bytes()).hexdigest() == pins["build/dependency-007.o"]
out = args.output.resolve()
out.mkdir()
exe = out / "impulse"
subprocess.run([os.environ.get("CC", "cc"), "-std=gnu99", "-O2", "-fno-builtin", "-ffunction-sections",
                "-fdata-sections", "-ffp-contract=off", "-fno-fast-math", "-DNO_LIBSNDFILE",
                f"-I{lib}", f"-I{lib / 'lib/dr_wav'}", here / "capture-impulse.c", obj,
                "-Wl,--gc-sections", "-lm", "-o", exe], check=True)
observations = json.loads((here / "impulse-calibration.json").read_text())
x, step, feedback = [], [], []
for hz, expected in observations.items():
    captures = []
    for resonance, digest in [("0", "zero_sha256"), (".5", "half_sha256")]:
        pcm = out / f"{hz}-{resonance}.f32"
        subprocess.run([exe, hz, resonance, pcm], check=True)
        assert hashlib.sha256(pcm.read_bytes()).hexdigest() == expected[digest], pcm
        captures.append(np.fromfile(pcm, "<f4").astype(float) / np.float32(.0001))
    relative = int(hz) / 22050
    x.append(relative)
    step.append(brentq(lambda s: s**4 * (3 - 2*s) - captures[0][0], .000001, 1) / relative)
    feedback.append(2 * (1 / captures[1].sum() - 1))
for name, samples, degree in [("cutoff", step, 4), ("feedback", feedback, 4)]:
    fit = np.polynomial.Polynomial.fit(x, samples, degree).convert()
    # The firmware retained the first three terms of the feedback fit.
    print(name, fit.coef[:3].tolist() if name == "feedback" else fit.coef.tolist())
