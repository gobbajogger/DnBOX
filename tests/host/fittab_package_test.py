#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""FITTAB (the SMK Synth One B fit table at 0xFE000-0xFEFFF) in the build, the package and the installers.

  the region image is 4 KiB: 16 B header (magic FTB1, length 1388, zlib CRC-32, build id) + blob, 0xFF tail;
  the package carries it as a type-102 fittab.bin entry outside flash.bin; tools/fm1_install.py CRC-checks
  it and reads it back from the running firmware (ED_FITTAB); the web installer the same
  (web/test_fittab_readback.mjs). tools/gate_flashmap.py knows the region: it fails when it moves or when
  the loader allow-list does not cover exactly that sector."""
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
BLOB = ROOT / "firmware/src/app/fittab_blob.bin"


def region(blob=None):
    if blob is None:
        blob = BLOB.read_bytes()
    crc = zlib.crc32(blob) & 0xFFFFFFFF
    r = struct.pack("<IIII", build.FITTAB_MAGIC, len(blob), crc, crc) + blob
    return bytearray(r + b"\xff" * (0x1000 - len(r)))


class Image(unittest.TestCase):
    def test_placeholder(self):
        blob = BLOB.read_bytes()
        self.assertEqual(len(blob), 1388)
        self.assertEqual(len(blob), build.FITTAB_BYTES)
        words = list(struct.unpack("<" + "H" * 694, blob))
        self.assertEqual(words[0], 0xB000)
        self.assertEqual(words[-1], 0xB000 + 693)
        self.assertEqual(build.fittab_problems(bytes(region(blob))), [])

    def test_header(self):
        for k, (o, v) in enumerate([(0, 0), (8, 1), (12, 1), (4, 0x2000)]):
            with self.subTest(k=k):
                r = region()
                struct.pack_into("<I", r, o, v if o in (0, 4) else struct.unpack_from("<I", r, o)[0] ^ v)
                self.assertTrue(build.fittab_problems(bytes(r)))
        r = region()
        r[-1] = 0
        self.assertIn("FITTAB tail after the blob is not erased", build.fittab_problems(bytes(r)))
        self.assertTrue(build.fittab_problems(bytes(region())[:0x800]))

    def test_built_image(self):
        img = build.fittab_image()
        self.assertEqual(build.fittab_problems(img), [])
        magic, n, crc, ident = struct.unpack_from("<IIII", img, 0)
        self.assertEqual(magic, build.FITTAB_MAGIC)
        self.assertEqual((n, ident), (1388, zlib.crc32(BLOB.read_bytes()) & 0xFFFFFFFF))
        self.assertEqual(crc, ident)
        imgs = sorted((ROOT / "build").glob("smk37*/fittab.img"))
        for p in imgs:
            with self.subTest(img=str(p.relative_to(ROOT))):
                self.assertEqual(build.fittab_problems(p.read_bytes()), [])
                self.assertEqual(p.read_bytes(), img)


class Package(unittest.TestCase):
    def pkgs(self):
        flash, ota = bytes([0xA5]) * 0x9D000, bytes([0x3C]) * 3000
        ft = bytes(region())
        a = fm1_install.logical_image(fm1pkg_make.ufw(flash, ota, "SMK-37 Pro_900", fittab=ft))
        b = fm1_install.logical_image(fm1pkg_make.ufw(flash, ota, "SMK-37 Pro_900"))
        return a, b, ft

    def test_entry(self):
        a, b, ft = self.pkgs()
        self.assertIsNone(fm1_install.package_damage(a))
        self.assertEqual(fm1_install.package_fittab(a), ft)
        self.assertIsNone(fm1_install.package_fittab(b))
        self.assertIsNone(fm1_install.package_xdata(a))
        bad = bytearray(a)
        bad[len(bad) - 0x100] ^= 1
        self.assertEqual(fm1_install.package_damage(bytes(bad)), "fittab.bin: CRC")
        self.assertEqual(a[:len(b) - 0x100][0x400:0x9D000], b[:len(b) - 0x100][0x400:0x9D000])
        with self.assertRaises(SystemExit):
            fm1pkg_make.ufw(b"\xff" * 0x9D000, b"\x00" * 64, "SMK-37 Pro_900", fittab=ft[:0x800])

    def test_readback(self):
        _, _, ft = self.pkgs()

        class Link:
            def __init__(self, reply):
                self.reply, self.q = reply, []

            def drain(self):
                self.q.clear()

            def send(self, pkt):
                assert pkt == fm1_install.ED_HDR + bytes([fm1_install.ED_FITTAB, 0xF7])
                r = self.reply()
                if r is not None:
                    self.q.append(r)
                return True

            def read(self, timeout):
                return self.q.pop(0) if self.q else None

        def frame(ok, hdr):
            return fm1_install.ED_HDR + bytes([fm1_install.ED_FITTAB, ok]) + \
                bytes(v for b in hdr for v in (b & 0x7F, b >> 7)) + b"\xF7"
        self.assertEqual(fm1_install.fittab_readback(Link(lambda: frame(1, ft[:16])), ft), ft[:16])
        wrong = bytearray(ft[:16])
        wrong[9] ^= 0x80
        for reply in (lambda: frame(1, wrong), lambda: frame(0, ft[:16]), lambda: None):
            with self.assertRaises(fm1_install.InstallError) as c:
                fm1_install.fittab_readback(Link(reply), ft, timeout=0.05)
            self.assertEqual(c.exception.code, "fittab")

    def test_web(self):
        if not shutil.which("node"):
            self.skipTest("no node")
        a, b, _ = self.pkgs()
        with tempfile.TemporaryDirectory() as d:
            pa, pb = Path(d) / "a.bin", Path(d) / "b.bin"
            pa.write_bytes(a)
            pb.write_bytes(b)
            r = subprocess.run(["node", str(ROOT / "web/test_fittab_readback.mjs"), str(pa), str(pb)],
                               capture_output=True, text=True)
            self.assertEqual(r.returncode, 0, r.stdout + r.stderr)


class Gate(unittest.TestCase):
    def gate(self, hdr=None):
        with tempfile.TemporaryDirectory() as d:
            h = Path(d) / "fm1_flash_map.h"
            h.write_text(hdr if hdr is not None else HDR.read_text())
            st = Path(d) / "storage.c"
            st.write_text((ROOT / "firmware/src/storage.c").read_text())
            r = subprocess.run([sys.executable, str(ROOT / "tools/gate_flashmap.py"), str(h), str(st)],
                               capture_output=True, text=True)
            return r.returncode, r.stdout

    def test_current(self):
        rc, out = self.gate()
        self.assertEqual(rc, 0, out)
        self.assertIn("0x0fe000-0x0fefff    4 KiB  FITTAB (Synth One B fit table)", out)
        self.assertIn("0x0ff000-0x0fffff    4 KiB  key_mac", out)

    def test_moved(self):
        h = HDR.read_text().replace("#define FL_FITTAB_LO    0x000FE000u",
                                    "#define FL_FITTAB_LO    0x000FD000u")
        rc, out = self.gate(hdr=h)
        self.assertEqual(rc, 1, out)

    def test_loader_omits_sector(self):
        h = HDR.read_text().replace("FL_IN(off, n, FL_FITTAB_LO, FL_FITTAB_HI)", "0")
        rc, out = self.gate(hdr=h)
        self.assertEqual(rc, 1, out)
        self.assertIn("FL_LDR_RANGE_OK does not allow the FITTAB sector", out)


if __name__ == "__main__":
    unittest.main()
