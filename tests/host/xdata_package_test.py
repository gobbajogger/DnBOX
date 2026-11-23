#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""XDATA (the SMK font region 0xAD000-0xAEFFF) in the build, the package and the installers.

  3. tools/build.py xdata_problems: no sector tail (0xADF00, 0xAEF00) of the region may pass the update
     loader's record test (magic 0x5441 at +6 and the CRC16 at +0), the header is exact, the tail erased;
     the region of the last build (build/smk37*/xdata.img) passes.
  4. the package carries the region as a type-101 xdata.bin entry outside flash.bin; tools/fm1_install.py
     CRC-checks it and reads it back from the running firmware (ED_XDATA); the web installer the same
     (web/test_xdata_readback.mjs).
  6. tools/gate_flashmap.py knows the region: it fails when the owner clear or FL_STORE_OK reach it, or when
     it moves."""
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import build  # noqa: E402
import fm1_install  # noqa: E402
import fm1pkg_make  # noqa: E402

HDR = ROOT / "firmware/hal/fm1_flash_map.h"
STORAGE = ROOT / "firmware/src/storage.c"


def region(blob=bytes(range(256)) * 30):
    crc = zlib.crc32(blob)
    r = struct.pack("<IIII", build.XDATA_MAGIC, len(blob), crc, crc) + blob
    return bytearray(r + b"\xff" * (0x2000 - len(r)))


def record(body=b"\x5a" * 78):
    r = bytearray(80)
    r[2:80] = body
    r[6:8] = b"\x41\x54"                          # 0x5441 little endian
    r[0:2] = struct.pack("<H", fm1pkg_make.crc16(r[2:80]))
    return r


class Tail(unittest.TestCase):
    def test_good(self):
        self.assertEqual(build.xdata_problems(bytes(region())), [])

    def test_record_tails(self):
        self.assertTrue(build.record_like(record()))
        for at in (0xF00, 0x1F00):
            with self.subTest(at=hex(at)):
                r = region(bytes(0x1F00 + 80 - 16))            # blob reaching both tails
                crc = zlib.crc32(r[16:16 + 0x1F00 + 80 - 16])
                r[at:at + 80] = record()
                crc = zlib.crc32(r[16:16 + 0x1F00 + 80 - 16])
                r[8:16] = struct.pack("<II", crc, crc)
                errs = build.xdata_problems(bytes(r))
                self.assertTrue(any(f"{0xAD000 + at:#x} looks like an update record" in e for e in errs), errs)
        r = region()
        r[0xF00:0xF00 + 80] = record()
        r[0xF00] ^= 1                                          # CRC no longer matches: not a record
        self.assertFalse(build.record_like(r[0xF00:0xF00 + 80]))

    def test_header(self):
        for k, (o, v) in enumerate([(0, 0), (8, 1), (12, 1), (4, 0x3000)]):
            with self.subTest(k=k):
                r = region()
                struct.pack_into("<I", r, o, v if o in (0, 4) else struct.unpack_from("<I", r, o)[0] ^ v)
                self.assertTrue(build.xdata_problems(bytes(r)))
        r = region()
        r[-1] = 0
        self.assertIn("XDATA tail after the blob is not erased", build.xdata_problems(bytes(r)))
        self.assertTrue(build.xdata_problems(bytes(region())[:0x1000]))

    def test_last_build(self):
        imgs = sorted((ROOT / "build").glob("smk37*/xdata.img"))
        if not imgs:
            self.skipTest("no build/smk37*/xdata.img (MODEL=pro|elite ./build.sh)")
        hdr = (ROOT / "build/gen/felucca_font.h").read_text()
        want_len = int(re.search(r"#define FONT_XDATA_LEN (\d+)u", hdr).group(1))
        want_id = int(re.search(r"#define FONT_XDATA_CRC32 0x([0-9A-F]+)u", hdr).group(1), 16)
        for p in imgs:
            with self.subTest(img=str(p.relative_to(ROOT))):
                r = p.read_bytes()
                self.assertEqual(build.xdata_problems(r), [])
                magic, n, crc, ident = struct.unpack_from("<IIII", r, 0)
                self.assertEqual((n, ident), (want_len, want_id))      # the font set this tree generates


class Package(unittest.TestCase):
    def pkgs(self):
        flash, ota = bytes([0xA5]) * 0x9D000, bytes([0x3C]) * 3000
        x = bytes(region())
        a = fm1_install.logical_image(fm1pkg_make.ufw(flash, ota, "SMK-37 Pro_900", xdata=x))
        b = fm1_install.logical_image(fm1pkg_make.ufw(flash, ota, "SMK-37 Pro_900"))
        return a, b, x

    def test_entry(self):
        a, b, x = self.pkgs()
        self.assertIsNone(fm1_install.package_damage(a))
        self.assertEqual(fm1_install.package_xdata(a), x)
        self.assertIsNone(fm1_install.package_xdata(b))
        bad = bytearray(a)
        bad[len(bad) - 0x1000] ^= 1
        self.assertEqual(fm1_install.package_damage(bytes(bad)), "xdata.bin: CRC")
        self.assertEqual(a[:len(b) - 0x100][0x400:0x9D000], b[:len(b) - 0x100][0x400:0x9D000])  # flash.bin unchanged
        with self.assertRaises(SystemExit):
            fm1pkg_make.ufw(b"\xff" * 0x9D000, b"\x00" * 64, "SMK-37 Pro_900", xdata=x[:0x1000])

    def test_readback(self):
        _, _, x = self.pkgs()

        class Link:
            def __init__(self, reply):
                self.reply, self.q = reply, []

            def drain(self):
                self.q.clear()

            def send(self, pkt):
                assert pkt == fm1_install.ED_HDR + bytes([fm1_install.ED_XDATA, 0xF7])
                r = self.reply()
                if r is not None:
                    self.q.append(r)
                return True

            def read(self, timeout):
                return self.q.pop(0) if self.q else None

        def frame(ok, hdr):
            return fm1_install.ED_HDR + bytes([fm1_install.ED_XDATA, ok]) + \
                bytes(v for b in hdr for v in (b & 0x7F, b >> 7)) + b"\xF7"
        self.assertEqual(fm1_install.xdata_readback(Link(lambda: frame(1, x[:16])), x), x[:16])
        wrong = bytearray(x[:16])
        wrong[9] ^= 0x80
        for reply in (lambda: frame(1, wrong), lambda: frame(0, x[:16]), lambda: None):
            with self.assertRaises(fm1_install.InstallError) as c:
                fm1_install.xdata_readback(Link(reply), x, timeout=0.05)
            self.assertEqual(c.exception.code, "xdata")

    def test_web(self):
        if not shutil.which("node"):
            self.skipTest("no node")
        a, b, _ = self.pkgs()
        with tempfile.TemporaryDirectory() as d:
            pa, pb = Path(d) / "a.bin", Path(d) / "b.bin"
            pa.write_bytes(a)
            pb.write_bytes(b)
            r = subprocess.run(["node", str(ROOT / "web/test_xdata_readback.mjs"), str(pa), str(pb)],
                               capture_output=True, text=True)
            self.assertEqual(r.returncode, 0, r.stdout + r.stderr)


class Gate(unittest.TestCase):
    def gate(self, hdr=None, storage=None):
        with tempfile.TemporaryDirectory() as d:
            h, s = Path(d) / "fm1_flash_map.h", Path(d) / "storage.c"
            h.write_text(hdr if hdr is not None else HDR.read_text())
            s.write_text(storage if storage is not None else STORAGE.read_text())
            r = subprocess.run([sys.executable, str(ROOT / "tools/gate_flashmap.py"), str(h), str(s)],
                               capture_output=True, text=True)
            return r.returncode, r.stdout

    def test_current(self):
        rc, out = self.gate()
        self.assertEqual(rc, 0, out)
        self.assertIn("0x0ad000-0x0aefff    8 KiB  XDATA (font bitmaps)", out)

    def test_owner_clear_reaches_xdata(self):
        st = STORAGE.read_text().replace("#define ST_CLEAR_HI     0xAD000u", "#define ST_CLEAR_HI     0xAF000u")
        rc, out = self.gate(storage=st)
        self.assertEqual(rc, 1, out)
        self.assertIn("ST_CLEAR_HI", out)

    def test_store_ok_reaches_xdata(self):
        h = HDR.read_text().replace("FL_IN(off, n, FL_DATA_LO, FL_XDATA_LO) || FL_IN(off, n, FL_XDATA_HI, FL_DATA_HI)",
                                    "FL_IN(off, n, FL_DATA_LO, FL_DATA_HI)")
        rc, out = self.gate(hdr=h)
        self.assertEqual(rc, 1, out)
        self.assertIn("FL_STORE_OK does not leave XDATA", out)

    def test_moved(self):
        h = HDR.read_text().replace("#define FL_XDATA_LO     0x000AD000u", "#define FL_XDATA_LO     0x000AC000u")
        rc, out = self.gate(hdr=h)
        self.assertEqual(rc, 1, out)


if __name__ == "__main__":
    unittest.main()
