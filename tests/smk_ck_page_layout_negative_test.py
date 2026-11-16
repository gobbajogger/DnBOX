#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import importlib.util
from pathlib import Path
import unittest
ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('layout', ROOT/'tests/smk_ck_page_layout_test.py')
layout = importlib.util.module_from_spec(spec)
spec.loader.exec_module(layout)
class NegativeControls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.main = (ROOT/'firmware/src/app/main_fm1.c').read_text()
        cls.header = (ROOT/'firmware/src/app/smk_clip_keys.h').read_text()
    def rejected(self, before, after):
        self.assertEqual(self.main.count(before), 1)
        self.assertTrue(any(not ok for ok, _ in layout.checks(self.main.replace(before, after), self.header)))
    def test_healthy(self):
        self.assertTrue(all(ok for ok, _ in layout.checks(self.main, self.header)))
    def test_held_key_lighting(self):
        self.rejected('CK_BIT(cp->keys, k) ? c', '0 ? c')
    def test_field_overlap(self):
        self.rejected('fy = 2 + (int32_t)(n % 5u) * 12', 'fy = 40 + (int32_t)(n % 5u) * 12')
    def test_keybed_bounds(self):
        self.rejected('ck_kbd(10, 76, 10, 46, CK_NKEY, col)', 'ck_kbd(10, 76, 10, 80, CK_NKEY, col)')
    def test_selected_field(self):
        self.rejected('rbox(fx - 2, fy', 'rbox(fx - 1, fy')
    def test_removed_instruction(self):
        self.rejected('static void ck_page_draw(void)\n{', 'static void ck_page_draw(void)\n{\n/* "PLAY KEYS" */')
    def test_ambiguous_source(self):
        with self.assertRaises(ValueError):
            layout.checks(self.main + '\nstatic void ck_page_draw(void)\n{', self.header)
if __name__ == '__main__':
    unittest.main()
