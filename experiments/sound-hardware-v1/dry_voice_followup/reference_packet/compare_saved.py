#!/usr/bin/env python3
"""Assemble saved original + new candidate, then invoke unchanged metrics.py.

No renderer, DSP source, fitting, alignment, resampling, normalization, or trimming.
"""
from pathlib import Path
import argparse
import gzip
import hashlib
import json
import math
import subprocess
import sys

import numpy as np

HERE = Path(__file__).resolve().parent
METRIC_SHA256 = "bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1"
FRAMES = 88192
NOTES = 3


def read_pcm(path):
    data = path.read_bytes()
    return gzip.decompress(data) if path.name.endswith(".gz") else data


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--case", required=True, help="case ID in case_plan.json")
    ap.add_argument("--reference", type=Path, help="Defaults to the verified original-mono file recorded for the case")
    ap.add_argument("--reference-layout", choices=["verified-mono", "historical-pairs", "mono-capture"], default="verified-mono")
    ap.add_argument("--reference-sha256", help="Required for a new mono capture; exact uncompressed hash supplied by reference owner")
    ap.add_argument("--candidate", required=True, type=Path, help="New mono float32 LE, three consecutive notes 36,60,84")
    ap.add_argument("--output", required=True, type=Path, help="New paired .f32 output; must not exist")
    args = ap.parse_args()
    plan = json.loads((HERE / "case_plan.json").read_text())
    cases = {c["case_id"]: c for c in plan["cases"]}
    if args.case not in cases:
        ap.error("unknown case")
    case = cases[args.case]
    reference = args.reference
    if reference is None:
        if args.reference_layout != "verified-mono" or not case.get("current_reference_mono_relative_path"):
            ap.error("no recovered mono recording for this case; supply a separately verified reference")
        reference = HERE / case["current_reference_mono_relative_path"]
    metric = HERE / "metrics.py"
    if hashlib.sha256(metric.read_bytes()).hexdigest() != METRIC_SHA256:
        ap.error("historical metric has changed")
    if args.output.exists() or args.output.with_suffix(".json").exists():
        ap.error("output or result already exists; choose a new output path")
    if args.output.resolve() in [reference.resolve(), args.candidate.resolve()]:
        ap.error("output must be separate from both inputs")
    ref = read_pcm(reference)
    actual_sha = hashlib.sha256(ref).hexdigest()
    if args.reference_layout == "historical-pairs":
        expected_sha = case["historical_paired_pcm_sha256"]
    elif args.reference_layout == "verified-mono":
        expected_sha = case.get("current_reference_mono_sha256")
    else:
        expected_sha = args.reference_sha256
    if not expected_sha:
        ap.error("mono-capture requires the reference owner's exact --reference-sha256")
    if actual_sha != expected_sha:
        ap.error(f"reference hash mismatch: {actual_sha} != {expected_sha}")
    if args.reference_layout == "historical-pairs":
        if len(ref) != NOTES * FRAMES * 2 * 4:
            ap.error("historical paired reference must contain exactly 2,116,608 bytes")
        original = np.frombuffer(ref, dtype="<f4").reshape(NOTES, FRAMES, 2)[:, :, 0]
    else:
        if len(ref) != NOTES * FRAMES * 4:
            ap.error("mono reference must contain exactly 1,058,304 bytes")
        original = np.frombuffer(ref, dtype="<f4").reshape(NOTES, FRAMES)
    candidate_bytes = args.candidate.read_bytes()
    if len(candidate_bytes) != NOTES * FRAMES * 4:
        ap.error("candidate must contain exactly 1,058,304 bytes")
    candidate = np.frombuffer(candidate_bytes, dtype="<f4").reshape(NOTES, FRAMES)
    pair = np.empty((NOTES, FRAMES, 2), dtype="<f4")
    pair[:, :, 0] = original
    pair[:, :, 1] = candidate
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("xb") as f:
        f.write(pair.tobytes())
    subprocess.run([sys.executable, str(metric), str(args.output), "1"], check=True)
    report = json.loads(args.output.with_suffix(".json").read_text())
    accepted = report["gain"] == 1 and report["nonfinite"] == 0 and all(
        math.isfinite(r["rms_db"]) and math.isfinite(r["spectral_db"])
        and r["rms_db"] <= 1 and r["spectral_db"] <= 3
        for r in report["rows"]
    )
    receipt = {
        "case_id": args.case,
        "reference_layout": args.reference_layout,
        "reference_path": str(reference.resolve()),
        "reference_uncompressed_sha256": actual_sha,
        "historical_source_paired_sha256": case["historical_paired_pcm_sha256"],
        "candidate_sha256": hashlib.sha256(candidate_bytes).hexdigest(),
        "metric_sha256": METRIC_SHA256,
        "comparison_gain": 1,
        "accepted": accepted,
        "scope": "One controlled dry three-note screen; zero full-preset coverage",
    }
    args.output.with_suffix(".receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
    print(json.dumps(receipt, indent=2))
    return 0 if accepted else 1


if __name__ == "__main__":
    raise SystemExit(main())
