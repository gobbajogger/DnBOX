#!/usr/bin/env python3
"""Check every post-apply file against the verified base-to-candidate Git blob map."""
import argparse
import json
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("checkout", type=Path)
parser.add_argument("roundtrip", type=Path)
args = parser.parse_args()
report = json.loads(args.roundtrip.read_text())
base = report["base"]
for row in report["results"]:
    name = row["path"]
    if row["base"] is not None:
        old = subprocess.check_output(["git", "-C", args.checkout, "rev-parse", f"{base}:{name}"], text=True).strip()
        if old != row["base"]:
            raise SystemExit(f"base blob mismatch: {name}")
    else:
        existed = subprocess.run(["git", "-C", args.checkout, "cat-file", "-e", f"{base}:{name}"],
                                 stderr=subprocess.DEVNULL).returncode == 0
        if existed:
            raise SystemExit(f"expected new path: {name}")
    actual = subprocess.check_output(["git", "-C", args.checkout, "hash-object", "--no-filters", "--", name], text=True).strip()
    if actual != row["actual_blob"]:
        raise SystemExit(f"candidate blob mismatch: {name}")
print(f"Verified all {len(report['results'])} paths against {base}.")
