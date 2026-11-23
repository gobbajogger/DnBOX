#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""perf row 3: the owner's 5 BREAK zones as the default contents of user slots 1 and 2.

Checks: the 5 pinned original ADPCM zones (no substitutes); the slot images tools/default_breaks.py
builds decode, with the firmware's ima_decode, to the pinned PCM sample for sample; they fit slot 1
(80 KiB) and slot 2 for every FL_FDATA_SIZE the flash map allows up to the agreed 0xF000 (60 KiB);
slot 3 stays empty. Writes build/default-breaks/slotN.hdr/.bin for tests/scenarios/default_breaks.x0x
(the firmware's own slots.c parses them and the BREAK part plays them)."""
import hashlib
import json
import os
import struct
import subprocess
import sys
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import default_breaks as db  # noqa: E402

OUT = ROOT / "build" / "default-breaks"
OUT.mkdir(parents=True, exist_ok=True)
cc = os.environ.get("CC", "cc")
dec = OUT / "decode"
subprocess.run([cc, "-std=c99", "-O2", "-Wall", "-Wextra", "-Werror", "-I" + str(ROOT / "firmware/src"),
                str(ROOT / "tests/host/default_breaks_decode.c"), "-o", str(dec)], check=True)

meta, zones = db.load()
ids = [z["id"] for z in zones]
assert ids == ["personal-br1-zone1", "personal-br1-zone2", "personal-br1-zone3",
               "personal-br2-zone1", "personal-br2-zone2"], ids
assert [z["slot"] for z in zones] == [1, 1, 1, 2, 2]
imgs = db.images()
assert sorted(imgs) == [1, 2], "slot 3 stays empty"
SLOT1, SLOT3 = 0x14000, 0x8000
used = 0
for k, img in imgs.items():
    magic, ver, nz, _, name, dlen, crc = struct.unpack_from("<IHBB8sII", img)
    assert magic == 0x504D5346 and ver == 1 and len(img) == 512 + dlen and zlib.crc32(img[512:]) == crc
    mine = [z for z in zones if z["slot"] == k]
    assert nz == len(mine) and name.rstrip(b"\0") == f"BR{k}".encode()
    for i, z in enumerate(mine):
        off, n, ls, le, q16, root16, pred, idx, lo, hi, looped = struct.unpack_from("<5I2h4B", img, 32 + 28 * i)
        assert (n, ls, le, q16, pred, idx) == (z["frames"], 0, z["frames"] - 1, 32768, 0, 0)
        assert img[512 + off:512 + off + z["adpcm_bytes"]] == z["adpcm"], "original ADPCM bytes, unchanged"
        (OUT / f"slot{k}.bin").write_bytes(img[512:])
        pcm = subprocess.run([str(dec), str(OUT / f"slot{k}.bin"), str(off), str(n)], capture_output=True, check=True).stdout
        assert hashlib.sha256(pcm).hexdigest() == z["decoded_pcm_sha256"], f"{z['id']}: decoded PCM differs"
    (OUT / f"slot{k}.hdr").write_bytes(img[:480])
    used += len(img)
# Capacity: slot 2 is 0x1E000 - FL_FDATA_SIZE; the agreed FL_FDATA is 0xF000 (slot 2 = 60 KiB).
for fdata in range(0, 0xF000 + 1, 0x1000):
    sizes = (SLOT1, 0x1E000 - fdata, SLOT3)
    assert len(imgs[1]) <= sizes[0] and len(imgs[2]) <= sizes[1], (hex(fdata), len(imgs[1]), len(imgs[2]))
sizes = (SLOT1, 0x1E000 - 0xF000, SLOT3)
free = sizes[0] - len(imgs[1]) + sizes[1] - len(imgs[2]) + sizes[2]
print(f"default-breaks: slot1 BR1 {len(imgs[1])} B of {sizes[0]}, slot2 BR2 {len(imgs[2])} B of {sizes[1]} "
      f"(FL_FDATA 0xF000), slot3 empty {sizes[2]}; user capacity {sum(sizes)} B, defaults {used} B, free {free} B")
print("default-breaks: PASS 5 original zones, bit-exact PCM, fit")
