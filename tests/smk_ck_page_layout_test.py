#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Retain playable clip keys/fields without obsolete instructional captions."""
import re
import sys
from pathlib import Path

def block(source, anchor):
    if source.count(anchor) != 1:
        raise ValueError('Expected unique source anchor: ' + anchor)
    start = source.index(anchor)
    return source[start:source.index('\n}\n', start)]

def checks(main, header):
    pd = block(main, 'static void ck_page_draw(void)\n{')
    panel = block(main, 'uint8_t smk_ck_panel(uint32_t *last, uint32_t force)')
    field = re.search(r'fy = (\d+) \+ \(int32_t\)\(n % 5u\) \* (\d+)', pd)
    keys = re.search(r'ck_kbd\(10, (\d+), 10, (\d+), CK_NKEY, col\)', pd)
    return [
        ('CK_PAGE_FIELDS[pg][n]' in pd and 'CK_E_ON' in header and 'ck_get(f)' in pd,
         'page draws its selectable fields, including ON'),
        ('rbox(fx - 2, fy' in pd and 'f == (uint32_t)cp->pg >> 4' in pd,
         'page marks the selected field'),
        ('ck_zones' not in pd and 'CK_ZONE' not in header and 'k - lo == v' not in pd,
         'page does not show editable key zones or value keys'),
        ('CK_BIT(cp->keys, k) ? c' in pd and bool(keys),
         'playable keyboard retains held-key illumination and geometry'),
        ('ck_nm(CK_MODE_N, i)' in panel and 'cp->on >> i & 1u' in panel,
         'KEYS strip retains stage names and ON state'),
        ('"PLAY KEYS"' not in pd and 'KNOB2 FIELD  KNOB3 / < > VALUE  KEYS PLAY' not in pd,
         'normal page omits removed instructional captions'),
        (bool(field and keys) and int(field.group(1)) + 4 * int(field.group(2)) + 12 <= int(keys.group(1))
         and int(keys.group(1)) + int(keys.group(2)) <= 138,
         'five field rows and playable keybed fit without overlap'),
    ]

if __name__ == '__main__':
    root = Path(__file__).resolve().parents[1]
    outcomes = checks((root/'firmware/src/app/main_fm1.c').read_text(),
                      (root/'firmware/src/app/smk_clip_keys.h').read_text())
    for ok, message in outcomes:
        print(('PASS ' if ok else 'FAIL ') + message)
    fails = sum(not ok for ok, _ in outcomes)
    print(f'smk_ck_page_layout_test: {fails} fail(s)')
    sys.exit(bool(fails))
