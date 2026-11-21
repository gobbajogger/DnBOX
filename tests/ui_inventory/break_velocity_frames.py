#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Run the BREAK velocity subset of the existing geometry and physical input fixtures."""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]


def render(output):
    output = output.resolve()
    if output.is_relative_to(ROOT):
        raise ValueError("Supply an output directory outside the repository")
    output.mkdir(parents=True, exist_ok=False)
    work = output / "source"
    for name in ("assets", "experiments", "firmware", "host", "tests", "tools"):
        shutil.copytree(ROOT / name, work / name, ignore=shutil.ignore_patterns("__pycache__"))
    env = {k: v for k, v in os.environ.items() if "MUTATE" not in k}
    for family, script in (("live", "ui_step_lane_test.sh"), ("edited", "ui_step_lane_edit_test.sh")):
        with (output / (family + ".log")).open("w") as log:
            subprocess.run(["sh", "tests/host/" + script], cwd=work,
                           env=dict(env, SMC_UI_OUT=str(output / family)),
                           stdout=log, stderr=subprocess.STDOUT, check=True)
    sys.path.insert(0, str(work / "tests/ui_geometry"))
    from instrument import instrument
    instrument(work)
    src = work / "tests/ui_geometry/capture.c"
    code = src.read_text()
    loop = "        if(view==V_GLO) continue;"
    end = "    /* A bare V_GLO"
    if code.count(loop) != 1 or code.count(end) != 1:
        raise ValueError("The selector no longer matches the ordinary geometry walker")
    code = code.replace(loop, "        if(view!=V_SEQ || part!=PART_BRK) continue;\n" + loop)
    src.write_text(code[:code.index(end)] +
                   "    CHECK(total==9 && audit_frames==9 && !geo_findings && "
                   "!presence_missing && !pixel_damaged && !pixel_missing);\n    return 0;\n}\n")
    with (output / "ordinary.log").open("w") as log:
        subprocess.run(["sh", "tests/host/smc_ui_test.sh"], cwd=work,
                       env=dict(env, SMC_UI_OUT=str(output / "ordinary"),
                                SMC_UI_TEST_FILE="tests/ui_geometry/capture.c"),
                       stdout=log, stderr=subprocess.STDOUT, check=True)
    audit = [json.loads(line[6:]) for line in (output / "ordinary.log").read_text().splitlines()
             if line.startswith("AUDIT ")]
    if len({row["id"] for row in audit}) != 9 or any(row["findings"] or not row["texts"] for row in audit):
        raise ValueError("Invalid BREAK geometry outcomes")
    captures = {row["id"]: output / "ordinary" / (row["id"] + ".png") for row in audit}
    for name, family in (("edited-part4-kind0", "edited"), ("live-part4-lane0", "live"),
                         ("preview-part4-lane0", "live")):
        captures[name] = output / family / (name + ".png")
    with (ROOT / "docs/lanes/fwui/screens/screen-index.tsv").open() as stream:
        indexed = {row["state_id"]: row["rgb_sha256"] for row in csv.DictReader(stream, delimiter="\t")}
    for state_id, path in captures.items():
        with Image.open(path) as image:
            if image.size != (240, 240) or hashlib.sha256(image.convert("RGB").tobytes()).hexdigest() != indexed[state_id]:
                raise ValueError(f"Unexpected renderer result for {state_id}")
    print(f"{len(captures)} BREAK velocity screenshots matched, {sum(r['texts'] for r in audit)} verified text masks")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    render(parser.parse_args().output)
