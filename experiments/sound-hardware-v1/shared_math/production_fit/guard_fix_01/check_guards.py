#!/usr/bin/env python3
"""Bounded compiler-free guard regression and four read-only input rechecks."""
from pathlib import Path
import ast
import datetime
import hashlib
import json
import os
import shutil
import subprocess
import sys

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent

def require(condition, message):
    if not condition:
        raise RuntimeError(message)

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def write(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")

def check_preserved_plans(before):
    checked = []
    for item in before["plans"]:
        path, tree = Path(item["path"]), Path(item["tree"])
        require(sha(path) == item["sha256"], "Production PLAN bytes changed")
        actual = {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob("*"))
                  if p.is_file() and "__pycache__" not in p.parts}
        require(actual == item["all_input_manifest"], "Production source tree changed")
        require(not (path.parent / "RESULT.json").exists() and not (path.parent / "COMMANDS.json").exists(), "Unexpected native execution receipt")
        checked.append({"path": str(path), "sha256": item["sha256"], "source_files": len(actual), "tree_unchanged": True})
    return checked

def main():
    require(sys.flags.optimize == 0, "Run guard validation with normal Python only")
    require(not (HERE / "VALIDATION.json").exists(), "Refuse to overwrite guard validation")
    before = json.loads((HERE / "BEFORE.json").read_text())
    check_preserved_plans(before)
    sources = {}
    for name in ("prepare_production.py", "verify_inputs.py"):
        path = ROOT / name
        require(not any(isinstance(node, ast.Assert) for node in ast.walk(ast.parse(path.read_text()))), "Mandatory check still uses assert: " + name)
        sources[name] = sha(path)
    require(sha(ROOT / "native_fit_replay.py") == "f896e0fd8ce2719876271476db261002d0909392e8059158427d80118a3b396e", "Historical executor changed")
    fixture = HERE / "negative_modified_input"
    tree = fixture / "native-plans" / "fixture" / "tree"
    tree.mkdir(parents=True, exist_ok=False)
    shutil.copy2(ROOT / "verify_inputs.py", fixture / "verify_inputs.py")
    input_file = tree / "input.c"
    input_file.write_text("int guard_fixture = 1;\n")
    expected_input_sha = sha(input_file)
    plan_path = tree.parent / "PLAN.json"
    write(plan_path, {"tree": str(tree), "all_input_manifest": {"input.c": expected_input_sha}})
    write(fixture / "PRODUCTION-PREPARATION.json", {"plans": [{"path": str(plan_path), "sha256": sha(plan_path)}]})
    input_file.write_text("int guard_fixture = 2;\n")
    # This fixture has neither toolchain metadata nor an executor. Even an
    # unexpectedly missed negative guard cannot launch a compiler or linker.
    require(not (fixture / "TOOLCHAIN-INPUTS.json").exists() and not (fixture / "native_fit_replay.py").exists(), "Negative fixture must be incapable of native execution")
    python = sys.executable
    commands = []
    base_env = dict(os.environ)
    base_env.pop("PYTHONOPTIMIZE", None)
    base_env["PYTHONDONTWRITEBYTECODE"] = "1"

    def invoke(label, argv, expected_message=None, overrides=None):
        env = dict(base_env)
        env.update(overrides or {})
        result = subprocess.run(argv, text=True, capture_output=True, env=env, timeout=30)
        log = HERE / (label + ".log")
        log.write_text(result.stdout + result.stderr)
        receipt = {"label": label, "argv": argv, "environment_overrides": overrides or {},
                   "returncode": result.returncode, "log": str(log), "log_sha256": sha(log)}
        if expected_message is not None:
            require(result.returncode != 0 and expected_message in result.stderr,
                    label + " did not reject at the intended guard")
            receipt.update(status="REJECTED_AS_EXPECTED", expected_guard=expected_message)
        else:
            require(result.returncode == 0, label + " input verification failed")
            receipt.update(status="PASS", verification=json.loads(result.stdout))
        commands.append(receipt)
        write(HERE / "COMMANDS.json", commands)
        return receipt

    negatives = [
        invoke("negative-modified-input", [python, str(fixture / "verify_inputs.py"), "--plan", str(plan_path)],
               "Prepared inputs changed or gained files"),
        invoke("negative-forbidden-cpath", [python, str(ROOT / "verify_inputs.py"), "--plan", before["plans"][0]["path"]],
               "Unpinned native include/library environment: CPATH", {"CPATH": str(HERE / "forbidden-include-path")}),
        invoke("negative-optimized-execute", [python, "-O", str(fixture / "verify_inputs.py"), "--execute", "--plan", str(plan_path)],
               "Optimized Python is forbidden for verification and native execution"),
        invoke("negative-optimized-preparation", [python, "-O", str(ROOT / "prepare_production.py"), "--source", str(HERE / "nonexistent-canonical-source")],
               "Optimized Python is forbidden for preparation and native execution"),
        invoke("negative-env-optimized-execute", [python, str(fixture / "verify_inputs.py"), "--execute", "--plan", str(plan_path)],
               "Optimized Python is forbidden for verification and native execution", {"PYTHONOPTIMIZE": "1"}),
    ]
    verified = []
    for index, plan in enumerate(before["plans"]):
        verified.append(invoke("normal-input-verification-" + str(index + 1),
                               [python, str(ROOT / "verify_inputs.py"), "--plan", plan["path"]])["verification"])
    final = {"status": "PASS_COMPILER_FREE_GUARD_VALIDATION", "created_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
             "python_executable": python, "python_optimization": sys.flags.optimize, "script_sha256": sources,
             "mandatory_assert_nodes": 0, "negative_cases": negatives, "normal_input_verifications": verified,
             "modified_input_fixture": {"path": str(input_file), "expected_sha256": expected_input_sha, "actual_sha256": sha(input_file), "not_a_production_tree": True},
             "production_plans_after": check_preserved_plans(before), "native_execution": "NOT_RUN", "emulator_execution": "NOT_RUN",
             "historical_executor_sha256": sha(ROOT / "native_fit_replay.py")}
    write(HERE / "VALIDATION.json", final)
    print(json.dumps({"status": final["status"], "negative_cases": len(negatives), "normal_input_verifications": len(verified), "production_plans_and_trees_unchanged": True, "native_execution": "NOT_RUN", "receipt": str(HERE / "VALIDATION.json")}, indent=2))

if __name__ == "__main__":
    main()
