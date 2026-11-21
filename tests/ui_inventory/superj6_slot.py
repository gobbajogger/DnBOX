#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Package the 26 SMK host frames reached by juno_slot_integration_test.c."""
import argparse
import gzip
import hashlib
import io
import json
import tarfile
from pathlib import Path

from PIL import Image


def digest(data):
    return hashlib.sha256(data).hexdigest()


def package(source, output):
    groups = ["oscillator", "filter", "envelope", "lfo-chorus"]
    routes = {
        "matrix": groups,
        "list": groups + ["sound"],
        "sound": ["poly", "pad", "pluck", "brass"],
    }
    frames = []
    archive = io.BytesIO()
    with tarfile.open(fileobj=archive, mode="w") as tar:
        for owner in ("A", "B"):
            for route, states in routes.items():
                for state in states:
                    name = f"slot-{owner.lower()}-{route}-{state}.png"
                    data = (source / name).read_bytes()
                    with Image.open(io.BytesIO(data)) as image:
                        if image.format != "PNG" or image.size != (240, 240) or image.mode != "RGB":
                            raise ValueError(f"Wrong capture format in {name}")
                        pixels = image.tobytes()
                    item = tarfile.TarInfo(name)
                    item.size = len(data)
                    tar.addfile(item, io.BytesIO(data))
                    frames.append(dict(file=name, owner=owner, route=route, state=state,
                                       width=240, height=240, sha256=digest(data),
                                       rgb_sha256=digest(pixels)))
    expected = {row["file"] for row in frames}
    if {p.name for p in source.glob("slot-*.png")} != expected:
        raise ValueError("Unexpected compact slot capture set")
    if len({row["rgb_sha256"] for row in frames}) != len(expected):
        raise ValueError("Different compact slot routes produced identical frames")
    output.mkdir(parents=True, exist_ok=True)
    packed = output / "frames.tar.gz"
    with packed.open("wb") as raw, gzip.GzipFile(filename="", fileobj=raw, mode="wb", mtime=0) as gz:
        gz.write(archive.getvalue())
    manifest = dict(provider="Stock compact juno6 slot", platform="SMK-37 host",
                    archive=packed.name, archive_sha256=digest(packed.read_bytes()), frames=frames)
    (output / "frames.json").write_text(json.dumps(manifest, indent=2) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture_dir", type=Path)
    parser.add_argument("output_dir", type=Path)
    args = parser.parse_args()
    package(args.capture_dir, args.output_dir)
