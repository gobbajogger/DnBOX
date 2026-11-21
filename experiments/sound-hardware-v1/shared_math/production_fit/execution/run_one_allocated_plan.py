#!/usr/bin/env python3
"""Execute one allocated stock plan and preserve outer resource evidence."""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import platform
import shutil
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent
PREP = HERE.parent / "fit_next_plan"
PLANS = [
    ("production-reference-elite-04", "54b75516b9904283b37ffd9de9a50738a5322de6bfce754ed06265e8a976f2b9"),
    ("production-shared-elite-05", "a8ba4504d514db2160af53fd76db0e2441d63420a92723104d64f70ef0e7c22d"),
    ("production-reference-pro-06", "a9a41f0c31110b5d8af7b17265df1cfed7b94621fc85c83d62d99ea060568650"),
    ("production-shared-pro-07", "253510c1009f19be438504be7e8d0a4554f006d321f87a40c3c03d888a4b83e4"),
]
PINS = {
    "verify_inputs.py": "38b9676de48bed752d94ffd412248b556f4672507df04d52b544d90422d30b09",
    "native_fit_replay.py": "f896e0fd8ce2719876271476db261002d0909392e8059158427d80118a3b396e",
    "PRODUCTION-PREPARATION.json": "417a64624fc0d46ecf98f5f9456d43a652c73023beac67440de5f53b93318e87",
    "TOOLCHAIN-INPUTS.json": "d1d7f3df116af504437ac8526a142e6e4830ae48039aaaf54ced25383527d40a",
}

def require(condition, message):
    if not condition:
        raise RuntimeError(message)

def utc():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def write(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")

def local_processes():
    result = []
    native_names = {"cc", "gcc", "g++", "cc1", "cc1plus", "clang", "clang-4.0", "ld", "ld.lld", "ld.gold", "lld", "make", "ninja", "cargo", "rustc", "fm1-live", "fm1-emu"}
    for proc in Path("/proc").iterdir():
        if not proc.name.isdigit():
            continue
        try:
            fields = (proc / "stat").read_text().rsplit(")", 1)[1].split()
            name = (proc / "comm").read_text().strip()
            argv = (proc / "cmdline").read_bytes().split(b"\0")
            script = Path(argv[1].decode(errors="replace")).name if len(argv) > 1 else ""
            is_job = name in native_names or (name.startswith("python") and script in {"verify_inputs.py", "native_fit_replay.py", "build.py", "audio_path_identity.py"})
            result.append({"pid": int(proc.name), "ppid": int(fields[1]), "pgid": int(fields[2]), "state": fields[0], "comm": name, "relevant_job": is_job})
        except (FileNotFoundError, PermissionError, ProcessLookupError):
            continue
    return result

def capacity():
    mem = {}
    for line in Path("/proc/meminfo").read_text().splitlines():
        name, value = line.split(":", 1)
        if name in ("MemTotal", "MemAvailable"):
            mem[name] = int(value.split()[0]) * 1024
    group = {}
    for name in ("memory.max", "memory.current", "memory/memory.limit_in_bytes", "memory/memory.usage_in_bytes"):
        path = Path("/sys/fs/cgroup") / name
        group[name] = path.read_text().strip() if path.is_file() else None
    procs = local_processes()
    relevant = [p for p in procs if p["relevant_job"] and p["state"] != "Z"]
    result = {"utc": utc(), "hostname": platform.node(), "pid_namespace": os.readlink("/proc/self/ns/pid"),
              "process_visibility": "Local PID namespace only; no global process absence inferred", "self_cgroup": Path("/proc/self/cgroup").read_text(),
              "memory": mem, "cgroup_memory_files": group, "disk_free_bytes": shutil.disk_usage(HERE).free,
              "minimum_mem_available_bytes": 3200000000, "minimum_disk_free_bytes": 1000000000,
              "observed_active_local_jobs": relevant}
    return result

def main():
    require(sys.flags.optimize == 0, "Outer evidence runner requires normal Python")
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--label", choices=[name for name, _ in PLANS], required=True)
    args = ap.parse_args()
    index = [name for name, _ in PLANS].index(args.label)
    name, expected = PLANS[index]
    plan_path = PREP / "native-plans" / name / "PLAN.json"
    terminal_path = HERE / (name + "-terminal.json")
    admission_path = HERE / (name + "-admission.json")
    require(not terminal_path.exists() and not admission_path.exists(), "Refuse any repeat of this allocated plan")
    require((HERE / "ROOT-RELEASE.json").is_file(), "Await root's first-job release")
    release = json.loads((HERE / "ROOT-RELEASE.json").read_text())
    require(release.get("released") is True, "Root has not released execution")
    previous = None
    if index:
        previous = json.loads((HERE / (PLANS[index - 1][0] + "-terminal.json")).read_text())
        require(previous.get("local_process_group_released") is True, "Previous process group was not released")
        require(previous.get("end_utc"), "Previous plan lacks terminal timestamp")
    require(sha(plan_path) == expected, "Allocated plan hash changed")
    for path, digest in PINS.items():
        require(sha(PREP / path) == digest, "Pinned guard or metadata changed: " + path)
    admission = capacity()
    admission.update(label=name, plan=str(plan_path), plan_sha256=expected, allocation_comment=6995594729,
                     allocation_receipt_sha256=sha(HERE / "ALLOCATION-6995594729.json"), root_release_sha256=sha(HERE / "ROOT-RELEASE.json"),
                     previous_terminal_sha256=sha(HERE / (PLANS[index - 1][0] + "-terminal.json")) if index else None,
                     guard_pins=PINS, status="PREFLIGHT_RECORDED")
    write(admission_path, admission)
    require(admission["memory"]["MemAvailable"] >= 3200000000, "Insufficient fresh local memory")
    require(admission["disk_free_bytes"] >= 1000000000, "Insufficient fresh local disk space")
    require(not admission["observed_active_local_jobs"], "A relevant local compiler/target job is still active")
    argv = ["python3", "verify_inputs.py", "--execute", "--plan", str(plan_path)]
    log_path = HERE / (name + "-outer.log")
    started, wall_start = utc(), time.monotonic()
    with log_path.open("x") as log:
        child = subprocess.Popen(argv, cwd=PREP, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
        pid, status, usage = os.wait4(child.pid, 0)
        code = os.waitstatus_to_exitcode(status)
        child.returncode = code
    ended, elapsed = utc(), time.monotonic() - wall_start
    after = local_processes()
    remaining = [p for p in after if p["pgid"] == child.pid]
    result_path = plan_path.parent / "RESULT.json"
    result = json.loads(result_path.read_text()) if result_path.exists() else None
    receipt = {"label": name, "plan_sha256": expected, "argv": argv, "cwd": str(PREP), "start_utc": started, "end_utc": ended,
               "elapsed_seconds": elapsed, "returncode": code, "waited_pid": pid, "process_group": child.pid,
               "maxrss_kib": usage.ru_maxrss, "rss_scope": "wait4 maximum resident set reported for the verifier process and its waited descendants; not a sum of concurrent RSS",
               "user_seconds": usage.ru_utime, "system_seconds": usage.ru_stime,
               "minor_faults": usage.ru_minflt, "major_faults": usage.ru_majflt, "block_input": usage.ru_inblock, "block_output": usage.ru_oublock,
               "outer_log": str(log_path), "outer_log_sha256": sha(log_path), "admission_sha256": sha(admission_path),
               "remaining_local_process_group_members": remaining, "local_process_group_released": not remaining,
               "process_visibility": "Local PID namespace only; no global process absence inferred",
               "other_observed_active_local_jobs_after": [p for p in after if p["relevant_job"] and p["state"] != "Z"],
               "result": result, "result_sha256": sha(result_path) if result_path.exists() else None,
               "emulator_execution": "NOT_RUN", "status": "TERMINAL" if result else "GUARD_OR_LAUNCH_FAILED"}
    write(terminal_path, receipt)
    print(json.dumps({"label": name, "status": receipt["status"], "result_status": result.get("status") if result else None,
                      "returncode": code, "link_returncode": result.get("physical_link_returncode") if result else None,
                      "load_image_bytes": result.get("load_image_bytes") if result else None,
                      "physical_free_bytes": result.get("physical_free_bytes") if result else None,
                      "reserve_margin_bytes": result.get("reserve_margin_bytes") if result else None,
                      "maxrss_kib": usage.ru_maxrss, "elapsed_seconds": elapsed, "local_process_group_released": not remaining,
                      "receipt": str(terminal_path)}, indent=2))
    require(not remaining, "Plan is terminal but local process group remains; do not start the next plan")
    return code

if __name__ == "__main__":
    raise SystemExit(main())
