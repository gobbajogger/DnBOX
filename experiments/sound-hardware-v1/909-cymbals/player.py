#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Compare the historical and shipping 909 against unchanged upstream 9W9."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys

from measure import ROOT, digest
from score_rows import rows


def run_suite(exe, samples, log):
    with log.open("w") as stream:
        status = subprocess.run([exe, samples, log.with_suffix(".wav")],
                                stdout=stream, stderr=subprocess.STDOUT)
    report = log.read_text()
    verdict = re.search(r"^(PASS|FAIL): (\d+) failure\(s\)$", report, re.MULTILINE)
    if not verdict or status.returncode != int(verdict[1] == "FAIL"):
        raise RuntimeError(f"incomplete 9W9 suite, see {log}")
    demo = next(line for line in report.splitlines() if line.startswith("demo pattern "))
    return {"failures": int(verdict[2]), "demo": demo.strip()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original", type=Path, help="upstream schwung-9W9 checkout")
    parser.add_argument("output", type=Path, help="new directory for raw PCM and reports")
    args = parser.parse_args()
    upstream = args.original.resolve()
    original = upstream / "src"
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    tests = ROOT / "tests/host"
    src = ROOT / "firmware/src/dsp"
    gen = ROOT / "build/gen"
    obj = ROOT / "build/drum909_test"
    expected = json.loads((ROOT / "assets/909/encoded/manifest.json").read_text())
    for stem, hashes in expected.items():
        if digest((original / "samples" / (stem + ".wav")).read_bytes()) != hashes["wav_sha256"]:
            raise ValueError(f"upstream PCM differs for {stem}")

    # Compile upstream, the normal production header and firmware through the existing suite.
    with (out / "matched.log").open("w") as log:
        subprocess.run(["sh", tests/"run_drum909.sh"], cwd=ROOT, check=True,
                       env={**os.environ, "NINEW9_DIR": str(upstream)}, stdout=log, stderr=subprocess.STDOUT)
    history = out / "historical"
    subprocess.run([sys.executable, tests/"cymbal_ref.py", "--historical", history], check=True)
    cc = os.environ.get("CC", "cc")
    flags = [cc, "-O2", "-ffp-contract=off", "-std=c99", "-I", src, "-I", src.parent]
    subprocess.run([*flags, "-I", history, "-I", gen, "-c", src/"drum909.c",
                    "-o", history/"drum909.o"], check=True)
    subprocess.run([*flags, "-I", tests, "-I", gen, "-c", Path(__file__).with_name("capture.c"),
                    "-o", out/"capture.o"], check=True)
    common = [obj/(name + ".o") for name in ("fxbus", "x0x_param", "er99_engine", "drum909_ref", "drum909_refd")]
    arms = {}
    for arm, drum in (("historical", history/"drum909.o"), ("shipping", obj/"drum909.o")):
        directory = out / arm
        directory.mkdir(exist_ok=True)
        capture = directory / "capture"
        subprocess.run([cc, out/"capture.o", drum, *common, "-lm", "-o", capture], check=True)
        pcm = directory / "rows.f32"
        with (directory/"capture.log").open("w") as log:
            subprocess.run([capture, original, pcm], check=True, stdout=log)
        scored = list(rows(pcm))
        if len(scored) != 108 or len({row["case"] for row in scored}) != 108:
            raise ValueError(f"incomplete {arm} coverage")
        (directory/"rows.json").write_text(json.dumps(scored, indent=2, allow_nan=False)+"\n")
        exe = directory / "suite"
        subprocess.run([cc, obj/"drum909_test.o", drum, *common, "-lm", "-o", exe], check=True)
        summary = run_suite(exe, original, directory/"raw-suite.log")
        arms[arm] = {row["case"]:row for row in scored}
        print(arm, sum(row["within_limits"] for row in scored), "/ 108 within the lane limits", summary)
    for label, report in arms["historical"].items():
        if report["reference_sha256"] != arms["shipping"][label]["reference_sha256"]:
            raise ValueError(f"different original for {label}")
    if not all(row["within_limits"] for row in arms["shipping"].values()):
        raise SystemExit(1)


if __name__ == "__main__":
    main()
