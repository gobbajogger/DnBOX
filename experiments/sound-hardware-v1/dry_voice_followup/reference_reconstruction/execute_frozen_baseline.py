"""Execute only the root-authorized frozen original-reference baseline attempt.

Build/orchestration and explicit guards only. This file contains no DSP code.
"""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parent
EXPECTED_PROPOSAL = "36d914226f921180f3c7d79643105e900650f65d9d3a1c5016e496891cb57aed"
EXPECTED_BASELINE = "b02703f0893f9e7676e4db03048f9fefe8575ae1b1a1f44da1167253309a6cd2"
INJECTION_KEYS = (
    "CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH", "OBJC_INCLUDE_PATH",
    "LIBRARY_PATH", "LD_LIBRARY_PATH", "LD_PRELOAD", "LD_AUDIT",
    "GCC_EXEC_PREFIX", "COMPILER_PATH", "GCC_SPECS", "CFLAGS", "CXXFLAGS",
    "CPPFLAGS", "LDFLAGS", "DEPENDENCIES_OUTPUT", "SUNPRO_DEPENDENCIES",
)
COMMAND_RECEIPTS = []


class StopAttempt(RuntimeError):
    pass


def now():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write_new(path, value):
    with Path(path).open("x") as stream:
        json.dump(value, stream, indent=2, allow_nan=False)
        stream.write("\n")


def require(condition, reason):
    if not condition:
        raise StopAttempt(reason)


def verify(record, resolved=False):
    path = Path(record["resolved_path"] if resolved else record["path"])
    require(path.is_file(), f"Pinned input is absent: {path}")
    require(path.stat().st_size == record["bytes"], f"Pinned input size differs: {path}")
    require(digest(path) == record["sha256"], f"Pinned input SHA-256 differs: {path}")


def processes():
    found = []
    for entry in Path("/proc").iterdir():
        if not entry.name.isdigit():
            continue
        try:
            name = (entry / "comm").read_text().strip()
        except OSError:
            continue
        if any(term in name.lower() for term in (
            "cc1", "gcc", "g++", "clang", "ld.bfd", "ld.lld", "render",
            "capture_filter", "capture_orig", "qemu",
        )):
            found.append({"pid": int(entry.name), "comm": name})
    return found


def wait_for_capacity(label, deadline=None):
    end = min(time.monotonic() + 30, deadline) if deadline else time.monotonic() + 30
    observations = []
    while True:
        active = processes()
        observations.append({"observed_utc": now(), "active_processes": active})
        if not active:
            break
        if time.monotonic() >= end:
            write_new(ROOT / "metadata" / (label + ".capacity.json"), observations)
            raise StopAttempt("Observed compiler/renderer did not clear inside the capacity bound")
        print(json.dumps({"capacity_wait": label, "active_processes": active}), flush=True)
        time.sleep(min(1, max(0.01, end - time.monotonic())))
    write_new(ROOT / "metadata" / (label + ".capacity.json"), observations)


def run_command(label, argv, timeout_seconds, env, classification):
    require(timeout_seconds > 0, "Global build deadline was exhausted before the next command")
    receipt = {
        "label": label, "argv": argv, "timeout_seconds": timeout_seconds,
        "start_utc": now(), "classification": classification, "attempt": 1,
        "native_target": False,
    }
    stdout = ROOT / "logs" / (label + ".stdout.log")
    stderr = ROOT / "logs" / (label + ".stderr.log")
    with stdout.open("xb") as out, stderr.open("xb") as err:
        child = subprocess.Popen(argv, cwd=ROOT, env=env, stdout=out, stderr=err,
                                 start_new_session=True)
        try:
            receipt["exit_code"] = child.wait(timeout=timeout_seconds)
            receipt["timed_out"] = False
        except subprocess.TimeoutExpired:
            os.killpg(child.pid, signal.SIGKILL)
            child.wait(timeout=5)
            receipt["exit_code"] = child.returncode
            receipt["timed_out"] = True
    receipt["finish_utc"] = now()
    receipt["stdout"] = {"path": str(stdout), "bytes": stdout.stat().st_size,
                         "sha256": digest(stdout)}
    receipt["stderr"] = {"path": str(stderr), "bytes": stderr.stat().st_size,
                         "sha256": digest(stderr)}
    receipt["raw_original_source_diagnostics_remain_reference_owner_only"] = classification == "compile_reference_source"
    write_new(ROOT / "metadata" / (label + ".receipt.json"), receipt)
    COMMAND_RECEIPTS.append(receipt)
    print(json.dumps({"completed": label, "exit_code": receipt["exit_code"],
                      "timed_out": receipt["timed_out"]}), flush=True)
    require(not receipt["timed_out"] and receipt["exit_code"] == 0,
            f"Command failed or timed out: {label}; raw logs preserved privately")
    return receipt


def main():
    proposal_file = ROOT / "PROPOSAL.json"
    require(digest(proposal_file) == EXPECTED_PROPOSAL, "Root-authorized proposal hash differs")
    require((ROOT / "PROPOSAL.sha256").read_text().split()[0] == EXPECTED_PROPOSAL,
            "Proposal sidecar hash differs")
    proposal = json.loads(proposal_file.read_text())
    for key in ("wrapper", "staged_exact_input_manifest", "numeric_fixture_manifest",
                "toolchain_identity", "map_exclusion_plan", "capacity_snapshot"):
        verify(proposal[key])
    staged = json.loads(Path(proposal["staged_exact_input_manifest"]["path"]).read_text())
    require(staged["count"] == 39 and len(staged["rows"]) == 39, "Expected exact 39-input closure")
    for row in staged["rows"]:
        verify(row["staged"])
        require(row["original"]["sha256"] == row["staged"]["sha256"],
                "Staging identity differs from its pinned original")
    expected_source = {Path(row["staged"]["path"]) for row in staged["rows"]
                       if Path(row["staged"]["path"]).is_relative_to(ROOT / "staged_reference")}
    actual_source = {p for p in (ROOT / "staged_reference").rglob("*") if p.is_file()}
    require(actual_source == expected_source, "Unpinned source/include file in the staged tree")
    numeric = json.loads(Path(proposal["numeric_fixture_manifest"]["path"]).read_text())
    derived = [row["input"] for row in numeric["controls"]] + [numeric["tuning"]["input"]]
    for record in derived:
        verify(record)
    expected_numeric = {Path(record["path"]) for record in derived}
    expected_numeric.update(Path(row["staged"]["path"]) for row in staged["rows"]
                            if Path(row["staged"]["path"]).is_relative_to(ROOT / "inputs"))
    require({p for p in (ROOT / "inputs").iterdir() if p.is_file()} == expected_numeric,
            "Unpinned numeric input file")
    toolchain = json.loads(Path(proposal["toolchain_identity"]["path"]).read_text())
    for record in toolchain["tools"].values():
        verify(record, resolved=True)
    for query in toolchain["metadata_queries"]:
        if "identity" in query:
            verify(query["identity"], resolved=True)
    for record in toolchain["runtime_files"]:
        verify(record, resolved=True)
    blocked_env = [key for key in INJECTION_KEYS if os.environ.get(key)]
    require(not blocked_env, "Unpinned include/library/tool environment: " + ", ".join(blocked_env))
    require(not any((ROOT / "build").iterdir()), "Build directory is not empty; no overwrite allowed")
    require(not any((ROOT / "outputs").iterdir()), "Output directory is not empty; no overwrite allowed")
    require(len(proposal["compile_commands"]) == 18, "Unexpected compiler command count")
    require(len(proposal["read_only_audit_commands"]) == 6, "Unexpected audit command count")
    require(proposal["limits"]["active_modulation_capture_commands"] == 0,
            "Active modulation is not authorized")
    require(proposal["baseline_command"]["required_original_mono_sha256"] == EXPECTED_BASELINE,
            "Baseline target identity differs")
    for item in json.loads((ROOT / "metadata/protected_before.json").read_text())["files"]:
        verify(item)
    assembler = Path("/usr/bin/as").resolve()
    require(assembler.is_file(), "System assembler not found at the controlled PATH")
    temporary = ROOT / "build" / "tmp"
    temporary.mkdir()
    env = {"PATH": "/usr/bin:/bin", "LC_ALL": "C", "LANG": "C", "TZ": "UTC",
           "TMPDIR": str(temporary)}
    write_new(ROOT / "metadata/explicit_execution_guards.json", {
        "root_authorized_proposal_sha256": EXPECTED_PROPOSAL,
        "wrapper_sha256": proposal["wrapper"]["sha256"],
        "original_staged_inputs_verified": 39, "derived_numeric_inputs_verified": 3,
        "unpinned_injection_environment": [], "child_environment": env,
        "system_assembler_identity": {"path": str(assembler), "bytes": assembler.stat().st_size,
                                      "sha256": digest(assembler)},
        "guard_implementation": "explicit exceptions and conditional checks; no asserts",
        "executor_sha256": digest(__file__), "native_target_plans_allocated": 0,
    })
    wait_for_capacity("authorized-build-start")
    deadline = time.monotonic() + 60
    for index, command in enumerate(proposal["compile_commands"], 1):
        wait_for_capacity(f"compile-{index:02d}", deadline)
        run_command(f"baseline-compile-{index:02d}", command["argv"],
                    min(command["timeout_seconds"], deadline - time.monotonic()),
                    env, command["stage"])
    wait_for_capacity("link", deadline)
    link = proposal["link_command"]
    run_command("baseline-host-link", link["argv"],
                min(link["timeout_seconds"], deadline - time.monotonic()), env, "host_link")
    require(time.monotonic() <= deadline, "Combined compile/link phase exceeded the 60-second bound")
    for command in proposal["read_only_audit_commands"]:
        run_command("baseline-audit-" + command["name"], command["argv"],
                    command["timeout_seconds"], env, "read_only_link_audit")
    checks_plan = json.loads(Path(proposal["map_exclusion_plan"]["path"]).read_text())
    def audit_text(name):
        return (ROOT / "logs" / ("baseline-audit-" + name + ".stdout.log")).read_text()
    defined = audit_text("all_defined_demangled_symbols")
    undefined = audit_text("undefined_demangled_symbols")
    retained_forbidden = [x for x in checks_plan["forbidden_retained_symbol_fragments"]
                          if x in defined or x in undefined]
    missing_required = [x for x in checks_plan["required_defined_demangled_fragments"] if x not in defined]
    map_file = ROOT / "build/capture_original_dry.map"
    loads = [line.split(maxsplit=1)[1] for line in map_file.read_text().splitlines()
             if line.startswith("LOAD ")]
    workspace_loads = [path for path in loads if path.startswith("/workspace/")]
    needed = re.findall(r"Shared library: \[(.*?)\]", audit_text("dynamic_dependencies"))
    pinned_runtime = {row["resolved_path"]: row["sha256"] for row in toolchain["runtime_files"]}
    resolved = []
    for line in audit_text("runtime_loader_resolution").splitlines():
        match = re.search(r"(?:=>\s*)?(/[^\s]+)", line)
        if match:
            file = Path(match.group(1)).resolve()
            resolved.append({"path": str(file), "sha256": digest(file),
                             "matches_pin": digest(file) == pinned_runtime.get(str(file))})
    audit = {
        "required_symbols_missing": missing_required,
        "forbidden_symbols_retained_or_unresolved": retained_forbidden,
        "workspace_LOAD_entries": workspace_loads,
        "dynamic_needed": needed, "resolved_runtime": resolved,
        "checks": {
            "required_original_symbols": not missing_required,
            "no_candidate_symbols": not retained_forbidden,
            "exact_workspace_objects": workspace_loads == checks_plan["expected_workspace_LOAD_entries"],
            "allowed_dynamic_dependencies": set(needed).issubset(checks_plan["dynamic_dependencies_allowed"]),
            "runtime_files_resolved": bool(resolved),
            "all_runtime_hashes_pinned": all(row["matches_pin"] for row in resolved),
            "host_x86_64": "Advanced Micro Devices X86-64" in audit_text("elf_header"),
        },
    }
    audit["passed"] = all(audit["checks"].values())
    write_new(ROOT / "metadata/authorized_link_audit.json", audit)
    require(audit["passed"], "Post-link audit failed; no baseline render authorized by this attempt")
    wait_for_capacity("baseline-capture")
    for row in staged["rows"]:
        verify(row["staged"])
    for record in derived:
        verify(record)
    verify(proposal["wrapper"])
    for record in toolchain["runtime_files"]:
        verify(record, resolved=True)
    executable = ROOT / "build/capture_original_dry"
    write_new(ROOT / "metadata/baseline_executable_identity.json", {
        "path": str(executable), "bytes": executable.stat().st_size, "sha256": digest(executable),
    })
    capture = proposal["baseline_command"]
    run_command("baseline-capture", capture["argv"], capture["timeout_seconds"], env,
                "single_original_dry_baseline_capture")
    wrapper_receipt = json.loads((ROOT / "logs/baseline-capture.stdout.log").read_text())
    output = Path(capture["argv"][4])
    actual_size = output.stat().st_size
    actual_hash = digest(output)
    result = {
        "classification": "NEW_ORIGINAL_RECONSTRUCTION_BASELINE_IDENTITY_CHECK",
        "case_id": capture["case_id"], "output_path": str(output),
        "expected_bytes": capture["required_original_mono_bytes"], "actual_bytes": actual_size,
        "expected_sha256": EXPECTED_BASELINE, "actual_sha256": actual_hash,
        "exact_identity": actual_size == capture["required_original_mono_bytes"] and actual_hash == EXPECTED_BASELINE,
        "wrapper_receipt": wrapper_receipt, "active_modulation_captures": 0,
        "native_target_links": 0,
    }
    write_new(ROOT / "metadata/baseline_identity_result.json", result)
    require(result["exact_identity"], "Original baseline byte identity differs; preserve PCM and stop")
    expected_counts = {"notes_completed": 3, "warmup_frames": 529344,
                       "retained_frames": 264576, "total_reference_frames": 793920,
                       "process_calls": 12405, "warmup_nonfinite_count": 0,
                       "output_nonfinite_count": 0, "status": 0}
    require(all(wrapper_receipt.get(k) == v for k, v in expected_counts.items()),
            "Baseline identity matched but declared counts/status differ")
    for item in json.loads((ROOT / "metadata/protected_before.json").read_text())["files"]:
        verify(item)
    write_new(ROOT / "metadata/authorized_attempt_summary.json", {
        "status": "EXACT_BASELINE_IDENTITY_CONFIRMED; ACTIVE_CAPTURE_NOT_RUN",
        "proposal_sha256": EXPECTED_PROPOSAL, "compiler_driver_commands": 18,
        "host_links": 1, "read_only_audits": 6, "baseline_captures": 1,
        "active_modulation_captures": 0, "native_target_links": 0, "retries": 0,
        "baseline_result": result, "protected_filter_and_alias_inputs_unchanged": True,
        "all_attempt_logs_preserved": True,
    })
    print(json.dumps({"status": "EXACT_BASELINE_IDENTITY_CONFIRMED",
                      "bytes": actual_size, "sha256": actual_hash,
                      "active_modulation_captures": 0}), flush=True)


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        blocker = {"status": "STOPPED_WITHOUT_RETRY", "reason": str(error),
                   "completed_command_receipts": len(COMMAND_RECEIPTS),
                   "active_modulation_captures": 0, "original_source_repaired": False,
                   "original_source_logs_remain_private": True, "observed_utc": now()}
        write_new(ROOT / "metadata/authorized_attempt_blocker.json", blocker)
        print(json.dumps(blocker), flush=True)
        sys.exit(1)
