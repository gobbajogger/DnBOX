#!/usr/bin/env python3
"""One host wrapper build and exactly eight serial matched C response records."""
from pathlib import Path
import datetime
import hashlib
import json
import os
import shutil
import subprocess

import numpy as np

HERE = Path(__file__).resolve().parent
CANDIDATE = HERE.parent / "s1_dry_voice_c"
PACKET = HERE.parent / "reference_filter_inputs" / "host_observation_20261119"


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, value):
    with path.open("x") as stream:
        json.dump(value, stream, indent=2, allow_nan=False); stream.write("\n")


def main():
    require(not (HERE / "BUILD-DECLARATION.json").exists(), "Probe already begun; no repeat build or capture")
    observed = json.loads((PACKET / "observations.json").read_text())
    manifest = json.loads((PACKET / "metadata" / "input_pcm_manifest.json").read_text())
    require(len(observed["rows"]) == 8 and len(manifest["records"]) == 4, "Unexpected packet coverage")
    frozen = json.loads((CANDIDATE / "DECLARATION.json").read_text())
    source_names = ["src/s1_dry_voice.c", "src/s1_dry_voice.h", "vendor/fastmath.h",
                    "vendor/dnbx_s1_envelope.c", "vendor/dnbx_s1_envelope.h"]
    for name in source_names:
        require(sha(CANDIDATE / name) == frozen["source_files_sha256"][name], "Frozen C changed")
    blocked = [key for key in ["CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH", "LIBRARY_PATH",
        "COMPILER_PATH", "GCC_EXEC_PREFIX", "LD_PRELOAD", "LD_LIBRARY_PATH"] if os.environ.get(key)]
    require(not blocked, "Unpinned compiler environment")
    compiler = Path(shutil.which("cc")).resolve()
    executable = HERE / "build" / "filter_probe"
    command = [str(compiler), "-std=c11", "-O2", "-ffp-contract=off", "-fno-fast-math",
        "-Wall", "-Wextra", "-Werror", "-Wdouble-promotion", "-DX0X_HOST=1",
        "-I" + str(CANDIDATE / "src"), "-I" + str(CANDIDATE / "vendor"),
        str(HERE / "src" / "filter_probe.c"), str(CANDIDATE / "vendor" / "dnbx_s1_envelope.c"),
        "-o", str(executable)]
    cases = []
    for row in observed["rows"]:
        require(row["sample_rate_hz"] == 44100 and row["cutoff_f32_hex"] == "44fa0000"
            and row["prewarm_frames"] == 4096 and row["retained_frames"] == 32768
            and row["steady_window_start"] == 16384 and row["steady_window_end_exclusive"] == 32768
            and row["initial_state_fresh"] and row["resonance_f32_hex"] in ["00000000", "3f666666"], "Protocol differs")
        input_path, reference_path = Path(row["input"]["path"]), Path(row["output"]["path"])
        require(input_path.parent.resolve() == (PACKET / "inputs").resolve()
                and reference_path.parent.resolve() == (PACKET / "outputs").resolve(), "Non-numeric packet path")
        require(input_path.stat().st_size == 131072 and reference_path.stat().st_size == 131072, "Wrong stream length")
        require(sha(input_path) == row["input"]["sha256"] and sha(reference_path) == row["output"]["sha256"], "Packet stream changed")
        supplied = next(item for item in manifest["records"] if item["sha256"] == row["input"]["sha256"])
        require(supplied["amplitude_f32_hex"] == row["input_amplitude_f32_hex"], "Input identity differs")
        output = HERE / "outputs" / (row["case_id"] + ".C-filter.f32le")
        cases.append({key: row[key] for key in ["case_id", "input_kind", "input_amplitude_f32", "input_amplitude_f32_hex",
            "resonance_f32", "resonance_f32_hex"]} | {"input_path": str(input_path),
            "input_sha256": sha(input_path), "reference_path": str(reference_path), "reference_sha256": sha(reference_path),
            "candidate_output_path": str(output), "command": [str(executable), str(input_path), str(output), row["resonance_f32_hex"]]})
    require(len({case["case_id"] for case in cases}) == 8 and len({case["input_sha256"] for case in cases}) == 4,
            "Unexpected case/input identity")
    declaration = {"schema": "matched C filter-only pre-execution declaration v1",
        "written_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "status": "BEFORE_WRAPPER_BUILD_AND_ALL_EIGHT_RESPONSE_RECORDS",
        "build_command": command, "compiler_sha256": sha(compiler),
        "wrapper_source_sha256": sha(HERE / "src" / "filter_probe.c"),
        "driver_sha256": sha(Path(__file__)), "analysis_source_sha256": sha(HERE / "analyze_probe.py"),
        "frozen_C_source_files_sha256": {name: sha(CANDIDATE / name) for name in source_names},
        "numeric_packet_files_sha256": {name: sha(PACKET / name) for name in ["observations.json", "metadata/input_pcm_manifest.json"]},
        "protocol": {"sample_rate_hz": 44100, "cutoff_hz": 2000, "fresh_owner_per_record": True,
            "resonance_f32_hex": ["00000000", "3f666666"], "silent_prewarm_frames": 4096,
            "retained_frames": 32768, "steady_window": [16384, 32768], "response_records": 8,
            "filter_path": "unchanged frozen C private filter_substep twice with held input, mean of two outputs",
            "oscillator_VCA_envelope_advance": False, "gain_alignment_fitting_normalization": "none"},
        "diagnostic_method": {"acceptance_gate": "none; component diagnostic, not historic three-note gate",
            "rms": "unmodified whole and16384:32768 input/output RMS and20log10 output/input ratio; null when input RMS zero",
            "sine_spectral": "fixed2000Hz and harmonics2-5 projections in steady Hann window; report transfer ratio/phase and harmonics relative to own fundamental; no fitting or waveform adjustment",
            "impulse_spectral": "whole unwindowed fixed-frequency response ratios at0,500,1000,2000,4000,8000,16000,22050Hz",
            "reference_differences": "unshifted direct differences and measured ratio differences only",
            "feedback_argument": "read-only reconstructed(current output+4old1+6old2+4old3+old4)/16; not exact traced solver d; never fed to DSP"},
        "source_boundary": "Only frozen independent C and listed source-free numeric observations/manifests/PCM; no original capture source, headers, objects or engine algorithms read",
        "cases": cases}
    write_json(HERE / "BUILD-DECLARATION.json", declaration)
    build = subprocess.run(command, cwd=HERE, text=True, capture_output=True, timeout=30)
    write_json(HERE / "logs" / "build.json", {"command": command, "exit_status": build.returncode,
        "stdout": build.stdout, "stderr": build.stderr})
    require(build.returncode == 0, "Only authorized wrapper build failed; log preserved")
    declaration["status"] = "COMPILED_BEFORE_FIRST_RESPONSE_RECORD"
    declaration["executable_sha256"] = sha(executable)
    declaration["build_log_sha256"] = sha(HERE / "logs" / "build.json")
    write_json(HERE / "DECLARATION.json", declaration)
    results = []
    for case in cases:
        capture = subprocess.run(case["command"], cwd=HERE, text=True, capture_output=True, timeout=30)
        log_path = HERE / "logs" / (case["case_id"] + ".json")
        write_json(log_path, {"command": case["command"], "exit_status": capture.returncode,
            "stdout": capture.stdout, "stderr": capture.stderr})
        require(capture.returncode == 0, "Response record failed; output/log preserved")
        output = Path(case["candidate_output_path"])
        require(output.stat().st_size == 131072 and np.isfinite(np.fromfile(output, "<f4")).all(), "Bad candidate output")
        results.append({"case_id": case["case_id"], "candidate_path": str(output),
            "candidate_sha256": sha(output), "log_sha256": sha(log_path), "wrapper_result": json.loads(capture.stdout)})
        print(case["case_id"] + ": captured", flush=True)
    require(len(results) == 8 and sha(executable) == declaration["executable_sha256"], "Capture count/binary changed")
    for name, expected in declaration["frozen_C_source_files_sha256"].items():
        require(sha(CANDIDATE / name) == expected, "C source changed during probe")
    for case in cases:
        require(sha(Path(case["input_path"])) == case["input_sha256"]
                and sha(Path(case["reference_path"])) == case["reference_sha256"], "Numeric input changed during probe")
    write_json(HERE / "CAPTURE-RESULT.json", {"status": "CAPTURE_COMPLETE", "response_records": 8,
        "wrapper_builds": 1, "serial_execution": True, "candidate_DSP_changed": False,
        "declaration_sha256": sha(HERE / "DECLARATION.json"), "rows": results})
    print("Eight matched component records complete; no acceptance gate asserted.")


if __name__ == "__main__":
    main()
