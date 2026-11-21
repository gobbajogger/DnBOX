#!/usr/bin/env python3
"""Analyze immutable saved C/B output only. No compiler, render or metric run."""
from pathlib import Path
import hashlib
import json
import math

import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
C_AUDIO = ROOT / "s1_dry_voice_c" / "results" / "audio180"
B_AUDIO = ROOT / "s1_dry_voice_unit" / "variant_b" / "results" / "ch4_2x_slope4"
METRIC_SHA = "bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1"
FRAMES, TAIL = 88192, 4096
NOTES = [36, 60, 84]


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def sha_bytes(data):
    return hashlib.sha256(data).hexdigest()


def sha(path):
    return sha_bytes(path.read_bytes())


def write_json(path, value):
    with path.open("x") as stream:
        json.dump(value, stream, indent=2, allow_nan=False)
        stream.write("\n")


def stats(rows):
    return {"notes": len(rows), "accepted": sum(r["accepted"] for r in rows),
            "maximum_rms_db": max(r["rms_db"] for r in rows),
            "maximum_spectral_db": max(r["spectral_db"] for r in rows)}


def tail_levels(pair, note_index):
    # These descriptive tail levels are separate from the full-record metric.
    tail = pair[note_index, -TAIL:, :].astype(np.float64)
    return {"reference_tail_rms": float(np.sqrt(np.mean(tail[:, 0] ** 2))),
            "candidate_tail_rms": float(np.sqrt(np.mean(tail[:, 1] ** 2))),
            "reference_tail_peak": float(np.max(np.abs(tail[:, 0]))),
            "candidate_tail_peak": float(np.max(np.abs(tail[:, 1])))}


def main():
    summary_path = C_AUDIO / "SUMMARY.json"
    summary = json.loads(summary_path.read_text())
    before_summary_sha = sha(summary_path)
    rows = summary["rows"]
    old_summary = json.loads((B_AUDIO / "all180_summary.json").read_text())
    old_rows = old_summary["rows"]
    require(summary["case_files"] == 60 and summary["note_cases"] == 180 and len(rows) == 180,
            "Expected the completed C180 screen")
    case_ids = sorted({row["case_id"] for row in rows})
    require(len(case_ids) == 60, "Expected60 unique C files")
    declaration = json.loads((C_AUDIO / "DECLARATION.json").read_text())
    require(declaration["metric_sha256"] == METRIC_SHA and declaration["gain"] == 1,
            "Comparison declaration differs from unchanged metric/gain")
    reports_verified = 0
    nonfinite = 0
    zero_hashes, tails = [], []
    receipt_by_case = {}
    for case_id in case_ids:
        selected = sorted([row for row in rows if row["case_id"] == case_id], key=lambda r: r["note"])
        require([row["note"] for row in selected] == NOTES, "Missing or reordered notes")
        name = case_id.split("/", 1)[1]
        receipt = json.loads((C_AUDIO / (name + ".receipt.json")).read_text())
        receipt_by_case[case_id] = receipt
        candidate_path = C_AUDIO / (name + ".candidate-mono.f32le")
        candidate_sha = sha(candidate_path)
        require(candidate_sha == receipt["candidate_sha256"], "C candidate hash mismatch: " + name)
        pair_bytes = (C_AUDIO / (name + ".paired.f32")).read_bytes()
        require(len(pair_bytes) == 3 * FRAMES * 2 * 4, "Paired file length differs")
        require(sha_bytes(pair_bytes) == receipt["paired_sha256"], "C pair hash mismatch: " + name)
        report_path = C_AUDIO / (name + ".paired.json")
        require(sha(report_path) == receipt["report_sha256"], "C report hash mismatch: " + name)
        report = json.loads(report_path.read_text())
        require(report["gain"] == 1 and receipt["metric_sha256"] == METRIC_SHA,
                "Saved gain or metric differs")
        require([r["note"] for r in report["rows"]] == NOTES, "Saved note order differs")
        for source_row, summary_row in zip(report["rows"], selected):
            require(source_row["rms_db"] == summary_row["rms_db"] and
                    source_row["spectral_db"] == summary_row["spectral_db"],
                    "Summary differs from saved metric")
        nonfinite += report["nonfinite"]
        reports_verified += 1
        pair = np.frombuffer(pair_bytes, dtype="<f4").reshape(3, FRAMES, 2)
        for i, row in enumerate(selected):
            tails.append({"case_id": case_id, "note": row["note"],
                "velocity": row["velocity"], "cutoff_hz": row["cutoff_hz"],
                "resonance": row["resonance"], **tail_levels(pair, i)})
        if selected[0]["resonance"] == 0:
            old_receipt = json.loads((B_AUDIO / (name + ".paired.receipt.json")).read_text())
            old_candidate = B_AUDIO / (name + ".candidate-mono.f32le")
            old_sha = sha(old_candidate)
            require(old_sha == old_receipt["candidate_sha256"], "Frozen B candidate changed")
            zero_hashes.append({"case_id": case_id, "bytes": candidate_path.stat().st_size,
                "B_candidate_sha256": old_sha, "C_candidate_sha256": candidate_sha,
                "identical": old_sha == candidate_sha})
    require(reports_verified == 60 and len(zero_hashes) == 15, "Coverage mismatch")

    groups = {}
    for field in ["resonance", "velocity", "cutoff_hz"]:
        groups[field] = [{field: value,
                         **stats([row for row in rows if row[field] == value])}
                        for value in sorted({row[field] for row in rows})]
    grid = []
    for velocity in [20, 100, 127]:
        for cutoff in [64, 500, 2000, 8000, 22050]:
            cell = {"velocity": velocity, "cutoff_hz": cutoff}
            for resonance, field in [(0.0, "resonance_0"),
                                      (0.8999999761581421, "resonance_0_9")]:
                cell[field] = stats([row for row in rows if row["velocity"] == velocity
                    and row["cutoff_hz"] == cutoff and row["resonance"] == resonance])
            grid.append(cell)
    b_pass = {(r["case_id"], r["note"]) for r in old_rows if r["accepted"]}
    c_pass = {(r["case_id"], r["note"]) for r in rows if r["accepted"]}
    strata = {"status": "SAVED_C_AUDIO_FAIL", "scope": "Read saved metrics only; no metric repeat",
        "source_summary": str(summary_path), "source_summary_sha256": before_summary_sha,
        "terminal_reported_by_root": {"start_utc": "2026-11-19 00:03:44 UTC",
                                      "end_utc": "2026-11-19 00:04:10 UTC",
                                      "exit_status": 1, "meaning": "normal audio-gate FAIL"},
        "totals": {key: summary[key] for key in ["case_files", "note_cases", "accepted_files",
            "accepted_notes", "maximum_rms_db", "maximum_spectral_db", "accepted"]},
        "nonfinite_from_saved_reports": nonfinite, "saved_case_reports_verified": reports_verified,
        "groups": groups, "velocity_cutoff_grid": grid,
        "worst_rms": max(rows, key=lambda row: row["rms_db"]),
        "worst_spectral": max(rows, key=lambda row: row["spectral_db"]),
        "B_vs_C_pass_sets": {"identical": b_pass == c_pass,
            "new_C_passes": [{"case_id": case, "note": note} for case, note in sorted(c_pass - b_pass)],
            "lost_B_passes": [{"case_id": case, "note": note} for case, note in sorted(b_pass - c_pass)]},
        "metric_sha256": METRIC_SHA, "gain": 1, "rms_limit_db": 1, "spectral_limit_db": 3}
    write_json(HERE / "reports" / "C-STRATIFIED.json", strata)
    identity = {"scope": "Saved raw candidate PCM hash identity, not a rerender",
        "case_files": len(zero_hashes), "notes": len(zero_hashes) * 3,
        "identical_files": sum(row["identical"] for row in zero_hashes), "rows": zero_hashes}
    write_json(HERE / "reports" / "ZERO-RESONANCE-IDENTITY.json", identity)

    worst_rms_case = strata["worst_rms"]["case_id"]
    worst_spectral_case = strata["worst_spectral"]["case_id"]
    worst_tail = max(tails, key=lambda row: row["candidate_tail_rms"])
    diagnostic_cases = sorted({worst_rms_case, worst_spectral_case, worst_tail["case_id"],
                               "physical-gain/20_22050_.9", "physical-gain/100_2000_.9"})
    comparisons = []
    for case_id in diagnostic_cases:
        name = case_id.split("/", 1)[1]
        old_receipt = json.loads((B_AUDIO / (name + ".paired.receipt.json")).read_text())
        old_report = json.loads((B_AUDIO / (name + ".paired.json")).read_text())
        old_pair_bytes = (B_AUDIO / (name + ".paired.f32")).read_bytes()
        require(sha_bytes(old_pair_bytes) == old_report["pcm_sha256"], "B saved pair changed")
        require(old_receipt["reference_uncompressed_sha256"] == receipt_by_case[case_id]["reference_sha256"],
                "B and C references differ")
        old_pair = np.frombuffer(old_pair_bytes, dtype="<f4").reshape(3, FRAMES, 2)
        for i, note in enumerate(NOTES):
            c_tail = next(row for row in tails if row["case_id"] == case_id and row["note"] == note)
            b_tail = tail_levels(old_pair, i)
            comparisons.append({"case_id": case_id, "note": note,
                "reference_tail_rms": c_tail["reference_tail_rms"],
                "B_candidate_tail_rms": b_tail["candidate_tail_rms"],
                "C_candidate_tail_rms": c_tail["candidate_tail_rms"],
                "B_candidate_tail_peak": b_tail["candidate_tail_peak"],
                "C_candidate_tail_peak": c_tail["candidate_tail_peak"]})
    diagnostics = {"scope": "Descriptive last4096-frame levels from saved paired PCM only; no gate recomputation",
        "tail_frame_range": [FRAMES - TAIL, FRAMES], "note_off_frame": 66112,
        "tail_seconds_after_noteoff": [(FRAMES - TAIL - 66112) / 44100, (FRAMES - 66112) / 44100],
        "worst_C_tail_rms": worst_tail,
        "worst_C_tail_peak": max(tails, key=lambda row: row["candidate_tail_peak"]),
        "reference_tails_exactly_zero": sum(row["reference_tail_peak"] == 0 for row in tails),
        "C_vs_B_selected_tails": comparisons, "all_C_tail_rows": tails}
    write_json(HERE / "reports" / "C-TAIL-DIAGNOSTICS.json", diagnostics)
    require(sha(summary_path) == before_summary_sha, "Summary changed during analysis")
    print(json.dumps({"totals": strata["totals"], "groups": groups,
        "zero_resonance_hash_identity": {k: v for k, v in identity.items() if k != "rows"},
        "B_vs_C_pass_sets": strata["B_vs_C_pass_sets"], "worst_C_tail_rms": worst_tail,
        "reference_tails_exactly_zero": diagnostics["reference_tails_exactly_zero"],
        "selected_tail_comparisons": comparisons}, indent=2))


if __name__ == "__main__":
    main()
