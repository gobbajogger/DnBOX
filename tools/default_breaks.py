#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Default user-slot contents: the owner's 5 BREAK zones (assets/default-breaks), preloaded.

BR1 zones 1-3 go in user slot 1 (0xC2000, 80 KiB), BR2 zones 1-2 in user slot 2 (0xD6000). Slot 3 stays
empty. Each zone is the original encoded IMA ADPCM, byte for byte (no re-encode, no resample); the
slot header is the one tools/sampleio.user_slot writes (firmware/src/app/slots.c reads it).
They are defaults, written on a fresh install only; users overwrite or clear them like any slot.
The installer chooses all five (the default) or any subset; a later install never refills a slot
or a zone the user cleared or omitted.

Listen before choosing: https://gobbajogger.github.io/DnBOX-nightly/starting-breaks/
  WAV: https://gobbajogger.github.io/DnBOX-nightly/private-audio/<id>.wav
  Nightly tests: tests/product-starting-break-preview.test.mjs, tests/product-break-http.test.mjs

  tools/default_breaks.py OUTDIR [WHICH]   writes slot images (header at 0, ADPCM at +512)
  WHICH is all (default), none, or a comma-separated list (personal-br1-zone1 or br1-1, ...).

fill_fresh(link, fresh_install, select=None) is the install-path step (tools/fm1_install.py
--fresh-install [--default-breaks WHICH]): only on a fresh install (the caller's explicit flag)
and only when every slot is blank does it write the selected defaults; it never erases or
overwrites a slot, and a later install never refills one, so a default the user cleared
(or a zone they omitted) stays cleared.
"""
import hashlib
import json
import struct
import sys
import zlib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

ROOT = Path(__file__).resolve().parent.parent
ASSETS = ROOT / "assets" / "default-breaks"
HDR_LEN, DATA_OFF, MAGIC, ROOT_BASE = 32 + 16 * 28, 512, 0x504D5346, 36

ZONE_IDS = (
    "personal-br1-zone1",
    "personal-br1-zone2",
    "personal-br1-zone3",
    "personal-br2-zone1",
    "personal-br2-zone2",
)
PREVIEW_PAGE = "https://gobbajogger.github.io/DnBOX-nightly/starting-breaks/"
PREVIEW_WAV = "https://gobbajogger.github.io/DnBOX-nightly/private-audio/{id}.wav"
PREVIEW_RAW = "https://gobbajogger.github.io/DnBOX-nightly/private-audio/{id}"
PREVIEW_TESTS = (
    "https://github.com/gobbajogger/DnBOX-nightly/blob/HEAD/tests/product-starting-break-preview.test.mjs",
    "https://github.com/gobbajogger/DnBOX-nightly/blob/HEAD/tests/product-break-http.test.mjs",
)


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
    ids = [z["id"] for z in zones]
    if ids != list(ZONE_IDS):
        raise SystemExit(f"default-breaks.json ids {ids} != {list(ZONE_IDS)}")
    return meta, zones


def preview_wav(pcm, rate=22050):
    """Standard mono 16-bit PCM WAV (44-byte header), the nightly /private-audio/ preview form."""
    data_len = len(pcm)
    return struct.pack("<4sI4s4sIHHIIHH4sI",
                       b"RIFF", 36 + data_len, b"WAVE", b"fmt ", 16,
                       1, 1, rate, rate * 2, 2, 16, b"data", data_len) + pcm


def preview_urls(zone_id):
    if zone_id not in ZONE_IDS:
        raise ValueError(f"unknown default BREAK {zone_id!r}")
    return {
        "page": PREVIEW_PAGE,
        "wav": PREVIEW_WAV.format(id=zone_id),
        "raw": PREVIEW_RAW.format(id=zone_id),
        "tests": list(PREVIEW_TESTS),
    }


def _aliases(zones=None):
    if zones is None:
        _, zones = load()
    out = {}
    for z in zones:
        i = z["id"]
        out[i.lower()] = i
        out[i.lower().removeprefix("personal-")] = i          # br1-zone1
        out[i.lower().replace("personal-", "").replace("-zone", "-")] = i  # br1-1
        out[z["name"].lower()] = i                            # br1 1
        out[z["name"].lower().replace(" ", "-")] = i          # br1-1
    return out


def parse_selection(spec=None):
    """Which of the 5 defaults to preload, in official order.

    None / 'all' / '*' -> all five. 'none' / '' -> []. Otherwise a comma-separated
    list of ids or short names (personal-br1-zone1, br1-zone1, br1-1, 'BR1 1'),
    or an iterable of the same. Unknown names raise ValueError."""
    if spec is None:
        return list(ZONE_IDS)
    if not isinstance(spec, str):
        tokens = list(spec)
        if not tokens:
            return []
        if any(not isinstance(t, str) for t in tokens):
            raise ValueError(f"unknown default BREAK {tokens!r}; want all, none, or "
                             + ", ".join(ZONE_IDS))
        # a pre-parsed list of official ids stays official-order, no 'all' expansion mid-list
        if all(t in ZONE_IDS for t in tokens):
            return [i for i in ZONE_IDS if i in set(tokens)]
        return parse_selection(",".join(tokens))
    text = spec.strip()
    if not text or text.lower() in ("none", "0"):
        return []
    if text.lower() in ("all", "*"):
        return list(ZONE_IDS)
    tokens = [p.strip() for p in text.split(",") if p.strip()]
    if not tokens:
        return []
    aliases = _aliases()
    want = set()
    for tok in tokens:
        key = tok.lower()
        if key in ("all", "*"):
            return list(ZONE_IDS)
        if key in ("none", "0"):
            continue
        if key not in aliases:
            raise ValueError(
                f"unknown default BREAK {tok!r}; want all, none, or "
                + ", ".join(ZONE_IDS)
                + " (short: br1-1, br1-2, br1-3, br2-1, br2-2). "
                f"Listen first: {PREVIEW_PAGE}"
            )
        want.add(aliases[key])
    return [i for i in ZONE_IDS if i in want]


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


def images(select=None):
    meta, zones = load()
    want = set(parse_selection(select))
    out = {}
    for s in meta["slots"]:
        mine = [(z["adpcm"], z["frames"]) for z in zones if z["slot"] == s["slot"] and z["id"] in want]
        if mine:
            out[s["slot"]] = slot_image(s["name"], mine)
    return out


# where each default goes: slot index -> the flash offset it was laid out for (slots.c SMP_USER_OFF)
HOME = {1: 0xC2000, 2: 0xD6000}


def plan_fill(lay, imgs=None, select=None):
    """SMP_INFO layout (x0x_slots.parse_info) of a blank device -> [(slot index k, image)].
    [] unless EVERY slot is blank (0 zones, 0 KiB used): defaults are for a fresh install only."""
    imgs = images(select) if imgs is None else imgs
    if any(sl["nz"] or sl["used"] for sl in lay["slots"]):
        return []
    todo = []
    for slot, img in sorted(imgs.items()):
        k = slot - 1
        if k >= lay["count"]:
            continue
        sl = lay["slots"][k]
        if lay["ext"] and sl["off"] != HOME[slot]:
            continue                                     # another map: not where the default belongs
        if len(img) > sl["size"]:
            continue
        todo.append((k, img))
    return todo


def fill_fresh(link, fresh_install, progress=None, select=None):
    """Fresh install only: write the selected defaults when the caller says this is the first
    X0X install on the keyboard (fresh_install=True) AND every slot is blank (SMP_INFO and
    the header bytes). A later install never refills a slot, so a default the user cleared
    (or a zone they left out) stays cleared.
    select: None/'all' or a parse_selection() spec.
    -> list of slot numbers filled (1-based)"""
    import x0x_slots as xs
    if not fresh_install:
        return []
    ids = parse_selection(select)
    if not ids:
        return []
    lay = xs.layout(link)
    todo = plan_fill(lay, select=ids)
    if not todo:
        return []
    for k in range(lay["count"]):                        # every header still erased: nothing was there
        if any(b != 0xFF for b in xs.read_bytes(link, k, 0, xs.HDR_LEN)):
            return []
    filled = []
    for k, img in todo:
        xs.write_slot(link, k, img, progress)
        if bytes(xs.read_bytes(link, k, 0, len(img))) != img:
            raise xs.SlotError(f"slot {k + 1}: default image did not read back")
        filled.append(k + 1)
    return filled


def main(argv):
    if len(argv) not in (2, 3):
        raise SystemExit(__doc__)
    try:
        select = parse_selection(argv[2] if len(argv) == 3 else "all")
    except ValueError as e:
        raise SystemExit(str(e))
    d = Path(argv[1])
    d.mkdir(parents=True, exist_ok=True)
    imgs = images(select)
    if not imgs:
        print("no slots written (none selected)")
        return
    for k, img in imgs.items():
        (d / f"slot{k}.img").write_bytes(img)
        print(f"slot{k}.img {len(img)} B")


if __name__ == "__main__":
    main(sys.argv)
