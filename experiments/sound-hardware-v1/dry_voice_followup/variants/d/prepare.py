#!/usr/bin/env python3
"""One declared, serial HOST preparation of D. No reference audio execution.

Writes new files exclusively; an existing run is never silently repeated.
The unchanged ownership test generates bounded in-memory samples only.
"""
from pathlib import Path
import datetime
import hashlib
import json
import os
import shutil
import subprocess

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
C = ROOT / "s1_dry_voice_c"
SIX_FILES = ["src/s1_dry_voice.c", "src/s1_dry_voice.h", "src/host.c",
             "vendor/fastmath.h", "vendor/dnbx_s1_envelope.c",
             "vendor/dnbx_s1_envelope.h"]
SOURCE_FILES = SIX_FILES + ["tests/ownership_test.c", "prepare.py",
    "inputs/NONLINEAR-HYPOTHESIS.md", "inputs/C-filter-observations.json",
    "inputs/reference_pcm_manifest.json", "reports/C-to-D.patch"]
EXPECTED_C_CORE = "29e04eb07c512bb390bf6f7cc7b9429baf46cf927db126bced2f1fffe1ae1bf9"
EXPECTED_D_CORE = "dab6afb5374a826bdd696f750709f22eaf658da8d9ddbf959646e95318cb43b1"
EXPECTED_OWNERSHIP = "bf7bba35b180e3291146b7267ccd4c16ec3286126c64478ad73ee72cfa81450e"
METRIC_SHA = "bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1"


def now():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, value):
    with path.open("x") as stream:
        json.dump(value, stream, indent=2, allow_nan=False)
        stream.write("\n")


def run(command, label, binary_sha=None):
    receipt = {"command": command, "cwd": str(HERE), "started_utc": now()}
    if binary_sha:
        receipt["binary_sha256_before_execution"] = binary_sha
    try:
        p = subprocess.run(command, cwd=HERE, capture_output=True, text=True,
                           timeout=30)
        receipt.update(exit_status=p.returncode, stdout=p.stdout, stderr=p.stderr)
    except subprocess.TimeoutExpired as error:
        receipt.update(exit_status=None, timed_out=True,
            stdout=(error.stdout or b"").decode() if isinstance(error.stdout, bytes)
                else error.stdout or "",
            stderr=(error.stderr or b"").decode() if isinstance(error.stderr, bytes)
                else error.stderr or "")
    receipt["finished_utc"] = now()
    write_json(HERE / "logs" / (label + "-attempt01.json"), receipt)
    if receipt["exit_status"] != 0:
        raise RuntimeError("Failed command retained: " + label)
    return receipt["stdout"]


def main():
    if (HERE / "BUILD-DECLARATION.json").exists():
        raise RuntimeError("Existing preparation is frozen; no automatic repeat")
    c_files = json.loads((C / "COMPARATOR-SOURCE.json").read_text())["files"]
    assert c_files["src/s1_dry_voice.c"] == EXPECTED_C_CORE
    for name, digest in c_files.items():
        assert sha(C / name) == digest, "Frozen C source changed: " + name
    assert sha(HERE / "src/s1_dry_voice.c") == EXPECTED_D_CORE
    assert sha(HERE / "tests/ownership_test.c") == EXPECTED_OWNERSHIP
    assert (HERE / "tests/ownership_test.c").read_bytes() == (
        ROOT / "s1_c_ownership/tests/ownership_test.c").read_bytes()
    for name in SIX_FILES:
        if name != "src/s1_dry_voice.c":
            assert (HERE / name).read_bytes() == (C / name).read_bytes()
    c_core = (C / "src/s1_dry_voice.c").read_text()
    d_core = (HERE / "src/s1_dry_voice.c").read_text()
    branch_start = "    if (R == 0.0f) {"
    branch_end = "    /* Compute the forward cascade"
    branch = lambda text: text[text.index(branch_start):text.index(branch_end)]
    assert branch(c_core) == branch(d_core)
    blocked = [key for key in ("CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH",
        "LIBRARY_PATH", "COMPILER_PATH", "GCC_EXEC_PREFIX", "LD_PRELOAD",
        "LD_LIBRARY_PATH") if os.environ.get(key)]
    if blocked:
        raise RuntimeError("Unpinned compiler environment: " + ", ".join(blocked))
    compiler = Path(shutil.which("cc")).resolve()
    flags = ["-std=c11", "-O2", "-ffp-contract=off", "-fno-fast-math",
        "-Wall", "-Wextra", "-Werror", "-Wdouble-promotion", "-DX0X_HOST=1",
        "-Ivendor", "-Isrc"]
    builds = {
        "build-core": [str(compiler), *flags, "-c", "src/s1_dry_voice.c",
                       "-o", "build/s1_dry_voice.o"],
        "build-host": [str(compiler), *flags, "build/s1_dry_voice.o",
            "vendor/dnbx_s1_envelope.c", "src/host.c", "-o", "build/s1_dry_host"],
        "build-ownership": [str(compiler), *flags, "build/s1_dry_voice.o",
            "vendor/dnbx_s1_envelope.c", "tests/ownership_test.c",
            "-o", "build/ownership_test"]}
    tests = {
        "describe": [str(HERE / "build/s1_dry_host"), "--describe"],
        "host-selftest": [str(HERE / "build/s1_dry_host"), "--selftest"],
        "ownership-selftest": [str(HERE / "build/ownership_test")]}
    packet = ROOT / "dry_reference_packet"
    packet_hashes = {name: sha(packet / name) for name in
        ("case_plan.json", "reference_pcm_manifest.json", "metrics.py")}
    assert packet_hashes["metrics.py"] == METRIC_SHA
    source_hashes = {name: sha(HERE / name) for name in SOURCE_FILES}
    variant = {"id": "d_ch4_centered_linear_feedback", "expected_host_bytes": 160,
        "source_root_for_root_to_register": str(HERE),
        "files": {name: source_hashes[name] for name in SIX_FILES},
        "historical_audio_reports": None,
        "audio_status": "NO_REFERENCE_COMPARISON_OR_PCM_CAPTURE"}
    write_json(HERE / "COMPARATOR-SOURCE.json", variant)
    gate = {"comparison_gain": 1, "rms_limit_db": 1, "spectral_limit_db": 3,
        "nonfinite_limit": 0, "alignment": "none", "normalization": "none",
        "metric_sha256": METRIC_SHA}
    declaration = {"schema": "Independent S1 Variant D pre-build freeze v1",
        "written_utc": now(), "status": "BEFORE_HOST_BUILD_OR_CANDIDATE_EXECUTION",
        "variant": variant,
        "exact_change": "Only nonzero feedback: phi(d)=5*d; d=w/(1+5*c). C complete r=0 branch, q/w/c ordering, four forward stages, histories, control law, excitation and gain unchanged.",
        "candidate_choice_count_in_this_task": 1,
        "source_files_sha256": source_hashes,
        "frozen_C_files_sha256": c_files,
        "frozen_C_declaration_sha256": sha(C / "DECLARATION.json"),
        "complete_r0_branch_source_byte_identical": True,
        "other_five_renderer_source_files_byte_identical": True,
        "ownership_test_source_byte_identical": True,
        "source_free_packet_sha256": packet_hashes,
        "gate": gate, "compiler": str(compiler), "compiler_sha256": sha(compiler),
        "build_flags": flags, "build_commands_serial": builds,
        "test_commands_serial": tests,
        "test_scope": "Unchanged host configuration/reset/gate invariants; unchanged focused ownership API test, 4096 frames independently plus 4096 interleaved per logical owner. Two owners, irregular 1/17/64 blocks, asymmetric off, live physical edits including depth1.2; in-memory samples only.",
        "reference_pcm_read_by_preparation": False,
        "audio_capture_or_reference_comparison_authorized": False,
        "original_source_boundary": "Frozen independent C, approved DnBOX vendors, independent numeric diagnosis and source-free observations only; no original engine source, header, object, assets or historical source-aware code.",
        "pending_action": "Stop after host/invariant checks; root review and registration before any new component or whole-voice audio comparison.",
        "production_activation": False}
    write_json(HERE / "BUILD-DECLARATION.json", declaration)
    run([str(compiler), "--version"], "compiler-version")
    for label, command in builds.items():
        run(command, label)
    nm = str(Path(shutil.which("nm")).resolve())
    run([nm, "-u", "build/s1_dry_voice.o"], "core-undefined-symbols")
    compiled = {name: sha(HERE / name) for name in
        ("build/s1_dry_voice.o", "build/s1_dry_host", "build/ownership_test")}
    pretest = {"schema": "Independent S1 Variant D before-tests freeze v1",
        "written_utc": now(), "status": "BUILT_NOT_EXECUTED",
        "build_declaration_sha256": sha(HERE / "BUILD-DECLARATION.json"),
        "source_files_sha256": source_hashes, "compiled_files_sha256": compiled,
        "tests": tests, "gate": gate,
        "audio_capture_or_reference_comparison_authorized": False}
    for name, digest in source_hashes.items():
        assert sha(HERE / name) == digest, "D input changed during build: " + name
    write_json(HERE / "PRETEST-DECLARATION.json", pretest)
    results = {}
    for label, command in tests.items():
        binary = Path(command[0])
        output = run(command, label, sha(binary))
        result = json.loads(output)
        write_json(HERE / "results" / (label + ".json"), result)
        results[label] = result
        if label == "describe":
            assert result["voice_bytes"] == 160 and result["prepared_bytes"] == 92
            assert result["coefficient_hex"] == ["3f7fce7c", "3f7ed775", "3f7fe249",
                "3f7fce7c", "3f7fe249", "3f7ffd07"]
        else:
            assert result["status"] == "PASS", "Test did not pass: " + label
    for name, digest in source_hashes.items():
        assert sha(HERE / name) == digest, "D input changed during tests: " + name
    for name, digest in compiled.items():
        assert sha(HERE / name) == digest, "D binary changed during tests: " + name
    for name, digest in c_files.items():
        assert sha(C / name) == digest, "C changed during preparation: " + name
    assert not list(HERE.rglob("*.f32le")), "Unexpected PCM file"
    result = {"status": "PASS_PREPARATION_ONLY", "finished_utc": now(),
        "source_files_sha256": source_hashes, "compiled_files_sha256": compiled,
        "build_declaration_sha256": sha(HERE / "BUILD-DECLARATION.json"),
        "pretest_declaration_sha256": sha(HERE / "PRETEST-DECLARATION.json"),
        "results": results, "failed_attempts": 0, "C_sources_unchanged": True,
        "in_memory_ownership_samples_evaluated": True,
        "reference_pcm_read": False, "pcm_files_written": False,
        "audio_acceptance_test_executed": False,
        "core_undefined_symbols_log": "logs/core-undefined-symbols-attempt01.json",
        "pending_action": declaration["pending_action"]}
    write_json(HERE / "PREPARATION-RESULT.json", result)
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
