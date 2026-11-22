#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Build the static catalog GIF layer from hashed 240x240 renderer evidence."""
import argparse
from collections import defaultdict
import csv
import hashlib
import io
import json
from pathlib import Path
import tarfile
from zipfile import ZIP_DEFLATED, ZipFile

from PIL import Image


def digest(data):
    return hashlib.sha256(data).hexdigest()


def catalog(root, archive):
    groups = defaultdict(list)
    with (root / "screen-index.tsv").open() as index:
        for row in csv.DictReader(index, delimiter="\t"):
            groups[row["archive"]].append(row)

    records = {}
    for path, rows in groups.items():
        with tarfile.open(root / path) if path else io.BytesIO() as source:
            for row in rows:
                name = row["state_id"]
                if name in records:
                    raise ValueError(f"duplicate catalog state {name}")
                file = source.extractfile(row["png_path"]) if path else (root / row["png_path"]).open("rb")
                with file, Image.open(file) as original:
                    rgb = original.convert("RGB")
                if rgb.size != (240, 240) or digest(rgb.tobytes()) != row["rgb_sha256"]:
                    raise ValueError(f"catalog raster drift: {name}")
                data = io.BytesIO()
                rgb.save(data, format="GIF")
                body = data.getvalue()
                with Image.open(data) as rendered:
                    exact = rendered.convert("RGB").tobytes() == rgb.tobytes()
                member = f"catalog/production-199f0444/{name}.gif"
                archive.writestr(member, body)
                records[name] = {
                    "gif": member,
                    "gif_sha256": digest(body),
                    "source_rgb_sha256": row["rgb_sha256"],
                    "rgb_exact_after_gif_encoding": exact,
                    "evidence_kind": row["evidence_kind"],
                }
    return records


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("screens", type=Path)
    parser.add_argument("coverage", type=Path)
    parser.add_argument("shared_inputs", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    coverage = json.loads(args.coverage.read_text())
    with ZipFile(args.output, "x", compression=ZIP_DEFLATED) as archive:
        records = catalog(args.screens, archive)
        missing = coverage["role_map"].keys() - records.keys()
        if missing:
            raise ValueError(f"catalog missing mapped roles: {sorted(missing)}")
        for item in coverage["verified_media"]:
            path = item["path"]
            data = (args.shared_inputs / path).read_bytes()
            if digest(data) != item["expected_sha256"]:
                raise ValueError(f"reference drift: {path}")
            if path.lower().endswith(".gif"):
                archive.writestr(f"references/{path}", data)
        archive.writestr("catalog-index.json", json.dumps(records, indent=2, sort_keys=True) + "\n")
        archive.write(args.coverage, "canonical-input-index.json")
    with ZipFile(args.output) as archive:
        if archive.testzip() is not None:
            raise ValueError("archive CRC failed")
    print(f"Catalog {len(records)} GIFs, exact decoded RGB {sum(r['rgb_exact_after_gif_encoding'] for r in records.values())}; mapped roles {len(coverage['role_map'])}")


if __name__ == "__main__":
    main()
