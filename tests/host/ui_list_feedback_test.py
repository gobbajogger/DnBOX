#!/usr/bin/env python3
"""Real list framebuffer positives + compiled missing-cue/state-owner negatives.

Every run copies current source into fresh scratch, compiles each candidate and
requires a runtime summary. Compiler failures never count as negative successes.
OUT may be reused; fresh scratch prevents stale sources/includes/executables.
"""
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
OUT = Path(os.environ.get("LIST_FEEDBACK_OUT", ROOT / "build/host/ui-list-feedback-proof")).resolve()
OUT.mkdir(parents=True, exist_ok=True)
(OUT / "receipt.json").unlink(missing_ok=True)
records = []
with tempfile.TemporaryDirectory(prefix="list-feedback-", dir=OUT) as temporary:
    source = Path(temporary) / "source"
    shutil.copytree(ROOT, source, ignore=shutil.ignore_patterns(".git", "build", "reference", "evidence", "docs", "artifacts", "web", "cloud-handoff"))
    ui = source / "firmware/src/app/ui.c"
    original = ui.read_text()

    def run(name, about=False, negative=False):
        destination = OUT / name
        command = ["sh", "tests/host/night_ui_test.sh", str(destination)] if about else ["sh", "tests/host/ui_list_feedback_test.sh", str(destination)]
        env = os.environ.copy()
        if about:
            env["NIGHT_UI_TEST_FILE"] = "tests/host/ui_about_list_feedback_test.c"
        process = subprocess.Popen(command, cwd=source, env=env, text=True,
                                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                   start_new_session=True)
        try:
            output, _ = process.communicate(timeout=120)
        except subprocess.TimeoutExpired:
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            output, _ = process.communicate()
            (OUT / (name + ".log")).write_text(output)
            (OUT / (name + ".exit")).write_text(str(process.returncode) + "\n")
            raise SystemExit(f"{name}: timed out; owned process group {process.pid} killed and reaped")
        result = subprocess.CompletedProcess(command, process.returncode, output)
        (OUT / (name + ".log")).write_text(result.stdout)
        (OUT / (name + ".exit")).write_text(str(result.returncode) + "\n")
        summary = re.search(r"list feedback: (\d+) expectations, (\d+) missing; (\d+) preserved behavior checks", result.stdout)
        if negative:
            assert result.returncode == 1 and summary and int(summary[2]) > 0, f"{name}: expected compiled runtime rejection; see {OUT / (name + '.log')}"
            assert "FAIL " not in result.stdout, f"{name}: preservation/oracle failure is not a caption-negative success"
        else:
            assert result.returncode == 0, f"{name} failed; see {OUT / (name + '.log')}"
            if not about:
                assert summary and int(summary[2]) == 0
            else:
                assert "ABOUT release fit PASS" in result.stdout
                assert "physical PARTON input PASS" in result.stdout
        records.append({"name": name, "exit": result.returncode, "runtime_summary": summary.group(0) if summary else result.stdout.splitlines()[-1]})
        print(records[-1]["runtime_summary"], flush=True)

    run("positive")
    run("about", about=True)
    start = original.index("static const char *action_key_cue(void)")
    end = original.index("/* Open-beta", start)
    helper = original[start:end]
    assert helper.count('return "SCALE";') == 1
    mutations = {"missing-cue": original[:start] + helper.replace('return "SCALE";', 'return "";') + original[end:]}
    draw = original.index("static UI_DRAW void draw_list(int band)")
    point = original.index("        pref_value(r, num, unit);", draw) + len("        pref_value(r, num, unit);")
    mutations["wrong-owner"] = original[:point] + '\n        if(r.kind==R_PARTON)put_s(num,pref_get(PR(R_PARTON,(r.a+1)%NPARTS,0,0))?"ON":"OFF");' + original[point:]
    mutations["wrong-state"] = original[:point] + '\n        if(r.kind==R_PARTON)put_s(num,pref_get(r)?"OFF":"ON");' + original[point:]
    try:
        for name, text in mutations.items():
            ui.write_text(text)
            run(name, negative=True)
    finally:
        ui.write_text(original)
(OUT / "receipt.json").write_text(json.dumps(records, indent=2) + "\n")
print("PASS list feedback current-source positive, ABOUT fit, and 3 compiled runtime negatives")
