#!/usr/bin/env python3
"""Replay current-head preset lifecycle and shipping-versus-candidate PCM comparisons on Linux."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
from compare import channel_score


def build(command, tree, log, env=None):
    with log.open("w") as output:
        subprocess.run(command, cwd=tree, env=env, stdout=output, stderr=subprocess.STDOUT, check=True)


def scores(shipping, candidate):
    rows = []
    for program in range(4):
        name = f"preset-{program}.f32le"
        a, b = (np.fromfile(root / name, dtype="<f4").reshape(-1, 2) for root in (shipping, candidate))
        if not (a.shape == b.shape == (32768, 2) and np.isfinite(a).all() and np.isfinite(b).all()):
            raise ValueError(f"Unexpected capture dimensions or nonfinite PCM for program {program}")
        changed = a.view("<u4") != b.view("<u4")
        frames = np.flatnonzero(changed.any(axis=1))
        rows.append({
            "factory": program, "frames": len(a), "different_words": int(changed.sum()),
            "first_changed_frame": int(frames[0]) if frames.size else None, "finite": True,
            "channels": [channel_score(left, right, 44100, (20, 16000)) for left, right in zip(a.T, b.T)],
            "shipping_sha256": hashlib.sha256(a.tobytes()).hexdigest(),
            "compact_sha256": hashlib.sha256(b.tobytes()).hexdigest(),
        })
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("shipping", "candidate", "out"):
        parser.add_argument(name, type=lambda p: Path(p).resolve())
    args = parser.parse_args()
    if sys.byteorder != "little":
        parser.error("The C fixture records native Float32 words and requires a little-endian host")
    args.out.mkdir(parents=True, exist_ok=False)
    for name, tree in (("shipping", args.shipping), ("candidate", args.candidate)):
        directory = args.out / name
        directory.mkdir()
        build(["sh", "host/build_host.sh", "--generate-only"], tree, directory / "generate.log")
        build(["sh", HERE / "build_runtime.sh", tree, directory], tree, directory / "runtime.log")
    lifecycle = args.out / "lifecycle"
    lifecycle.mkdir()
    env = dict(os.environ, SMC_UI_OUT=str(lifecycle), SMC_UI_TEST_FILE=str(HERE / "lifecycle.c"),
               X0X_HOST_DEFS="-Itests/host -Wl,--wrap=juno6_init")
    build(["sh", "tests/host/smc_ui_test.sh"], args.candidate, lifecycle / "build-run.log", env)
    receipt_line = "two-owner compact lifecycle: 82 assertions PASS"
    if receipt_line not in (lifecycle / "build-run.log").read_text().splitlines():
        raise RuntimeError(f"Missing completion marker in {lifecycle / 'build-run.log'}")
    rows = scores(args.out / "shipping", args.out / "candidate")
    (args.out / "measured.json").write_text(json.dumps(rows, indent=2, allow_nan=False) + "\n")
    expected = json.loads((HERE / "observed.json").read_text())
    for actual, old in zip(rows, expected, strict=True):
        for key in old:
            if key == "channels":
                if not np.allclose(actual[key], old[key], rtol=0, atol=1e-8):
                    raise ValueError(f"Changed spectral or level measurements for program {actual['factory']}")
            elif actual[key] != old[key]:
                raise ValueError(f"Changed {key} for program {actual['factory']}")
    print(receipt_line)
    print("Four shipping and four compact stereo PCM hashes match the recorded measurements.")


if __name__ == "__main__":
    main()
