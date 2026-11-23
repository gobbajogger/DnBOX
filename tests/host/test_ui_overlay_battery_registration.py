#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Run the real selector/scheduler with cheap bodies, not firmware qualification.

Catches a missing/duplicate registration, wrong command, lost reader isolation,
selection leakage, or swallowed test failure. Only expensive compile bodies are
replaced; the runner's admission, scheduling and status propagation are real.
"""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
WORKER = r'''import os, pathlib, sys
name = sys.argv[1]
if os.environ["X0X_TEST_JOBS"] != "1":
    iso = pathlib.Path.cwd() / "build/host/iso" / name
    for variable, subdir in (("SMC_UI_OUT", "smc-ui"), ("HOME", "home"), ("TMPDIR", "tmp")):
        path = pathlib.Path(os.environ[variable])
        assert path == iso / subdir and path.is_dir(), (name, variable, path)
    assert not pathlib.Path("build/host/iso/.lock-gen").is_dir(), "UI reader holds scheduler writer lock"
    (iso / "smc-ui" / "reader").write_text(name)
with open("events", "a") as f: f.write(name + "\n")
sys.exit(7 if os.environ.get("FAIL_TEST") == name else 0)
'''


def check(source, jobs, selected, fail=""):
    with tempfile.TemporaryDirectory(prefix="overlay-selector-") as tmp:
        root = Path(tmp)
        (root / "tests/host").mkdir(parents=True)
        (root / "tests/run_tests.sh").write_text(source)
        (root / "worker.py").write_text(WORKER)
        for name, file in (("smc-ui", "smc_ui_test.sh"),
                           ("ui-overlay-battery", "ui_overlay_battery_test.sh"),
                           ("ui-toast-battery", "ui_toast_battery_test.sh")):
            (root / "tests/host" / file).write_text(
                f'#!/bin/sh\nexec python3 worker.py {name}\n')
        env = dict(os.environ, X0X_TEST_JOBS=str(jobs), X0X_TESTS=selected,
                   X0X_STRICT_PREFLIGHT="0", FAIL_TEST=fail)
        result = subprocess.run(["sh", "tests/run_tests.sh"], cwd=root, env=env,
                                capture_output=True, text=True, timeout=30)
        events = (root / "events").read_text().splitlines() if (root / "events").exists() else []
        expected = selected.split()
        assert sorted(events) == sorted(expected), (events, expected, result.stdout)
        assert result.returncode == bool(fail), (result.returncode, result.stdout, result.stderr)
        for name in expected:
            line = ("  FAIL " if name == fail else "  ok   ") + name
            assert sum(row.startswith(line) for row in result.stdout.splitlines()) == 1, result.stdout


def main():
    source = (ROOT / "tests/run_tests.sh").read_text()
    start = source.index("\ntest_class() {") + 1
    end = source.index("\n}", start) + 2
    classes = subprocess.check_output(
        ["sh", "-c", source[start:end] + "\nfor name; do test_class \"$name\"; echo \"$TEST_CLASS\"; done",
         "_", "smc-ui", "ui-overlay-battery", "ui-toast-battery"], text=True)
    assert classes.splitlines() == ["G", "G", "G"], classes
    cases = 0
    for jobs in (1, 4):
        for selected, fail in (("ui-overlay-battery", ""),
                               ("smc-ui", ""),
                               ("smc-ui ui-overlay-battery", ""),
                               ("ui-overlay-battery", "ui-overlay-battery"),
                               ("ui-toast-battery", ""),
                               ("smc-ui ui-toast-battery", ""),
                               ("ui-toast-battery", "ui-toast-battery")):
            check(source, jobs, selected, fail)
            cases += 1
    print(f"overlay selector: {cases} cases passed (scheduler only)")


if __name__ == "__main__":
    main()
