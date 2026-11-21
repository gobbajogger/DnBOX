#!/usr/bin/env python3
"""One declared frozen-D active-filter diagnostic, after root release only.

This source-only preparation does not authorize execution. A later invocation
requires the newly captured original PCM and its reviewed SHA-256. One build,
one static replay, then (only after byte identity) one active capture and the
unchanged metric at explicit gain 1. Every output is new; failures are terminal.
Mandatory guards use explicit exceptions and remain active under python -O.
"""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import math
import os
import re
import shutil
import subprocess
import sys

HERE = Path(__file__).resolve().parent
FRAMES = 88192
MONO_BYTES = 3 * FRAMES * 4
METRIC_SHA = "bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1"
BASELINE_SHA = "7aded201280455c1ddaee86abab4d82b6d791953e81d0d9cb40e17946cbfa380"
D_SUMMARY_SHA = "9f4f5d42595c997c01af70b700c934403d86febacb2bf893745fc4934298f352"
ACTIVE_ORIGINAL_SHA = "3f7303aa1383c678e4e251c36506a141a0ece419e38d4411e2c0a0447bf0bb74"
ACTIVE_MANIFEST_SHA = "ea1a572e0d0b682a938fc83bdb7f7dd535bb645efd4b749576d039af64710044"
D_FILES = {
    "src/s1_dry_voice.c": "dab6afb5374a826bdd696f750709f22eaf658da8d9ddbf959646e95318cb43b1",
    "src/s1_dry_voice.h": "c18a55d812b5424b80fc611125d67d29b1297b59179ecb24b01f8807c3646ec2",
    "src/host.c": "068ed60874878844f65d0b90b4ac6b76597de3990962227f5ee74908ad0b7b07",
    "vendor/fastmath.h": "c82737d5302d77b974d5071c9bc58915aca723a6e6f5f5185cbe43bad962189d",
    "vendor/dnbx_s1_envelope.c": "4a5037f847f9488e3b44ee6252176ddfe5344380c6f59dc235d72cdf0c69caf9",
    "vendor/dnbx_s1_envelope.h": "6c949b9f93c7d0f60ee6938903341c10e224eb0857065bdc133892e80e641278",
}
FLAGS = ["-std=c11", "-O2", "-ffp-contract=off", "-fno-fast-math",
         "-Wall", "-Wextra", "-Werror", "-Wdouble-promotion", "-DX0X_HOST=1"]


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def now():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, value):
    with path.open("x") as stream:
        json.dump(value, stream, indent=2, allow_nan=False)
        stream.write("\n")


def verify_hash(path, expected, label):
    require(path.is_file(), label + " is not a regular file: " + str(path))
    actual = sha(path)
    require(actual == expected, label + " SHA-256 mismatch: " + actual)


def run_command(command, out, label, binary_sha=None):
    record = {"command": command, "cwd": str(out), "started_utc": now()}
    if binary_sha:
        record["binary_sha256_before_execution"] = binary_sha
    try:
        completed = subprocess.run(command, cwd=out, capture_output=True,
                                   text=True, timeout=30)
        record.update(exit_status=completed.returncode,
                      stdout=completed.stdout, stderr=completed.stderr)
    except subprocess.TimeoutExpired as error:
        def text(value):
            return value.decode(errors="replace") if isinstance(value, bytes) else value or ""
        record.update(exit_status=None, timed_out=True,
                      stdout=text(error.stdout), stderr=text(error.stderr))
    record["finished_utc"] = now()
    write_json(out / "logs" / (label + ".json"), record)
    require(record["exit_status"] == 0, label + " failed; terminal log retained")
    return record["stdout"]


def validate_receipt(receipt, mode, protocol):
    expected = protocol["modes"][mode]["control_hex"]
    checks = {"mode": mode, "frames_per_note": FRAMES, "notes": [36, 60, 84],
        "warmup_frames": 176448, "off_frame": 66112, "sample_rate_hz": 44100,
        "nonfinite": 0, "phase_at_note_on": 0, "comparison_gain": 1,
        "velocity": 100, "cutoff_hex": expected["cutoff"],
        "resonance_hex": expected["resonance"], "depth_hex": expected["depth"],
        "filter_ADSR_hex": expected["filter_ADSR"]}
    for key, value in checks.items():
        require(receipt.get(key) == value, mode + " host receipt mismatch: " + key)


def read_pcm(path, np, label):
    require(path.stat().st_size == MONO_BYTES, label + " has wrong byte count")
    pcm = np.fromfile(path, dtype="<f4").reshape(3, FRAMES)
    require(bool(np.isfinite(pcm).all()), label + " contains nonfinite samples")
    return pcm


def execute(args, out):
    import numpy as np
    frozen = json.loads((HERE / "DECLARATION.json").read_text())
    require(frozen.get("status") == "SOURCE_ONLY_PREPARED_NOT_COMPILED",
            "Wrong source-preparation declaration")
    for name, digest in frozen["source_files_sha256"].items():
        verify_hash(HERE / name, digest, "Frozen harness " + name)
    verify_hash(HERE / "metrics.py", METRIC_SHA, "Unchanged metric")
    protocol = json.loads((HERE / "inputs/PROTOCOL.json").read_text())
    metadata_path = HERE / "inputs/reference_capture_manifest.json"
    verify_hash(metadata_path, ACTIVE_MANIFEST_SHA, "Numeric active reference manifest")
    metadata = json.loads(metadata_path.read_text())
    require(metadata.get("classification") ==
        "NEW_ORIGINAL_REFERENCE_PCM; NOT_MISSING_HISTORICAL_PAIR_RECOVERY",
        "Active reference provenance classification changed")
    require(metadata.get("case_id") == "filter-modulation/1.2_2000_0.9",
        "Wrong active reference case")
    require(metadata["reference"]["sha256"] == ACTIVE_ORIGINAL_SHA and
        metadata["reference"]["bytes"] == MONO_BYTES,
        "Active reference identity/length contract mismatch")
    require(metadata["filter_ADSR"] == [0.2, 0.3, 0.4, 0.5] and
        metadata["filter_depth_f32"] == protocol["modes"]["active"]["filter_depth"],
        "Active reference physical filter contract mismatch")
    require(metadata["frames_per_note"] == FRAMES and
        metadata["note_off_frame_per_note"] == 66112 and
        metadata["actual_wrapper_receipt"]["warmup_frames"] == 3 * 176448,
        "Active reference event contract mismatch")
    d_root = args.d_root.resolve()
    for name, digest in D_FILES.items():
        verify_hash(d_root / name, digest, "Frozen D " + name)

    def verify_sources(stage):
        for name, digest in frozen["source_files_sha256"].items():
            verify_hash(HERE / name, digest, stage + " harness " + name)
        for name, digest in D_FILES.items():
            verify_hash(d_root / name, digest, stage + " D " + name)

    d_summary = d_root / "results/audio180/SUMMARY.json"
    verify_hash(d_summary, D_SUMMARY_SHA, "Completed D180 summary")
    baseline = (args.d_selected or d_root / "results/audio180/100_2000_.9.candidate-mono.f32le").resolve()
    verify_hash(baseline, BASELINE_SHA, "D selected static PCM")
    read_pcm(baseline, np, "D selected static PCM")
    original = args.new_original.resolve()
    require(original.suffix == ".f32le", "New original must be numeric .f32le PCM")
    require(original != baseline, "Active original cannot be the D baseline")
    require(re.fullmatch(r"[0-9a-f]{64}", args.new_original_sha256) is not None,
            "A reviewed lowercase SHA-256 for the new original is required")
    require(args.new_original_sha256 == ACTIVE_ORIGINAL_SHA,
            "This harness permits only the one pinned new active original")
    verify_hash(original, args.new_original_sha256, "New active original PCM")
    reference_pcm = read_pcm(original, np, "New active original PCM")
    blocked = [key for key in ("CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH",
        "LIBRARY_PATH", "COMPILER_PATH", "GCC_EXEC_PREFIX", "LD_PRELOAD",
        "LD_LIBRARY_PATH") if os.environ.get(key)]
    require(not blocked, "Unpinned compiler environment: " + ", ".join(blocked))
    compiler_name = shutil.which("cc")
    require(compiler_name is not None, "Host cc is unavailable")
    compiler = Path(compiler_name).resolve()
    verify_hash(compiler, frozen["compiler_sha256"], "Pinned host compiler")
    executable = out / "active_host"
    static_pcm = out / "static.candidate-mono.f32le"
    active_pcm = out / "active.candidate-mono.f32le"
    build = [str(compiler), *FLAGS, "-I" + str(d_root / "vendor"),
        "-I" + str(d_root / "src"), str(d_root / "src/s1_dry_voice.c"),
        str(d_root / "vendor/dnbx_s1_envelope.c"),
        str(HERE / "src/active_host.c"), "-o", str(executable)]
    commands = {"build": build,
        "static": [str(executable), "static", str(static_pcm)],
        "active": [str(executable), "active", str(active_pcm)]}
    run_declaration = {"status": "BEFORE_ONE_HOST_BUILD_OR_RENDER", "written_utc": now(),
        "source_preparation_declaration_sha256": sha(HERE / "DECLARATION.json"),
        "source_files_sha256": frozen["source_files_sha256"],
        "D_files_sha256": D_FILES, "D180_summary_sha256": D_SUMMARY_SHA,
        "new_original_numeric_manifest_sha256": ACTIVE_MANIFEST_SHA,
        "D_selected_static_pcm": {"path": str(baseline), "sha256": BASELINE_SHA},
        "new_original_active_pcm": {"path": str(original), "sha256": args.new_original_sha256,
            "bytes": MONO_BYTES, "origin": "new capture, not recovered historical active PCM"},
        "protocol": protocol, "gate": frozen["gate"],
        "compiler": str(compiler), "compiler_sha256": sha(compiler),
        "python": sys.version, "numpy_version": np.__version__,
        "commands_serial": commands, "maximum_host_builds": 1,
        "maximum_static_replays": 1, "maximum_active_captures": 1,
        "precondition_for_active": "static replay is byte-identical to pinned D selected PCM",
        "production_activation": False}
    write_json(out / "RUN-DECLARATION.json", run_declaration)
    run_command(build, out, "build-attempt01")
    binary_sha = sha(executable)
    verify_sources("After build")
    write_json(out / "PRE-RENDER-DECLARATION.json", {
        "status": "BUILT_NOT_RENDERED", "written_utc": now(),
        "run_declaration_sha256": sha(out / "RUN-DECLARATION.json"),
        "binary_sha256": binary_sha, "commands": commands,
        "source_files_sha256": frozen["source_files_sha256"],
        "new_original_sha256": args.new_original_sha256,
        "baseline_sha256": BASELINE_SHA, "metric_sha256": METRIC_SHA,
        "comparison_gain": 1})
    static_receipt = json.loads(run_command(commands["static"], out,
                                           "static-attempt01", binary_sha))
    validate_receipt(static_receipt, "static", protocol)
    read_pcm(static_pcm, np, "Static replay")
    static_hash = sha(static_pcm)
    static_exact = static_hash == BASELINE_SHA and static_pcm.read_bytes() == baseline.read_bytes()
    write_json(out / "STATIC-IDENTITY.json", {"status": "PASS" if static_exact else "FAIL",
        "baseline_sha256": BASELINE_SHA, "replay_sha256": static_hash,
        "byte_identical": static_exact, "receipt": static_receipt})
    require(static_exact, "Static replay differs from frozen D; active capture is forbidden")
    verify_hash(executable, binary_sha, "Host before active capture")
    verify_sources("Before active capture")
    active_receipt = json.loads(run_command(commands["active"], out,
                                           "active-attempt01", binary_sha))
    validate_receipt(active_receipt, "active", protocol)
    candidate_pcm = read_pcm(active_pcm, np, "Active D candidate")
    verify_hash(original, args.new_original_sha256, "Original after capture")
    verify_sources("After capture")
    verify_hash(executable, binary_sha, "Host after capture")
    verify_hash(HERE / "metrics.py", METRIC_SHA, "Metric before comparison")
    paired_path = out / "active.paired.f32le"
    paired = np.empty((3, FRAMES, 2), dtype="<f4")
    paired[:, :, 0] = reference_pcm
    paired[:, :, 1] = candidate_pcm
    with paired_path.open("xb") as stream:
        stream.write(paired.tobytes(order="C"))
    metric_json = paired_path.with_suffix(".json")
    require(not metric_json.exists(), "Metric result exists; overwrite forbidden")
    metric_command = [sys.executable, str(HERE / "metrics.py"), str(paired_path), "1"]
    write_json(out / "PRE-METRIC-DECLARATION.json", {
        "written_utc": now(), "metric_sha256": METRIC_SHA, "command": metric_command,
        "paired_sha256": sha(paired_path), "paired_columns": ["original", "candidate"],
        "original_sha256": args.new_original_sha256,
        "candidate_sha256": sha(active_pcm), "gain": 1,
        "normalization": "none", "time_alignment": "none"})
    run_command(metric_command, out, "metric-attempt01")
    measured = json.loads(metric_json.read_text())
    require(measured.get("gain") == 1, "Metric did not use explicit gain 1")
    require(measured.get("nonfinite") == 0, "Metric contains nonfinite PCM")
    require(measured.get("pcm_sha256") == sha(paired_path), "Metric PCM identity changed")
    require([r.get("note") for r in measured.get("rows", [])] == [36, 60, 84],
            "Metric note order/count changed")
    for row in measured["rows"]:
        require(math.isfinite(row["rms_db"]) and math.isfinite(row["spectral_db"]),
                "Nonfinite metric result")
    accepted = all(row["rms_db"] <= 1 and row["spectral_db"] <= 3
                   for row in measured["rows"])
    verify_sources("After metric")
    summary = {"status": "PASS" if accepted else "FAIL", "finished_utc": now(),
        "scope": "One new active-filter three-note diagnostic, not a 72-case or full-engine gate",
        "case_id": "filter-modulation/1.2_2000_0.9", "gain": 1,
        "static_replay_byte_identical": True, "gate": frozen["gate"],
        "rows": measured["rows"], "nonfinite": measured["nonfinite"],
        "accepted_notes": sum(row["rms_db"] <= 1 and row["spectral_db"] <= 3
                              for row in measured["rows"]),
        "new_original_sha256": args.new_original_sha256,
        "candidate_sha256": sha(active_pcm), "paired_sha256": sha(paired_path),
        "host_binary_sha256": binary_sha, "metric_sha256": METRIC_SHA,
        "counts": {"host_builds": 1, "static_replays": 1, "active_captures": 1,
                   "metric_executions": 1},
        "D_source_unchanged": True, "production_activation": False,
        "failure_policy": "Terminal result; no adjustment, automatic repeat or new variant"}
    write_json(out / "SUMMARY.json", summary)
    print(json.dumps(summary, indent=2))
    return 0 if accepted else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--d-root", type=Path, default=HERE.parent / "s1_dry_voice_d")
    parser.add_argument("--d-selected", type=Path,
                        help="Optional relocated copy; exact frozen baseline hash still required")
    parser.add_argument("--new-original", type=Path, required=True)
    parser.add_argument("--new-original-sha256", required=True)
    parser.add_argument("--output", type=Path, default=HERE / "results/active-one")
    args = parser.parse_args()
    out = args.output.resolve()
    require(out.parent == (HERE / "results").resolve(),
            "Output must be a new direct child of this follow-up's results directory")
    require(not out.exists(), "Output exists; every failure is terminal and overwrite is forbidden")
    out.mkdir(parents=True, exist_ok=False)
    (out / "logs").mkdir(exist_ok=False)
    try:
        return execute(args, out)
    except Exception as error:
        failure = {"status": "TERMINAL_HARNESS_FAILURE", "finished_utc": now(),
            "error_type": type(error).__name__, "error": str(error),
            "automatic_retry": False, "artifacts_preserved": True,
            "production_activation": False}
        write_json(out / "FAILURE.json", failure)
        print(json.dumps(failure, indent=2), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
