#!/usr/bin/env python3
"""Read-only hash/closure verification. Does not compile, link or run targets."""
from pathlib import Path
import argparse
import hashlib
import json
import os

HERE = Path(__file__).resolve().parent
def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def manifest(root):
    return {str(p.relative_to(root)): sha(p) for p in sorted(root.rglob("*"))
            if p.is_file() and "__pycache__" not in p.parts}

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--plan", type=Path, required=True)
    a = ap.parse_args()
    path = a.plan.resolve()
    assert path.is_relative_to(HERE / "native-plans"), "Unexpected output root"
    summary = json.loads((HERE / "PRODUCTION-PREPARATION.json").read_text())
    pin = next(p for p in summary["plans"] if p["path"] == str(path))
    assert sha(path) == pin["sha256"], "Plan changed"
    plan = json.loads(path.read_text())
    root = Path(plan["tree"])
    assert root.is_relative_to(HERE), "Input tree outside prepared root"
    assert manifest(root) == plan["all_input_manifest"], "Prepared inputs changed or gained files"
    assert not (path.parent / "RESULT.json").exists() and not (path.parent / "COMMANDS.json").exists(), "Plan already attempted"
    tc = json.loads((HERE / "TOOLCHAIN-INPUTS.json").read_text())
    for name, digest in tc["binaries"].items():
        assert sha(Path(tc["root"]) / name) == digest, name
    assert manifest(Path(tc["root"]) / "pi32v2/include") == tc["include_headers"], "Compiler headers changed"
    assert manifest(Path(tc["root"]) / "common/lib/clang") == tc["clang_resource_files"], "Compiler builtin resources changed"
    assert sha(HERE / "native_fit_replay.py") == "f896e0fd8ce2719876271476db261002d0909392e8059158427d80118a3b396e", "Executor adapter changed"
    forbidden_env = ("CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH", "OBJC_INCLUDE_PATH", "LIBRARY_PATH", "COMPILER_PATH", "GCC_EXEC_PREFIX", "LD_PRELOAD", "LD_LIBRARY_PATH")
    found = [name for name in forbidden_env if os.environ.get(name)]
    assert not found, "Unpinned native include/library environment: " + ", ".join(found)
    print(json.dumps({"status": "INPUTS_VERIFIED_NOT_RUN", "plan": str(path), "sha256": pin["sha256"], "compile_units": len(plan["compile_commands"]), "native_execution": "NOT_RUN", "allocation": "REQUIRED"}, indent=2))

if __name__ == "__main__":
    main()
