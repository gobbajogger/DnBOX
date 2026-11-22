#!/usr/bin/env python3
"""Check the existing five witnesses and generate independent two-rate holdouts."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import subprocess
import sys
from zipfile import ZipFile

import numpy as np

from probe_chorus_arm64 import EXPECTED, render

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from compare import channel_score


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def sustained(rate, frames):
    tones = (116.54, 174.61, 261.63, 440., 653.4, 1080., 2244., 4930.)
    samples = []
    for side in range(2):
        for i in range(frames):
            gain = min(1., i / 4096., (frames - i - 1) / 4096.)
            samples.append(gain * sum(.038 * math.sin(2 * math.pi * hz * i / rate + side * j * .23)
                                      for j, hz in enumerate(tones)))
    return struct.pack(f'<{2 * frames}f', *samples)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_au", type=Path)
    parser.add_argument("reference_pack", type=Path)
    parser.add_argument("candidate", type=Path)
    parser.add_argument("out", type=Path)
    args = parser.parse_args()
    args.out.mkdir()
    with ZipFile(args.original_au) as archive:
        original = archive.read("SuperJ6 AU.appex/SuperJ6 AU")
    if sha(original) != EXPECTED:
        raise ValueError("unexpected original ARM64 executable")

    cases = []

    def evaluate(label, mode, rate, frames, mix, input_raw, reference, readback):
        candidate = subprocess.run([str(args.candidate.resolve()), str(mode), str(rate), str(frames), str(mix)],
                                   input=input_raw, capture_output=True, check=True).stdout
        a = np.frombuffer(reference, '<f4').reshape(2, frames)
        b = np.frombuffer(candidate, '<f4').reshape(2, frames)
        if not np.isfinite(a).all() or not np.isfinite(b).all():
            raise ValueError("nonfinite component capture")
        metrics = [channel_score(x, y, rate, [20, 16000]) for x, y in zip(a, b)]
        bit_exact = candidate == reference
        passed = bit_exact or all(level is not None and spectral is not None and abs(level) <= 1 and spectral <= 3
                                  for level, spectral in metrics)
        if not label.startswith("archived-"):
            (args.out / f"{label}.original.f32").write_bytes(reference)
        (args.out / f"{label}.candidate.f32").write_bytes(candidate)
        cases.append(dict(case=label, mode=mode, rate=rate, frames=frames, mix=mix,
                          bit_exact=bit_exact, passed=passed, metrics=metrics, params_readback=readback,
                          stimulus_sha256=sha(input_raw), reference_sha256=sha(reference),
                          candidate_sha256=sha(candidate)))

    prefix = "original/chorus/"
    with ZipFile(args.reference_pack) as pack:
        receipt = json.loads(pack.read(prefix + "receipt.json"))
        if receipt["original_au_sha256"] != EXPECTED:
            raise ValueError("unexpected archived reference executable")
        raw = pack.read(prefix + "stimulus.f32")
        assert sha(raw) == receipt["stimulus_sha256"]
        for case in receipt["cases"]:
            name = case["case"]
            original_pcm = pack.read(prefix + name + ".f32")
            assert sha(original_pcm) == case["reference_sha256"]
            readback = case["params_readback"]
            mode = int(readback["mode"]) if case["started"] else 0
            evaluate("archived-" + name, mode, receipt["rate"], receipt["frames"],
                     readback["mix"], raw, original_pcm, readback)

    for rate in (44100, 48000):
        frames = 131072
        impulses = np.zeros((2, frames), dtype='<f4')
        impulses[:, ::4096] = .25
        impulse_raw = impulses.tobytes()
        sustained_frames = 65536
        chord_raw = sustained(rate, sustained_frames)
        (args.out / f"impulses-{rate}.stimulus.f32").write_bytes(impulse_raw)
        (args.out / f"chord-{rate}.stimulus.f32").write_bytes(chord_raw)
        for mode in (1, 2, 3):
            original_pcm, readback = render(original, impulse_raw, mode, 1., True, rate)
            evaluate(f"impulses-{rate}-{mode}", mode, rate, frames, 1., impulse_raw, original_pcm, readback)
            for mix in (1., .5):
                original_pcm, readback = render(original, chord_raw, mode, mix, True, rate)
                evaluate(f"chord-{rate}-{mode}-{mix:g}", mode, rate, sustained_frames, mix,
                         chord_raw, original_pcm, readback)

    assert len(cases) == 23
    assert all(c["passed"] for c in cases if c["case"].startswith(("archived-", "impulses-")))
    assert sum(c["bit_exact"] for c in cases) == 2
    manifest = {"software_only": True, "level_limit_db": 1, "spectral_limit_db": 3,
                "original_executable_sha256": sha(original),
                "original_archive_sha256": sha(args.original_au.read_bytes()),
                "archived_pack_sha256": sha(args.reference_pack.read_bytes()),
                "candidate_executable_sha256": sha(args.candidate.read_bytes()), "cases": cases}
    (args.out / "comparison.json").write_text(json.dumps(manifest, indent=2) + "\n")


if __name__ == "__main__":
    main()
