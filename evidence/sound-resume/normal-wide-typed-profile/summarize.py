#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Attribute official fm1-live time-sample counts to exact ELF symbols."""
from argparse import ArgumentParser
from bisect import bisect_right
from collections import Counter
from hashlib import sha256
from pathlib import Path
import json
import subprocess


def digest(path):
    return sha256(path.read_bytes()).hexdigest()


def symbols(elf):
    entries = []
    for line in subprocess.check_output(["nm", "-n", "-S", str(elf)], text=True).splitlines():
        fields = line.split()
        if len(fields) == 4 and fields[2] in ("T", "t", "W", "w"):
            address, size, _, name = fields
            entries.append((int(address, 16), int(size, 16), name))
    return entries


def attribute(entries, profile):
    starts = [address for address, _, _ in entries]
    counts = {core: Counter() for core in ("0", "1", "m0", "m1")}
    for line in profile.read_text().splitlines():
        core, address, weight = line.split()
        address = int(address, 16)
        index = bisect_right(starts, address) - 1
        name = f"{address:08x}"
        if index >= 0:
            start, size, symbol = entries[index]
            if address - start < size:
                name = symbol
        counts[core][name] += int(weight)
    return {
        core: {
            "total": histogram.total(),
            "top": histogram.most_common(18),
            "typed_hot_path": {
                name: histogram[name] for name in ("juno6_render_stereo", "factory_shape",
                    "shape.1092", "blep", "phase_section", "envelope_sample")
            },
        }
        for core, histogram in counts.items()
    }


def main():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path)
    parser.add_argument("baseline", type=Path)
    parser.add_argument("without_wave", type=Path)
    args = parser.parse_args()
    entries = symbols(args.elf)
    output = {
        "elf_sha256": digest(args.elf),
        "window_ms": [5620, 5670],
        "samples": {},
    }
    for label in ("baseline", "without_wave"):
        profile = getattr(args, label)
        output["samples"][label] = {"input_sha256": digest(profile), "cores": attribute(entries, profile)}
    print(json.dumps(output, indent=2))


if __name__ == "__main__":
    main()
