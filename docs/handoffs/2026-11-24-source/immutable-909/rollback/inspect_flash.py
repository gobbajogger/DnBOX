#!/usr/bin/env python3
"""Inspect a complete SMK-37 snapshot without modifying it or contacting a device."""
import argparse
import hashlib
import json
from pathlib import Path

FLASH_BYTES = 0x100000
# Exact stock map. These two regions are worth measuring, not authorized writes.
REGIONS = {
    "factory_data": (0xE5000, 0xF4000),
    "sample_slot_3": (0xB3000, 0xBB000),
}
MOVE_BYTES = {"factory_data": 55307, "sample_slot_3": 31807}
KNOWN = {b"S1PK": "Synth One factory data", b"FSMP": "BREAK sample",
         b"D9RD": "guarded test companion"}


def inspect(image):
    if len(image) != FLASH_BYTES:
        raise ValueError("Expected exactly one complete 1 MiB SMK-37 flash snapshot")
    rows = {}
    for name, (lo, hi) in REGIONS.items():
        contents = bytes(image[lo:hi])
        occupied = len(contents.rstrip(b"\xff"))
        payload = MOVE_BYTES[name]
        rows[name] = dict(
            start=hex(lo), end=hex(hi), capacity=hi-lo, planned_payload=payload,
            raw_room=hi-lo-payload,
            content=KNOWN.get(contents[:4], "unknown contents") if occupied else "erased in snapshot",
            non_erased_extent=occupied,
            blank_in_snapshot=occupied == 0,
            sha256=hashlib.sha256(contents).hexdigest(),
            status="candidate for offline testing only" if occupied == 0 else "occupied; do not replace",
        )
    return {"snapshot_sha256": hashlib.sha256(image).hexdigest(),
            "expected_flash_layout": "stock SMK-37 Pro/Elite",
            "write_authorized": False, "regions": rows}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("snapshot", type=Path)
    args = parser.parse_args()
    print(json.dumps(inspect(args.snapshot.read_bytes()), indent=2))
