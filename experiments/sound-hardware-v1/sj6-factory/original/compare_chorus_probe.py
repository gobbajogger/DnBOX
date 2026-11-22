#!/usr/bin/env python3
"""Score the isolated DnBOX chorus against original ARM64 probe captures."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from compare import channel_score


def sha(data):
    return hashlib.sha256(data).hexdigest()


def load_channels(raw, frames):
    audio = np.frombuffer(raw, '<f4')
    if len(audio) != 2 * frames or not np.isfinite(audio).all():
        raise ValueError("invalid planar stereo")
    return audio.reshape(2, frames)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference_dir", type=Path)
    parser.add_argument("reference_receipt", type=Path)
    parser.add_argument("candidate", type=Path)
    parser.add_argument("candidate_source")
    parser.add_argument("output_dir", type=Path)
    args = parser.parse_args()
    args.output_dir.mkdir()
    receipt = json.loads(args.reference_receipt.read_text())
    stimulus = (args.reference_dir / "stimulus.f32").read_bytes()
    assert sha(stimulus) == receipt["stimulus_sha256"]
    load_channels(stimulus, receipt["frames"])
    receipt["candidate_source"] = args.candidate_source
    receipt["candidate_runner_sha256"] = sha(args.candidate.read_bytes())
    receipt["candidate_wet_controls"] = {"rate_hz": 0.8, "depth_ms": 3.4, "mix": 0.22}
    receipt["candidate_wet_route"] = "stock chorus at reset defaults for each original wet mode"
    candidates = {}
    for active in (False, True):
        candidates[active] = subprocess.run([str(args.candidate.resolve()), str(int(active))],
                                            input=stimulus, capture_output=True, check=True).stdout
        (args.output_dir / f"active-{int(active)}.f32").write_bytes(candidates[active])
    for case in receipt["cases"]:
        original = (args.reference_dir / (case["case"] + ".f32")).read_bytes()
        assert sha(original) == case["reference_sha256"]
        active = case["started"] and case["params_readback"]["mix"] != 0
        candidate = candidates[active]
        case["candidate_sha256"] = sha(candidate)
        case["bit_exact"] = original == candidate
        x = load_channels(original, receipt["frames"])
        y = load_channels(candidate, receipt["frames"])
        case["levels_spectra_db"] = [channel_score(a, b, receipt["rate"], (20, 16000))
                                      for a, b in zip(x, y)]
    print(json.dumps(receipt, indent=2))


if __name__ == "__main__":
    main()
