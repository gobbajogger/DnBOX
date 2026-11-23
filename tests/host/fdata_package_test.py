#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Tracked factory data, app-side loader region, and no-device updater readback."""
import hashlib
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import build
import fm1_install
import fm1pkg_make as pm


class FactoryData(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.folder = Path(cls.temp.name)
        out = cls.folder / "factory.bin"
        subprocess.run([sys.executable, str(ROOT / "tools/pack_synthone_factory.py"), str(out)],
                       check=True, stdout=subprocess.DEVNULL)
        cls.data = out.read_bytes()
        cls.region = cls.data + b"\xff" * (pm.FDATA_SIZE - len(cls.data))
        cls.flash = bytes([0xa5]) * 0x9d000
        cls.ota = bytes([0x3c]) * 3000
        cls.logical = fm1_install.logical_image(pm.ufw(cls.flash, cls.ota, "SMK-37 Pro_900", fdata=cls.region))

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def test_factory_contents_and_corruption(self):
        self.assertEqual(len(self.data), 61208)
        self.assertEqual(hashlib.sha256(self.data).hexdigest(),
                         "c6c39e8425e51ede8e46b94b7272a7c96e2987894f2e9c0b598d312c7fe04d44")
        self.assertTrue(pm.valid_fdata(self.region))
        for i in (0, 4, 6, 12, 16, 650, len(self.region) - 1):
            bad = bytearray(self.region)
            bad[i] ^= 1
            with self.subTest(offset=i):
                self.assertFalse(pm.valid_fdata(bad))

    def test_entry_and_verified_readback(self):
        self.assertIsNone(fm1_install.package_damage(self.logical))
        self.assertEqual(fm1_install.package_fdata(self.logical), self.region)
        pos = self.logical.index(self.region)
        broken = bytearray(self.logical)
        broken[pos + 60] ^= 1
        self.assertIsNotNone(fm1_install.package_damage(broken))
        self.assertIsNone(fm1_install.package_fdata(broken))

        head = self.region[:32]

        class Link:
            def __init__(self, reply):
                self.reply = reply

            def send(self, payload):
                self.sent = bytes(payload)
                return True

            def drain(self):
                pass

            def read(self, _timeout):
                return self.reply

        body = bytes([1]) + bytes(y for x in head for y in (x & 0x7f, x >> 7))
        prefix = fm1_install.ED_HDR + bytes([fm1_install.ED_FDATA])
        good = Link(prefix + body + b"\xf7")
        self.assertEqual(fm1_install.fdata_readback(good, self.region), head)
        self.assertEqual(good.sent, prefix + b"\xf7")
        bad = Link(prefix + b"\0" + body[1:] + b"\xf7")
        with self.assertRaises(fm1_install.InstallError):
            fm1_install.fdata_readback(bad, self.region)

    def test_native_stock_defaults_to_compact_without_changing_host_default(self):
        self.assertTrue(build.native_compact_synthone("smk37", "elite", 0xf000, None))
        self.assertTrue(build.native_compact_synthone("smk37", "pro", 0xf000, None))
        self.assertFalse(build.native_compact_synthone("smk37", "mke", 0xf000, None))
        self.assertFalse(build.native_compact_synthone("fm1", "pro", 0xf000, None))
        self.assertFalse(build.native_compact_synthone("smk37", "pro", 0xf000, "0"))
        env = dict(os.environ)
        env.pop("X0X_S1C", None)
        self.assertEqual(subprocess.check_output([sys.executable, "-c",
            "from tools import build; print(build.S1C)"], cwd=ROOT, env=env, text=True).strip(), "0")

    def test_boot_initializes_pack_and_editor_reports_bound_state(self):
        main = (ROOT / "firmware/src/app/main_fm1.c").read_text()
        self.assertRegex(main, r"if \(flash_ok\)\s*\(void\)s1_part_set_pack\(fm1_xip_ptr\(FL_FDATA_LO\), FL_FDATA_SIZE\);\s*#endif\s*engine_init\(")
        editor = (ROOT / "firmware/src/app/editor.c").read_text()
        self.assertRegex(editor, r"case ED_FDATA:[\s\S]+?ed_b\(s1_part_programs\(\) != 0\);")


if __name__ == "__main__":
    unittest.main()
