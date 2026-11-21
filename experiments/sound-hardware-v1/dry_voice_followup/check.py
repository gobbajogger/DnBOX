#!/usr/bin/env python3
"""Portable, fresh-output host comparison against immutable dry reference PCM.

Historical runners/declarations remain evidence only. This entry uses explicit
checks under normal or optimized Python, never resumes, and runs the unchanged
metric with gain 1. No original engine, native build, emulator or timing task.
"""
from pathlib import Path
import argparse
import datetime
import gzip
import hashlib
import json
import math
import os
import shutil
import subprocess
import sys

import numpy as np

HERE = Path(__file__).resolve().parent
MANIFEST_SHA = "2f16ef62c422d2bbd8100271790d6aa0a253d33e77bb126c2ceb79b2f9a8b3c1"
PLAN_SHA = "e8049a0247019d211ec9f96636073e6839ee321135fdf16001065dd54b565a9d"
METRIC_SHA = "bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1"
NOTES = [36, 60, 84]
MONO_BYTES = 3 * 88192 * 4
SELECTED = "physical-gain/100_2000_.9"
FLAGS = ["-std=c11", "-O2", "-ffp-contract=off", "-fno-fast-math",
         "-Wall", "-Wextra", "-Werror", "-Wdouble-promotion", "-DX0X_HOST=1",
         "-Ivendor", "-Isrc"]


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def sha(path):
    return digest(path.read_bytes())


def write_json(path, value):
    with path.open("x") as stream:
        json.dump(value, stream, indent=2, allow_nan=False)
        stream.write("\n")


def fresh_directory(path):
    # Test the lexical path before resolving: a dangling symlink also exists.
    path = path.absolute()
    require(not os.path.lexists(path), "Output already exists (including symlinks)")
    require(all(not p.is_symlink() for p in path.parents), "Symlink output parent")
    path.mkdir(parents=True, exist_ok=False)
    return path


def load_inputs(variant, reference_root, all_cases, case_id):
    manifest_data = (HERE / "FROZEN-INPUTS.json").read_bytes()
    require(digest(manifest_data) == MANIFEST_SHA, "Frozen manifest changed")
    manifest = json.loads(manifest_data)
    require(variant in manifest["variants"], "Unknown variant")
    meta = manifest["variants"][variant]
    source = HERE / meta["source_root"]
    source_bytes = {}
    for name, expected in meta["files"].items():
        data = (source / name).read_bytes()
        require(digest(data) == expected, "Frozen source changed: " + name)
        source_bytes[name] = data
    packet = HERE / "reference_packet"
    plan_data = (packet / "case_plan.json").read_bytes()
    metric_data = (packet / "metrics.py").read_bytes()
    require(digest(plan_data) == PLAN_SHA, "Case plan changed")
    require(digest(metric_data) == METRIC_SHA, "Metric changed")
    plan = json.loads(plan_data)
    cases = [c for c in plan["cases"] if c["case_id"].startswith("physical-gain/")]
    require(len(cases) == 60 and len({c["case_id"] for c in cases}) == 60,
            "Expected 60 unique physical cases")
    require([n["midi_note"] for n in plan["notes"]] == NOTES, "Note coverage changed")
    require(plan["comparison_gain"] == 1 and plan["frames_per_note"] == 88192
            and plan["note_off_frame"] == 66112 and plan["sample_rate_hz"] == 44100
            and plan["warmup_frames_per_note"] == 176448, "Scene changed")
    if not all_cases:
        cases = [c for c in cases if c["case_id"] == case_id]
        require(len(cases) == 1, "Unknown physical case")
    cases.sort(key=lambda c: (c["case_id"] != SELECTED, c["case_id"]))
    for case in cases:
        name = Path(case["current_reference_mono_relative_path"]).name
        data = (reference_root / name).read_bytes()
        require(len(data) == MONO_BYTES, "Reference length changed: " + name)
        require(digest(data) == case["current_reference_mono_sha256"],
                "Reference changed: " + name)
        require(np.isfinite(np.frombuffer(data, "<f4")).all(), "Nonfinite reference")
    return meta, source_bytes, metric_data, cases


def validate_report(report, pair_bytes):
    require(len(pair_bytes) == MONO_BYTES * 2, "Paired PCM length changed")
    require(report.get("pcm_sha256") == digest(pair_bytes), "Report PCM hash mismatch")
    require(report.get("gain") == 1, "Comparison gain must be exactly 1")
    rows = report.get("rows", [])
    require([r.get("note") for r in rows] == NOTES, "Missing, duplicated or reordered note")
    actual_nonfinite = int((~np.isfinite(np.frombuffer(pair_bytes, "<f4"))).sum())
    require(report.get("nonfinite") == actual_nonfinite, "Nonfinite count mismatch")
    for row in rows:
        require(all(isinstance(row.get(key), (float, int))
                    and not isinstance(row[key], bool)
                    and math.isfinite(row[key]) and row[key] >= 0
                    for key in ("rms_db", "spectral_db")), "Invalid metric row")
    return [actual_nonfinite == 0 and r["rms_db"] <= 1 and r["spectral_db"] <= 3
            for r in rows]


def command_run(command, directory, log, timeout=30):
    started = datetime.datetime.now(datetime.timezone.utc).isoformat()
    result = subprocess.run(command, cwd=directory, capture_output=True, text=True,
                            timeout=timeout)
    with log.open("x") as stream:
        stream.write(json.dumps({"command": command, "cwd": str(directory),
                                "started_utc": started, "exit_status": result.returncode}) + "\n")
        stream.write(result.stdout + result.stderr)
    require(result.returncode == 0, "Command failed; retained log: " + str(log))
    return result.stdout


def build_host(source_bytes, output):
    blocked = [key for key in ("CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH",
        "LIBRARY_PATH", "COMPILER_PATH", "GCC_EXEC_PREFIX", "LD_PRELOAD", "LD_LIBRARY_PATH")
        if os.environ.get(key)]
    require(not blocked, "Unpinned compiler environment: " + ", ".join(blocked))
    compiler = shutil.which("cc")
    require(compiler is not None, "Host C compiler is unavailable")
    compiler = str(Path(compiler).resolve())
    build = output / "build"
    build.mkdir()
    for name in ("src/s1_dry_voice.c", "src/s1_dry_voice.h", "src/host.c",
                 "vendor/fastmath.h", "vendor/dnbx_s1_envelope.c", "vendor/dnbx_s1_envelope.h"):
        target = build / name
        target.parent.mkdir(parents=True, exist_ok=True)
        with target.open("xb") as stream:
            stream.write(source_bytes[name])
        require(sha(target) == digest(source_bytes[name]), "Staged source differs")
    executable = build / "s1_dry_host"
    command = [compiler, *FLAGS, "src/s1_dry_voice.c", "vendor/dnbx_s1_envelope.c",
               "src/host.c", "-o", "s1_dry_host"]
    command_run(command, build, output / "build.log")
    version = command_run([compiler, "--version"], build, output / "compiler.log")
    description = json.loads(command_run([str(executable), "--describe"], build,
                                         output / "describe.log"))
    test = json.loads(command_run([str(executable), "--selftest"], build,
                                  output / "selftest.log"))
    require(test.get("status") == "PASS", "Host invariants failed")
    require(description.get("sample_rate_hz") == 44100, "Host rate differs")
    require(description.get("coefficient_hex") ==
            ["3f7fce7c", "3f7ed775", "3f7fe249", "3f7fce7c", "3f7fe249", "3f7ffd07"],
            "Frozen envelope coefficients differ")
    return executable, {"compiler": compiler, "compiler_sha256": sha(Path(compiler)),
                        "compiler_version": version, "command": command,
                        "executable_sha256": sha(executable), "description": description}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--variant", required=True)
    ap.add_argument("--reference-root", required=True, type=Path,
                    help="Directory containing the 60 original-mono recordings")
    ap.add_argument("--output", type=Path, help="New directory; never resumed or overwritten")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--case", default=SELECTED)
    ap.add_argument("--verify-only", action="store_true")
    ap.add_argument("--saved-candidates", type=Path,
                    help="Remeasure frozen candidate PCM without building or rendering")
    args = ap.parse_args()
    meta, source_bytes, metric_data, cases = load_inputs(args.variant, args.reference_root,
                                                       args.all, args.case)
    if args.verify_only:
        print(json.dumps({"status": "INPUTS_VERIFIED_NOT_RUN", "files": len(cases),
                          "variant": meta["id"], "python_optimization": sys.flags.optimize}))
        return 0
    require(args.output is not None, "A new --output directory is required")
    output = fresh_directory(args.output)
    metric_path = output / "metrics.py"
    with metric_path.open("xb") as stream:
        stream.write(metric_data)
    require(sha(metric_path) == METRIC_SHA, "Staged metric differs")
    executable = None
    historical = None
    if args.saved_candidates:
        require("historical_reports_sha256" in meta,
                "No historical saved-candidate bundle is supplied for this variant")
        historical_path = HERE / "evidence" / args.variant / "all_reports.json.gz"
        historical_data = historical_path.read_bytes()
        require(digest(historical_data) == meta["historical_reports_sha256"],
                "Historical report bundle changed")
        historical = json.loads(gzip.decompress(historical_data))
        execution = {"mode": "remeasure saved PCM", "audio_render": "NOT_RUN"}
    else:
        executable, execution = build_host(source_bytes, output)
        require(execution["description"]["voice_bytes"] == meta["expected_host_bytes"],
                "Unexpected host object size")
    write_json(output / "DECLARATION.json", {
        "written_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "status": "BEFORE_FIRST_COMPARISON", "variant": meta,
        "manifest_sha256": MANIFEST_SHA, "plan_sha256": PLAN_SHA,
        "checker_sha256": sha(Path(__file__)),
        "metric_sha256": METRIC_SHA, "execution": execution,
        "case_ids": [c["case_id"] for c in cases], "gain": 1,
        "rms_limit_db": 1, "spectral_limit_db": 3, "nonfinite_limit": 0,
        "alignment": "none", "normalization": "none", "numpy_version": np.__version__})
    rows = []
    receipts = []
    for case in cases:
        name = case["case_id"].split("/", 1)[1]
        candidate = output / (name + ".candidate-mono.f32le")
        if historical is not None:
            candidate_bytes = (args.saved_candidates / candidate.name).read_bytes()
            old_receipt = json.loads(historical[name + ".paired.receipt.json"])
            require(old_receipt["case_id"] == case["case_id"]
                    and old_receipt["candidate_sha256"] == digest(candidate_bytes),
                    "Saved candidate differs from historical receipt")
        else:
            command_run([str(executable), str(candidate), str(case["velocity"]),
                repr(case["cutoff_hz"]), repr(case["resonance"]), repr(case["filter_depth"])],
                output, output / (name + ".render.log"), timeout=15)
            candidate_bytes = candidate.read_bytes()
        require(len(candidate_bytes) == MONO_BYTES, "Candidate length changed")
        reference = args.reference_root / Path(case["current_reference_mono_relative_path"]).name
        reference_bytes = reference.read_bytes()
        require(digest(reference_bytes) == case["current_reference_mono_sha256"],
                "Reference changed during comparison")
        pair = np.empty((3 * 88192, 2), dtype="<f4")
        pair[:, 0] = np.frombuffer(reference_bytes, "<f4")
        pair[:, 1] = np.frombuffer(candidate_bytes, "<f4")
        pair_bytes = pair.tobytes()
        paired = output / (name + ".paired.f32")
        with paired.open("xb") as stream:
            stream.write(pair_bytes)
        require(sha(metric_path) == METRIC_SHA, "Staged metric changed")
        command_run([sys.executable, str(metric_path), str(paired), "1"],
                    output, output / (name + ".metric.log"), timeout=15)
        report = json.loads(paired.with_suffix(".json").read_text())
        accepted = validate_report(report, pair_bytes)
        receipt = {"case_id": case["case_id"], "reference_sha256": digest(reference_bytes),
                   "candidate_sha256": digest(candidate_bytes), "paired_sha256": digest(pair_bytes),
                   "report_sha256": sha(paired.with_suffix(".json")), "metric_sha256": METRIC_SHA,
                   "gain": 1, "accepted": all(accepted), "notes": NOTES}
        write_json(output / (name + ".receipt.json"), receipt)
        receipts.append(receipt)
        rows.extend({"case_id": case["case_id"], "velocity": case["velocity"],
                     "cutoff_hz": case["cutoff_hz"], "resonance": case["resonance"],
                     **row, "accepted": ok} for row, ok in zip(report["rows"], accepted))
        print(f"{case['case_id']}: {'PASS' if all(accepted) else 'FAIL'}", flush=True)
    if executable is not None:
        require(sha(executable) == execution["executable_sha256"], "Executable changed")
    require(len(rows) == len(cases) * 3, "Incomplete comparison")
    summary = {"scope": "Static dry voice only; zero intact preset coverage",
        "variant": meta["id"], "case_files": len(cases), "note_cases": len(rows),
        "accepted_files": sum(r["accepted"] for r in receipts),
        "accepted_notes": sum(r["accepted"] for r in rows),
        "maximum_rms_db": max(r["rms_db"] for r in rows),
        "maximum_spectral_db": max(r["spectral_db"] for r in rows), "rows": rows,
        "accepted": all(r["accepted"] for r in rows),
        "unqualified": ["72 dynamic cases", "694 intact presets", "150 controls",
                        "native fit", "callback deadline", "production activation"]}
    write_json(output / "SUMMARY.json", summary)
    print(json.dumps({k: v for k, v in summary.items() if k != "rows"}, indent=2))
    return 0 if summary["accepted"] else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (RuntimeError, OSError, ValueError, KeyError, subprocess.TimeoutExpired) as error:
        print("COMPARISON ERROR: " + str(error), file=sys.stderr)
        raise SystemExit(2)
