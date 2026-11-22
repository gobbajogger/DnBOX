#!/usr/bin/env python3
"""Offline integrity/provenance check for the *historical* FWUI GIF archive.

This does not qualify an original 0.0.26 render, a current firmware capture,
any design comparison, or the final role-complete GIF set.

Usage: python3 fwui_archived_gif_integrity.py DnBOX-975-all-GIFs.zip --output report.json
Requires Pillow. The archive is kept in ChatGPT Library, not checked into Git.
"""
from __future__ import annotations
import argparse
import collections
import hashlib
import io
import json
import sys
import zipfile
from pathlib import Path

from PIL import Image

EXPECTED_ARCHIVE_SHA256 = "2c35210da618125088494bf6aac1c1fd21b855970d7cb0fc337df7af4e6e8e2f"
EXPECTED_GROUPS = {
    "01-firmware-checklist": 1712,
    "02-native-supplement": 40,
    "03-startup-loop": 16,
    "04-synth-one-opt-in": 88,
    "05-linked-stock-guard": 10,
    "06-renderer-motion": 6,
    "07-smc-pickup": 8,
}


def inspect(archive: Path) -> tuple[dict, bool]:
    checksum = hashlib.sha256(archive.read_bytes()).hexdigest()
    group_counts = collections.Counter()
    frame_counts = collections.Counter()
    sizes = collections.Counter()
    failures = []
    examples = {}
    with zipfile.ZipFile(archive) as z:
        failed_crc = z.testzip()
        for name in z.namelist():
            if not name.lower().endswith(".gif"):
                continue
            group_counts[name.split("/", 1)[0]] += 1
            try:
                data = z.read(name)
                with Image.open(io.BytesIO(data)) as picture:
                    size, count = picture.size, picture.n_frames
                    for i in range(count):
                        picture.seek(i)
                        picture.convert("RGB").load()
                sizes[f"{size[0]}x{size[1]}"] += 1
                frame_counts[count] += 1
                if size != (240, 240):
                    failures.append({"member": name, "error": f"size {size!r} != (240, 240)"})
                if name in (
                    "06-renderer-motion/checklist-battery.gif",
                    "06-renderer-motion/checklist-play--2.gif",
                    "07-smc-pickup/seq.gif",
                ):
                    examples[name] = {"frames": count, "sha256": hashlib.sha256(data).hexdigest()}
            except Exception as exc:
                failures.append({"member": name, "error": str(exc)})
    groups_match = dict(group_counts) == EXPECTED_GROUPS
    ok = (checksum == EXPECTED_ARCHIVE_SHA256 and failed_crc is None
          and not failures and groups_match and sum(group_counts.values()) == 1880)
    report = {
        "schema": "dnbox-fwui-historical-gif-integrity-v1",
        "status": "PASS_ARCHIVE_ONLY_NOT_ORIGINAL_026_NOT_CURRENT" if ok else "FAIL",
        "archive_file": archive.name,
        "archive_bytes": archive.stat().st_size,
        "archive_sha256": checksum,
        "expected_archive_sha256": EXPECTED_ARCHIVE_SHA256,
        "zip_crc_failure": failed_crc,
        "gif_total": sum(group_counts.values()),
        "group_counts": dict(sorted(group_counts.items())),
        "group_counts_match_expected": groups_match,
        "frame_count_distribution": dict(sorted(frame_counts.items())),
        "gif_size_distribution": dict(sorted(sizes.items())),
        "selected_historical_candidates": examples,
        "failures": failures,
        "excludes": ["original 0.0.26 pixel parity", "current 199f0444 framebuffer parity",
                     "complete R22 visual reference selection", "target Elite/Pro build"],
    }
    return report, ok


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    if not args.archive.is_file():
        parser.error(f"missing archive: {args.archive}")
    report, passed = inspect(args.archive)
    result = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.write_text(result)
    else:
        sys.stdout.write(result)
    print(report["status"], file=sys.stderr)
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
