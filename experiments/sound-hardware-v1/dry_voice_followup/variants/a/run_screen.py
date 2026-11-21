#!/usr/bin/env python3
"""Render fixed mono cases serially and use the unchanged packet comparison.

No fitting, delay alignment, gain, resampling, trimming or reference generation.
Only candidate-owned files in this unit's results directory are written.
"""
from pathlib import Path
import argparse
import hashlib
import json
import math
import subprocess
import sys

HERE = Path(__file__).resolve().parent
PACKET = HERE.parent / "dry_reference_packet"
SELECTED = "physical-gain/100_2000_.9"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--all", action="store_true", help="Run all recovered physical cases serially")
    ap.add_argument("--case", default=SELECTED)
    args = ap.parse_args()
    declaration = json.loads((HERE / "DECLARATION.json").read_text())
    for path, expected in declaration["frozen_files_sha256"].items():
        if sha(Path(path)) != expected:
            raise SystemExit(f"frozen file changed: {path}")
    plan = json.loads((PACKET / "case_plan.json").read_text())
    assert plan["sample_rate_hz"] == 44100
    assert plan["frames_per_note"] == 88192 and plan["note_off_frame"] == 66112
    assert plan["warmup_frames_per_note"] == 176448 and plan["comparison_gain"] == 1
    assert [n["frequency_f32_hex"] for n in plan["notes"]] == ["4282d013", "4382d013", "4482d013"]
    common = plan["historical_common_patch"]
    assert common["amp_ADSR"] == [0.05, 0.005, 0.8, 0.05]
    assert common["filter_ADSR_physical_gain"] == [0.05, 0.05, 1, 0.5]
    assert common["oscillator1_level"] == 1 and common["balance"] == 0.5
    cases = [c for c in plan["cases"] if c["case_id"].startswith("physical-gain/")]
    if args.all:
        assert len(cases) == 60
        cases.sort(key=lambda c: c["case_id"] != SELECTED)
    else:
        cases = [c for c in cases if c["case_id"] == args.case]
        assert len(cases) == 1
    results = HERE / "results" / declaration["variant"]["id"]
    results.mkdir(parents=True, exist_ok=True)
    all_rows = []
    run_receipts = []
    for case in cases:
        name = case["case_id"].split("/", 1)[1]
        candidate = results / f"{name}.candidate-mono.f32le"
        paired = results / f"{name}.paired.f32"
        report_path = paired.with_suffix(".json")
        receipt_path = paired.with_suffix(".receipt.json")
        reference = PACKET / case["current_reference_mono_relative_path"]
        assert sha(reference) == case["current_reference_mono_sha256"]
        if not receipt_path.exists():
            command = [str(HERE / "build" / "s1_dry_host"), str(candidate),
                       str(case["velocity"]), repr(case["cutoff_hz"]),
                       repr(case["resonance"]), repr(case["filter_depth"])]
            render = subprocess.run(command, text=True, capture_output=True)
            (results / f"{name}.render.log").write_text(
                json.dumps({"command": command, "exit_status": render.returncode}) + "\n" +
                render.stdout + render.stderr)
            if render.returncode:
                raise SystemExit(f"candidate render failed: {case['case_id']}")
            compare = subprocess.run([sys.executable, str(PACKET / "compare_saved.py"),
                "--case", case["case_id"], "--candidate", str(candidate),
                "--output", str(paired)], text=True, capture_output=True)
            (results / f"{name}.compare.log").write_text(
                json.dumps({"exit_status": compare.returncode}) + "\n" +
                compare.stdout + compare.stderr)
            if compare.returncode not in (0, 1) or not receipt_path.exists():
                raise SystemExit(f"comparison utility failed: {case['case_id']}")
        report = json.loads(report_path.read_text())
        receipt = json.loads(receipt_path.read_text())
        assert report["gain"] == 1 and receipt["comparison_gain"] == 1
        assert receipt["candidate_sha256"] == sha(candidate)
        assert receipt["reference_uncompressed_sha256"] == sha(reference)
        for row in report["rows"]:
            all_rows.append({"case_id": case["case_id"], "velocity": case["velocity"],
                "cutoff_hz": case["cutoff_hz"], "resonance": case["resonance"],
                **row, "nonfinite": report["nonfinite"],
                "accepted": math.isfinite(row["rms_db"]) and math.isfinite(row["spectral_db"])
                    and row["rms_db"] <= 1 and row["spectral_db"] <= 3 and report["nonfinite"] == 0})
        run_receipts.append(receipt)
        print(f"{case['case_id']}: {'PASS' if receipt['accepted'] else 'FAIL'} "
              f"RMS max {max(r['rms_db'] for r in report['rows']):.6f} dB, "
              f"spectral max {max(r['spectral_db'] for r in report['rows']):.6f} dB", flush=True)
    summary = {"scope": "180 static dry note cases only; zero intact preset coverage",
        "variant": declaration["variant"], "case_file_count": len(cases),
        "note_case_count": len(all_rows), "accepted_note_cases": sum(r["accepted"] for r in all_rows),
        "accepted_case_files": sum(r["accepted"] for r in run_receipts),
        "nonfinite": sum(r["nonfinite"] for r in all_rows[::3]),
        "max_rms_db": max(r["rms_db"] for r in all_rows),
        "max_spectral_db": max(r["spectral_db"] for r in all_rows),
        "gate": declaration["gate"], "rows": all_rows,
        "unqualified": ["72 dynamic filter cases", "all 694 intact presets", "150 control IDs",
                        "target size/link", "callback deadline", "listening approval"]}
    output = results / ("all180_summary.json" if args.all else "first_case_summary.json")
    output.write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps({k: v for k, v in summary.items() if k != "rows"}, indent=2))
    return 0 if all(r["accepted"] for r in all_rows) else 1


if __name__ == "__main__":
    raise SystemExit(main())
