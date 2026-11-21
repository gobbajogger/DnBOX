#!/usr/bin/env python3
"""Inventory the actual Synth One selector against the authenticated factory streams."""
import argparse
import csv
import json
import hashlib
import os
from pathlib import Path
import subprocess
import tempfile

here = Path(__file__).resolve().parent
repo = here.parents[2]
cli = argparse.ArgumentParser(description=__doc__)
cli.add_argument("output", type=Path)
args = cli.parse_args()
with tempfile.TemporaryDirectory() as tmp:
    work = Path(tmp)
    subprocess.run(["python3", repo / "tools/compile-synthone-factory.py", "--out", work], check=True)
    subprocess.run([os.environ.get("CC", "cc"), "-std=c11", "-O2", "-flto", "-ffunction-sections",
                    "-fdata-sections", "-Ifirmware/src", "-Ifirmware/src/app", "-Ifirmware/src/dsp",
                    here / "catalog.c", "firmware/src/app/original_sound.c", "-Wl,--gc-sections",
                    "-o", work / "catalog"], cwd=repo, check=True)
    entries = subprocess.check_output([work / "catalog"], text=True).splitlines()
    if entries != ["0\tINIT"]:
        raise ValueError("Audited selector changed; verify any new program-to-source bindings")
    manifest = work / "ORIGINAL-PRESET-COMMANDS.json"
    original = json.loads(manifest.read_text())["cases"]
    assert len(original) == 694 and len(set(row["bank"] for row in original)) == 12
    print("Command manifest SHA-256", hashlib.sha256(manifest.read_bytes()).hexdigest())
    with args.output.open("x", newline="") as out:
        w = csv.writer(out, lineterminator="\n")
        w.writerow(["bank", "bank_index", "name", "uid", "bank_sha256", "command_count",
                    "candidate_status", "candidate_rms_db", "candidate_spectral_db"])
        for row in original:
            w.writerow([row["bank"], row["preset_index"], row["name"], row["uid"],
                        row["source_sha256"], len(row["commands"]), "NO_FACTORY_BINDING", "", ""])
