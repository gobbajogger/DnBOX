#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Assemble and decode-check a single FWUI evidence archive."""
import argparse
import hashlib
from io import BytesIO
import json
from pathlib import Path
import shutil
from zipfile import ZIP_DEFLATED, ZipFile

from PIL import Image


def add_member(archive, name, path):
    data = path.read_bytes()
    info = {"sha256": hashlib.sha256(data).hexdigest(), "bytes": len(data)}
    if path.suffix.lower() in {".gif", ".png"}:
        with Image.open(BytesIO(data)) as image:
            if image.size != (240, 240) and path.suffix.lower() == ".gif":
                raise ValueError(f"non-full-page GIF: {path}")
            for frame in range(image.n_frames):
                image.seek(frame)
                image.load()
            info.update(size=list(image.size), frames=image.n_frames)
    archive.writestr(name, data)
    return info


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("catalog", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--folder", action="append", type=Path, default=[])
    parser.add_argument("--index", type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise ValueError(f"output already exists: {args.output}")
    shutil.copyfile(args.catalog, args.output)
    allowed = {".gif", ".png", ".json", ".md", ".log", ".c", ".inc", ".py", ".patch", ".tsv", ".txt"}
    with ZipFile(args.output, "a", compression=ZIP_DEFLATED) as archive:
        existing = set(archive.namelist())
        cases = {}
        for folder in args.folder:
            for path in sorted(folder.rglob("*")):
                if not path.is_file() or path.is_symlink() or path.suffix.lower() not in allowed:
                    continue
                member = f"comparisons/{folder.name}/{path.relative_to(folder).as_posix()}"
                if member in existing:
                    raise ValueError(f"duplicate archive member: {member}")
                existing.add(member)
                cases[member] = add_member(archive, member, path)
        add_member(archive, "README.md", args.index)
        archive.writestr("comparison-file-index.json", json.dumps(cases, indent=2, sort_keys=True) + "\n")
    with ZipFile(args.output) as archive:
        if archive.testzip() is not None:
            raise ValueError("archive CRC failed")
        print(f"{len(archive.namelist())} archive members; {sum(name.endswith('.gif') for name in archive.namelist())} full-page GIFs")


if __name__ == "__main__":
    main()
