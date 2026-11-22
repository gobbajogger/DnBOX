#!/usr/bin/env python3
"""Build/run the one bounded ownership API test against unchanged frozen C."""
from pathlib import Path
import datetime
import hashlib
import json
import os
import shutil
import subprocess

HERE = Path(__file__).resolve().parent
CANDIDATE = HERE.parent / "s1_dry_voice_c"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, value):
    with path.open("x") as stream:
        json.dump(value, stream, indent=2, allow_nan=False)
        stream.write("\n")


def main():
    frozen = json.loads((CANDIDATE / "DECLARATION.json").read_text())
    for name, expected in frozen["source_files_sha256"].items():
        if sha(CANDIDATE / name) != expected:
            raise RuntimeError("Frozen C source changed: " + name)
    blocked = [key for key in ("CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH",
        "LIBRARY_PATH", "COMPILER_PATH", "GCC_EXEC_PREFIX", "LD_PRELOAD", "LD_LIBRARY_PATH")
        if os.environ.get(key)]
    if blocked:
        raise RuntimeError("Unpinned compiler environment: " + ", ".join(blocked))
    compiler = Path(shutil.which("cc")).resolve()
    executable = HERE / "build" / "ownership_test"
    flags = ["-std=c11", "-O2", "-ffp-contract=off", "-fno-fast-math",
        "-Wall", "-Wextra", "-Werror", "-Wdouble-promotion", "-DX0X_HOST=1",
        "-I" + str(CANDIDATE / "vendor"), "-I" + str(CANDIDATE / "src")]
    command = [str(compiler), *flags, str(HERE / "tests" / "ownership_test.c"),
        str(CANDIDATE / "src" / "s1_dry_voice.c"),
        str(CANDIDATE / "vendor" / "dnbx_s1_envelope.c"), "-o", str(executable)]
    candidate_files = ["src/s1_dry_voice.c", "src/s1_dry_voice.h", "vendor/fastmath.h",
                       "vendor/dnbx_s1_envelope.c", "vendor/dnbx_s1_envelope.h"]
    declaration = {"status": "BEFORE_OWNERSHIP_TEST_BUILD", "written_utc":
        datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "test_source_sha256": sha(HERE / "tests" / "ownership_test.c"),
        "driver_sha256": sha(Path(__file__)),
        "candidate_declaration_sha256": sha(CANDIDATE / "DECLARATION.json"),
        "candidate_files_sha256": {name: sha(CANDIDATE / name) for name in candidate_files},
        "compiler": str(compiler), "compiler_sha256": sha(compiler),
        "build_command": command,
        "scope": "Two owners;4096 frames independently and interleaved per owner;8192 evaluated frames per logical owner; irregular blocks1/17/64; live controls/depth1.2; asymmetric note-off; no reference PCM or saved audio; no fidelity test",
        "algorithm_changes": "NONE; link frozen C source unchanged"}
    write_json(HERE / "TEST-DECLARATION.json", declaration)
    build = subprocess.run(command, cwd=HERE, capture_output=True, text=True, timeout=30)
    write_json(HERE / "logs" / "build-attempt01.json", {"command": command,
        "exit_status": build.returncode, "stdout": build.stdout, "stderr": build.stderr})
    if build.returncode:
        raise RuntimeError("Build failed; exact output retained in logs/build-attempt01.json")
    binary_sha = sha(executable)
    test = subprocess.run([str(executable)], cwd=HERE, capture_output=True, text=True, timeout=30)
    write_json(HERE / "logs" / "test-attempt01.json", {"command": [str(executable)],
        "binary_sha256": binary_sha, "exit_status": test.returncode,
        "stdout": test.stdout, "stderr": test.stderr})
    if test.returncode:
        raise RuntimeError("Ownership test failed; exact output retained in logs/test-attempt01.json")
    result = json.loads(test.stdout)
    if result.get("status") != "PASS":
        raise RuntimeError("Test did not report PASS")
    for name, expected in declaration["candidate_files_sha256"].items():
        if sha(CANDIDATE / name) != expected:
            raise RuntimeError("Candidate source changed during test: " + name)
    if sha(executable) != binary_sha:
        raise RuntimeError("Test binary changed during execution")
    result["evidence"] = {"test_declaration_sha256": sha(HERE / "TEST-DECLARATION.json"),
        "source_sha256": declaration["test_source_sha256"], "binary_sha256": binary_sha,
        "build_log_sha256": sha(HERE / "logs" / "build-attempt01.json"),
        "test_log_sha256": sha(HERE / "logs" / "test-attempt01.json"),
        "candidate_files_still_unchanged": True}
    write_json(HERE / "RESULT.json", result)
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
