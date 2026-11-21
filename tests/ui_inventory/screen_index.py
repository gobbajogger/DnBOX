#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Verify the packed screenshots and export a searchable case-to-archive index."""
import argparse
import csv
import hashlib
import io
import json
import tarfile
from pathlib import Path

from PIL import Image


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def build_index(directory, output):
    index = json.loads((directory / "index.json").read_text())
    frames = None
    members = {}
    for part in index["archives"]:
        path = directory / part["path"]
        if path.stat().st_size != part["bytes"] or sha256(path.read_bytes()) != part["sha256"]:
            raise ValueError(f"Archive integrity failure in {path.name}")
        seen = {}
        with tarfile.open(path, "r|xz") as archive:
            for item in archive:
                if item.name == "frame-map.json":
                    if frames is not None:
                        raise ValueError("Duplicate frame map")
                    frames = json.load(archive.extractfile(item))
                if not item.name.startswith("renders/"):
                    continue
                if item.name in members:
                    raise ValueError(f"Duplicate screenshot {item.name}")
                if item.islnk():
                    digests = seen[item.linkname]
                elif item.isfile():
                    png = archive.extractfile(item).read()
                    with Image.open(io.BytesIO(png)) as im:
                        if im.format != "PNG" or im.size != (240, 240):
                            raise ValueError(f"Wrong screenshot format for {item.name}")
                        digests = sha256(png), sha256(im.convert("RGB").tobytes())
                else:
                    raise ValueError(f"Unexpected archive member {item.name}")
                members[item.name] = (path.name, digests)
                seen[item.name] = digests
    if frames is None or len(members) != index["frames"] or len(frames) != len(members):
        raise ValueError("Missing screenshots or frame map")
    rows = []
    for frame in frames:
        name = frame["storedPath"]
        part, digests = members.pop(name)
        if digests != (frame["storedPNGsha256"], frame["pixelSha256"]):
            raise ValueError(f"Screenshot digest mismatch for {name}")
        rows.append((frame["id"], part, name, frame["evidenceKind"], digests[1]))
    if members or len({r[0] for r in rows}) != index["stableStateIDs"]:
        raise ValueError("Inconsistent state or screenshot count")
    with output.open("w", newline="") as stream:
        writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
        writer.writerow(("state_id", "archive", "png_path", "evidence_kind", "rgb_sha256"))
        writer.writerows(sorted(rows))
    print(f"Verified {len(rows)} archived 240x240 screenshots")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    build_index(args.directory, args.output)
