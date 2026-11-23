#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""perf rows 3 and 4 (firmware): the owner's 5 BREAK zones as default user-slot contents,
with an installer choice of all or any and a link to the nightly preview WAVs.

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

# The install path: fresh install only (mocked device, no hardware).
sys.path.insert(0, str(ROOT / "tests"))
from slot_mock import MockDevice  # noqa: E402
import x0x_slots as xs  # noqa: E402
AGREED = [(0xC2000, 0x14000), (0xD6000, 0x1E000 - 0xF000), (0xB3000, 0x8000)]
NOW = [(0xC2000, 0x14000), (0xD6000, 0x1E000), (0xB3000, 0x8000)]
for slots in (AGREED, NOW):
    dev = MockDevice(slots=slots)                        # blank keyboard
    assert db.fill_fresh(dev, fresh_install=False) == [], "no flag: an ordinary install writes nothing"
    assert all(b == 0xFF for b in dev.fl.mem[0xB3000:0xF4000]), "nothing written without the flag"
    assert db.fill_fresh(dev, fresh_install=True) == [1, 2], "fresh install gets both defaults"
    assert [s["nz"] for s in xs.layout(dev)["slots"]] == [3, 2, 0]
    assert bytes(dev.fl.mem[0xC2000:0xC2000 + len(imgs[1])]) == imgs[1]
    assert bytes(dev.fl.mem[0xD6000:0xD6000 + len(imgs[2])]) == imgs[2]
    assert db.fill_fresh(dev, fresh_install=True) == [], "a second install changes nothing"
# The user clears default slot 1 (erase): later installs, flagged or not, never bring it back.
dev = MockDevice(slots=AGREED)
db.fill_fresh(dev, fresh_install=True)
xs.write_slot(dev, 0, None)
cleared = bytes(dev.fl.mem[0xC2000:0xD6000])
for flag in (False, True):
    assert db.fill_fresh(dev, fresh_install=flag) == []
assert bytes(dev.fl.mem[0xC2000:0xD6000]) == cleared and xs.layout(dev)["slots"][0]["nz"] == 0, "cleared stays cleared"
# The user clears BOTH defaults: an ordinary install still writes nothing.
xs.write_slot(dev, 1, None)
assert db.fill_fresh(dev, fresh_install=False) == []
assert xs.layout(dev)["slots"][0]["nz"] == 0 and xs.layout(dev)["slots"][1]["nz"] == 0
# A user's own loop anywhere: even a flagged install writes nothing at all.
dev = MockDevice(slots=AGREED)
own = db.slot_image("MINE", [(bytes(range(256)) * 8, 4096)])
xs.write_slot(dev, 2, own)
before = bytes(dev.fl.mem[0xB3000:0xF4000])
assert db.fill_fresh(dev, fresh_install=True) == []
assert bytes(dev.fl.mem[0xB3000:0xF4000]) == before, "user data and empty slots untouched"
# A different slot map (slot 1 elsewhere): nothing is written there.
dev = MockDevice(slots=[(0xA0000, 0x14000), (0xB4000, 0x14000)])
assert db.fill_fresh(dev, fresh_install=True) == []
print("default-breaks: PASS fresh-install-only path (flag + all blank; repeat, cleared stays cleared, user data, foreign map)")

# Row 4 (firmware): choose all or any of the 5, with nightly preview URLs to listen first.
# Selection is an installer option (tools/fm1_install.py --default-breaks); no new UI.
assert db.parse_selection(None) == ids and db.parse_selection("all") == ids
assert db.parse_selection("none") == [] and db.parse_selection("") == []
assert db.parse_selection("br1-1, personal-br2-zone2") == ["personal-br1-zone1", "personal-br2-zone2"]
assert db.parse_selection(["BR1 2", "br2-1", "br1-2"]) == ["personal-br1-zone2", "personal-br2-zone1"]
try:
    db.parse_selection("personal-br9-zone1")
except ValueError as e:
    assert "unknown default BREAK" in str(e) and db.PREVIEW_PAGE in str(e)
else:
    raise SystemExit("bad id must raise")
# Manifest preview links and WAV hashes match the nightly contract (decoded PCM + 44-byte header).
assert meta["preview"]["page"] == db.PREVIEW_PAGE
assert meta["preview"]["wav"] == db.PREVIEW_WAV and meta["preview"]["raw"] == db.PREVIEW_RAW
assert meta["preview"]["tests"] == list(db.PREVIEW_TESTS)
for z in zones:
    urls = db.preview_urls(z["id"])
    assert urls["page"] == db.PREVIEW_PAGE
    assert urls["wav"] == f"https://gobbajogger.github.io/DnBOX-nightly/private-audio/{z['id']}.wav"
    assert urls["raw"] == f"https://gobbajogger.github.io/DnBOX-nightly/private-audio/{z['id']}"
    assert urls["tests"] == list(db.PREVIEW_TESTS)
# Rebuild the nightly preview WAV from the firmware-decoded PCM and check the pinned hash.
for k, img in imgs.items():
    mine = [z for z in zones if z["slot"] == k]
    for i, z in enumerate(mine):
        off, n, *_ = struct.unpack_from("<5I2h4B", img, 32 + 28 * i)
        pcm = subprocess.run([str(dec), str(OUT / f"slot{k}.bin"), str(off), str(n)],
                             capture_output=True, check=True).stdout
        wav = db.preview_wav(pcm, z["sample_rate"])
        assert len(wav) == z["preview_bytes"] == 44 + z["frames"] * 2, z["id"]
        assert hashlib.sha256(wav).hexdigest() == z["preview_sha256"], f"{z['id']}: preview WAV differs"
print("default-breaks: PASS preview URLs + WAV hashes (nightly private-audio form)")

# Subset preload: only the chosen zones are written; unselected slots stay empty.
dev = MockDevice(slots=AGREED)
pick = ["personal-br1-zone1", "personal-br1-zone3", "personal-br2-zone2"]
assert db.fill_fresh(dev, fresh_install=True, select=pick) == [1, 2]
assert [s["nz"] for s in xs.layout(dev)["slots"]] == [2, 1, 0]
want = db.images(pick)
assert bytes(dev.fl.mem[0xC2000:0xC2000 + len(want[1])]) == want[1]
assert bytes(dev.fl.mem[0xD6000:0xD6000 + len(want[2])]) == want[2]
# A later install, even with --default-breaks=all, does not add the omitted zones.
before = bytes(dev.fl.mem[0xB3000:0xF4000])
assert db.fill_fresh(dev, fresh_install=True, select="all") == []
assert bytes(dev.fl.mem[0xB3000:0xF4000]) == before and [s["nz"] for s in xs.layout(dev)["slots"]] == [2, 1, 0]
# Only slot-2 zones: slot 1 stays erased (empty), slot 2 is written.
dev = MockDevice(slots=AGREED)
assert db.fill_fresh(dev, fresh_install=True, select="br2-1,br2-2") == [2]
assert [s["nz"] for s in xs.layout(dev)["slots"]] == [0, 2, 0]
assert all(b == 0xFF for b in dev.fl.mem[0xC2000:0xC2000 + 32])
# --default-breaks=none writes nothing.
dev = MockDevice(slots=AGREED)
assert db.fill_fresh(dev, fresh_install=True, select="none") == []
assert all(b == 0xFF for b in dev.fl.mem[0xB3000:0xF4000])
# Per-zone clear: after a full preload the user drops one zone from slot 1 (rewrites the
# slot with the remaining zones). Later installs do not put the cleared zone back.
dev = MockDevice(slots=AGREED)
assert db.fill_fresh(dev, fresh_install=True) == [1, 2]
kept = [z for z in zones if z["id"] in ("personal-br1-zone1", "personal-br1-zone3")]
cleared = db.slot_image("BR1", [(z["adpcm"], z["frames"]) for z in kept])
xs.write_slot(dev, 0, cleared)
after = bytes(dev.fl.mem[0xC2000:0xC2000 + len(cleared)])
assert after == cleared and xs.layout(dev)["slots"][0]["nz"] == 2
assert db.fill_fresh(dev, fresh_install=True, select="all") == []
assert bytes(dev.fl.mem[0xC2000:0xC2000 + len(cleared)]) == after
assert xs.layout(dev)["slots"][0]["nz"] == 2 and xs.layout(dev)["slots"][1]["nz"] == 2
print("default-breaks: PASS choose-any (--default-breaks subset/none) and per-zone clear stays cleared")
