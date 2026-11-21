#!/usr/bin/env python3
"""Default read-only input verification; guarded execution only with --execute.

--execute is reserved for an explicitly allocated future native job. It verifies
inputs and enters the unchanged historical executor in this same, unoptimized
Python process. It does not request or imply an allocation.
"""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import os
import sys
from types import SimpleNamespace

HERE = Path(__file__).resolve().parent
def require(condition, message):
    if not condition:
        raise RuntimeError(message)

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def manifest(root):
    return {str(p.relative_to(root)): sha(p) for p in sorted(root.rglob("*"))
            if p.is_file() and "__pycache__" not in p.parts}

def main():
    require(sys.flags.optimize == 0, "Optimized Python is forbidden for verification and native execution; use normal Python without -O/-OO or PYTHONOPTIMIZE")
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--plan", type=Path, required=True)
    ap.add_argument("--execute", action="store_true", help="enter the historical executor only for an already allocated native job")
    a = ap.parse_args()
    forbidden_env = ("CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH", "OBJC_INCLUDE_PATH", "LIBRARY_PATH", "COMPILER_PATH", "GCC_EXEC_PREFIX", "LD_PRELOAD", "LD_LIBRARY_PATH")
    found = [name for name in forbidden_env if os.environ.get(name)]
    require(not found, "Unpinned native include/library environment: " + ", ".join(found))
    path = a.plan.resolve()
    require(path.is_relative_to(HERE / "native-plans"), "Unexpected output root")
    summary = json.loads((HERE / "PRODUCTION-PREPARATION.json").read_text())
    pins = [p for p in summary["plans"] if p["path"] == str(path)]
    require(len(pins) == 1, "Plan must have exactly one preparation receipt")
    pin = pins[0]
    require(sha(path) == pin["sha256"], "Plan changed")
    plan = json.loads(path.read_text())
    root = Path(plan["tree"]).resolve()
    require(root.is_relative_to(HERE), "Input tree outside prepared root")
    require(manifest(root) == plan["all_input_manifest"], "Prepared inputs changed or gained files")
    require(not (path.parent / "RESULT.json").exists() and not (path.parent / "COMMANDS.json").exists(), "Plan already attempted")
    tc = json.loads((HERE / "TOOLCHAIN-INPUTS.json").read_text())
    for name, digest in tc["binaries"].items():
        require(sha(Path(tc["root"]) / name) == digest, "Native tool changed: " + name)
    require(manifest(Path(tc["root"]) / "pi32v2/include") == tc["include_headers"], "Compiler headers changed")
    require(manifest(Path(tc["root"]) / "common/lib/clang") == tc["clang_resource_files"], "Compiler builtin resources changed")
    executor = HERE / "native_fit_replay.py"
    require(sha(executor) == "f896e0fd8ce2719876271476db261002d0909392e8059158427d80118a3b396e", "Executor adapter changed")
    if a.execute:
        # The historical executor contains assertions. Keeping this handoff in
        # the already checked interpreter prevents -O/PYTHONOPTIMIZE from
        # silently disabling them in a separately launched Python process.
        sys.dont_write_bytecode = True
        spec = importlib.util.spec_from_file_location("guarded_native_fit_replay", executor)
        require(spec is not None and spec.loader is not None, "Cannot load historical executor")
        replay = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(replay)
        return replay.execute(SimpleNamespace(plan=path))
    print(json.dumps({"status": "INPUTS_VERIFIED_NOT_RUN", "plan": str(path), "sha256": pin["sha256"], "compile_units": len(plan["compile_commands"]), "native_execution": "NOT_RUN", "allocation": "REQUIRED", "python_optimization": sys.flags.optimize}, indent=2))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
