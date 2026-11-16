#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import importlib.util
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[3]
spec=importlib.util.spec_from_file_location('bounds_run',Path(__file__).with_name('run.py'))
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
GFX=(ROOT/'firmware/src/gfx.c').read_text()
UI=(ROOT/'firmware/src/app/ui.c').read_text()
class InstrumentTests(unittest.TestCase):
    def test_exact_input(self):
        output,dims=m.instrument(GFX,UI)
        self.assertEqual(dims['KNOB_Y'],158)
        self.assertIn('fwui_text_logical_outside(px,py,cv_h)',output)
        self.assertEqual(output.count('if (!a || px < clip_left'),1)
    def test_unique_graphics_anchors(self):
        for anchor in ['static int32_t cv_text_clip(', '    uint32_t r = c >> 11, g = (c >> 5) & 63u, b = c & 31u;', '                a = (gx & 1u) ? (a & 15u) : (a >> 4);']:
            with self.subTest(anchor=anchor):
                with self.assertRaises(ValueError):m.instrument(GFX+'\n'+anchor,UI)
                with self.assertRaises(ValueError):m.instrument(GFX.replace(anchor,''),UI)
    def test_unknown_layout(self):
        with self.assertRaises(ValueError):m.instrument(GFX,UI.replace('#define KNOB_Y 158','#define KNOB_Y 159'))
        with self.assertRaises(ValueError):m.instrument(GFX,UI+'\n#define KNOB_Y 158\n')
if __name__=='__main__':unittest.main()
