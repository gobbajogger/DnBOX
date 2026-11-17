#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""The clip stage page lists selectable settings and depicts keys as playable notes."""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
main = (ROOT / "firmware/src/app/main_fm1.c").read_text()
header = (ROOT / "firmware/src/app/smk_clip_keys.h").read_text()
d0 = main.index("static void ck_page_draw(void)\n{")
pd = main[d0:main.index("\n}\n", d0)]
p0 = main.index("uint8_t smk_ck_panel(uint32_t *last, uint32_t force)")
panel = main[p0:main.index("\n}\n", p0)]
fails = 0


def check(ok, msg):
    global fails
    fails += not ok
    print(("PASS " if ok else "FAIL ") + msg)


check("CK_PAGE_FIELDS[pg][n]" in pd and "CK_E_ON" in header and "ck_get(f)" in pd,
      "page draws its selectable fields, including ON")
check("rbox(fx - 2, fy" in pd and "f == (uint32_t)cp->pg >> 4" in pd,
      "page marks the selected field")
check("ck_zones" not in pd and "CK_ZONE" not in header and "k - lo == v" not in pd,
      "page does not show editable key zones or value keys")
check("CK_BIT(cp->keys, k) ? c" in pd and
      "ck_kbd(10, 76, 10, 46, CK_NKEY, col)" in pd,
      "page draws its playable keyboard and lights held keys")
check("ck_nm(CK_MODE_N, i)" in panel and "cp->on >> i & 1u" in panel,
      "KEYS strip retains stage names and ON state")
check('"PLAY KEYS"' not in pd and '"KNOB2 FIELD' not in pd,
      "page has no explanatory keyboard or knob instructions")
field = re.search(r"fy = (\d+) \+ \(int32_t\)\(n % 5u\) \* (\d+)", pd)
keyboard = re.search(r"ck_kbd\(10, (\d+), 10, (\d+), CK_NKEY, col\)", pd)
check(bool(field and keyboard) and int(field.group(1)) + 4 * int(field.group(2)) + 12 <= int(keyboard.group(1))
      and int(keyboard.group(1)) + int(keyboard.group(2)) <= 138,
      "fields and keyboard do not overlap and fit the stage page")
print(f"smk_ck_page_layout_test: {fails} fail(s)")
sys.exit(1 if fails else 0)
