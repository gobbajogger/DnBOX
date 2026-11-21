#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Publish the stock selector and short-list frames from synthone_slot_test.c."""
import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image


def frame_names():
    states = ("ready", "updating", "pending", "long-name", "list-engine", "list-preset", "list-status")
    return {f"stock-init-{state}-{owner}" for owner in "ab" for state in states}


def publish(source, output):
    names = frame_names()
    if {p.stem for p in source.glob("stock-init-*.png")} != names:
        raise ValueError("Expected the stock selector and its three list selections on both bass parts")
    output.mkdir(parents=True, exist_ok=True)
    hashes = {}
    for name in sorted(names):
        with Image.open(source / (name + ".png")) as original:
            if original.format != "PNG" or original.size != (240, 240):
                raise ValueError(f"Invalid native capture: {name}")
            rgb = original.convert("RGB")
            rgb.save(output / (name + ".png"), optimize=True)
            hashes[name] = hashlib.sha256(rgb.tobytes()).hexdigest()
    (output / "frames.json").write_text(json.dumps(hashes, indent=2) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    publish(args.source, args.output)
