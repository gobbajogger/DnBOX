#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""FL_FDATA (factory synth data at the end of user sample slot 2): the flash map gate,
the package slot map and slots.c agree for the stock size (0) and for a nonzero size.

The region must be whole sectors, slots 1/2/3 keep their start offsets, slot 1 (80 KiB) and
slot 3 (32 KiB) keep their sizes, and slot 2 ends where FL_FDATA starts."""
import re
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HDR = ROOT / "firmware/hal/fm1_flash_map.h"
STORAGE = ROOT / "firmware/src/storage.c"
sys.path.insert(0, str(ROOT / "tools"))
import fm1pkg_make  # noqa: E402


def with_size(n):
    src = HDR.read_text()
    out, k = re.subn(r"(#define FL_FDATA_SIZE\s+)0x[0-9A-Fa-f]+u", lambda m: f"{m.group(1)}0x{n:08X}u", src)
    assert k == 1
    return out


def gate(src):
    with tempfile.TemporaryDirectory() as d:
        p = Path(d) / "fm1_flash_map.h"
        p.write_text(src)
        r = subprocess.run([sys.executable, str(ROOT / "tools/gate_flashmap.py"), str(p), str(STORAGE)],
                           capture_output=True, text=True)
        return r.returncode, r.stdout


class FData(unittest.TestCase):
    def test_current_header(self):
        rc, out = gate(HDR.read_text())
        self.assertEqual(rc, 0, out)
        size = fm1pkg_make.fdata_size(HDR)
        self.assertEqual(fm1pkg_make.SLOTMAP["smk37"],
                         [(0xC2000, 0x14000), (0xD6000, 0x1E000 - size), (0xB3000, 0x8000)])
        self.assertIn(tuple(fm1pkg_make.SLOTMAP["smk37"]), fm1pkg_make.SLOTMAP_ALLOWED)

    def test_sizes(self):
        for n, ok in [(0, True), (0x1000, True), (0xD000, True), (0x19000, True), (0x1D000, True),
                      (0x1E000, False), (0x800, False), (0xD800, False)]:
            with self.subTest(size=hex(n)):
                rc, out = gate(with_size(n))
                self.assertEqual(rc == 0, ok, out)
                if ok:
                    self.assertIn(f"sample slot 2: 0xd6000-{0xF4000 - n - 1:#x}, {(0x1E000 - n) // 1024} KiB", out)
                    self.assertIn("sample slot 1", out)
                    if n:
                        self.assertIn(f"factory synth data: {n // 1024} KiB at {0xF4000 - n:#x}-0xf3fff", out)
                with tempfile.TemporaryDirectory() as d:
                    p = Path(d) / "h.h"
                    p.write_text(with_size(n))
                    if ok:
                        self.assertEqual(fm1pkg_make.fdata_size(p), n)
                    else:
                        with self.assertRaises(SystemExit):
                            fm1pkg_make.fdata_size(p)

    def test_slots_c_follows_the_region(self):
        s = (ROOT / "firmware/src/app/slots.c").read_text()
        self.assertIn("#define SMP_SLOT2_SIZE (0x1E000u - FL_FDATA_SIZE)", s)
        self.assertRegex(s, r"#define SMP_USER_OFF\(k\) \(\(k\) == 0u \? 0xC2000u : \(k\) == 1u \? 0xD6000u : 0xB3000u\)")
        self.assertIn("SMP_USER_OFF(1u) + SMP_USER_SIZE(1u) == FL_FDATA_LO", s)
        h = HDR.read_text()
        self.assertIn("#define FL_FDATA_HI     FL_SMP_HI", h)
        self.assertIn("#define FL_FDATA_LO     (FL_SMP_HI - FL_FDATA_SIZE)", h)


if __name__ == "__main__":
    unittest.main(verbosity=1)
