#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Re-link an existing pair of stock builds using the recorded object order.

Run from any directory after building both models on the measured revision:
  python3 reproduce-link-maps.py --source CHECKOUT --toolchain JIELI_ROOT --output MAP_DIR
"""
import argparse
import gzip
import hashlib
import json
from pathlib import Path
import subprocess


def reproduce(row, source, toolchain, output):
    model = row["model"]
    command = list(row["command"])
    command[0] = str(toolchain / "pi32v2/bin/ld")
    command[command.index("-plugin") + 1] = str(toolchain / "pi32v2/bin/LLVMgold.so")
    map_file = output / f"{model}.map"
    linked = output / f"{model}-mapped.elf"
    command[command.index("-Map") + 1] = str(map_file)
    command[command.index("-o") + 1] = str(linked)
    subprocess.run(command, cwd=source, check=True)
    original = source / "build" / ("smk37-elite" if model == "elite" else "smk37") / "x0x.elf"
    data = linked.read_bytes()
    if data != original.read_bytes():
        raise ValueError(f"{model}: adding -Map changed the ELF")
    if hashlib.sha256(data).hexdigest() != row["original_elf_sha256"]:
        raise ValueError(f"{model}: built ELF differs from recorded revision")
    map_file.with_suffix(".map.gz").write_bytes(gzip.compress(map_file.read_bytes(), mtime=0))
    print(f"{model}: stock ELF matches, map at {map_file}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("source", "toolchain", "output"):
        parser.add_argument("--" + name, required=True, type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    receipt = json.loads(Path(__file__).with_name("stock-fit.json").read_text())
    for build in receipt["stock_builds"]:
        reproduce(build, args.source.resolve(), args.toolchain.resolve(), args.output.resolve())
