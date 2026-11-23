#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Default user-slot contents: the owner's 5 BREAK zones (assets/default-breaks), preloaded.

BR1 zones 1-3 go in user slot 1 (0xC2000, 80 KiB), BR2 zones 1-2 in user slot 2 (0xD6000). Slot 3 stays
empty. Each zone is the original encoded IMA ADPCM, byte for byte (no re-encode, no resample); the
slot header is the one tools/sampleio.user_slot writes (firmware/src/app/slots.c reads it).
They are defaults: an installer may write a slot image only into a slot with no valid FSMP header
(an empty slot or a fresh install), never over a slot that holds data.

  tools/default_breaks.py OUTDIR      writes slot1.img and slot2.img (header at 0, ADPCM at +512)
"""
import hashlib
import json
import struct
import sys
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASSETS = ROOT / "assets" / "default-breaks"
HDR_LEN, DATA_OFF, MAGIC, ROOT_BASE = 32 + 16 * 28, 512, 0x504D5346, 36


def key_split(roots):
    n = len(roots)
    return [(0 if j == 0 else (roots[j - 1] + r) // 2 + 1, 127 if j == n - 1 else (r + roots[j + 1]) // 2)
            for j, r in enumerate(roots)]


def load():
    meta = json.loads((ASSETS / "default-breaks.json").read_text())
    zones = []
    for a in meta["assets"]:
        data = (ASSETS / (a["id"] + ".adpcm")).read_bytes()
        if len(data) != a["adpcm_bytes"] or len(data) != (a["frames"] + 1) // 2:
            raise SystemExit(f"{a['id']}: {len(data)} B, want {a['adpcm_bytes']}")
        if hashlib.sha256(data).hexdigest() != a["adpcm_sha256"]:
            raise SystemExit(f"{a['id']}: ADPCM is not the pinned original")
        if a["sample_rate"] != 22050:
            raise SystemExit(f"{a['id']}: rate {a['sample_rate']}")
        zones.append(dict(a, adpcm=data))
    return meta, zones


def slot_image(name, zones, rate=22050):
    """[(adpcm, frames)] -> header (480 B) padded to 512 + ADPCM; predictor 0 / index 0 at sample 0"""
    data, zs = bytearray(), []
    for i, (adpcm, n) in enumerate(zones):
        zs.append((len(data), n, ROOT_BASE + i))
        data += adpcm
    q16 = int(round(rate / 44100 * 65536))
    zb = b"".join(struct.pack("<5I2h4B", off, n, 0, n - 1, q16, root * 16, 0, 0, lo, hi, 0)
                  for (off, n, root), (lo, hi) in zip(zs, key_split([z[2] for z in zs])))
    nm = name.upper().encode()[:8].ljust(8, b"\0")
    hdr = struct.pack("<IHBB8sII8x", MAGIC, 1, len(zs), 0, nm, len(data), zlib.crc32(data)) + zb
    return hdr.ljust(DATA_OFF, b"\0") + bytes(data)


def images():
    meta, zones = load()
    out = {}
    for s in meta["slots"]:
        mine = [(z["adpcm"], z["frames"]) for z in zones if z["slot"] == s["slot"]]
        out[s["slot"]] = slot_image(s["name"], mine)
    return out


def main(argv):
    if len(argv) != 2:
        raise SystemExit(__doc__)
    d = Path(argv[1])
    d.mkdir(parents=True, exist_ok=True)
    for k, img in images().items():
        (d / f"slot{k}.img").write_bytes(img)
        print(f"slot{k}.img {len(img)} B")


if __name__ == "__main__":
    main(sys.argv)
