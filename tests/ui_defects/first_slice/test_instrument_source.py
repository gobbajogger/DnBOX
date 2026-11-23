# SPDX-License-Identifier: GPL-3.0-only
"""The first-slice audit must instrument the current real panels, fail closed."""
from pathlib import Path
import shutil
import tempfile
import unittest
from instrument import instrument

ROOT = Path(__file__).resolve().parents[3]
FILES = ('firmware/src/gfx.c', 'firmware/src/app/ui.c',
         'firmware/src/app/ui_drum_voice.inc')

class CurrentSourceHooks(unittest.TestCase):
    def prepare(self, root):
        for name in FILES:
            path = root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / name, path)
        (root / 'tests/host').mkdir(parents=True)

    def test_all_three_real_panel_hooks(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            self.prepare(root)
            instrument(root)
            source = (root / FILES[2]).read_text()
            for role in ('control-banks', 'active-bank', 'voice-body', 'owner-stage-banks'):
                self.assertEqual(source.count('first_panel("'+role+'"'), 1)
            self.assertEqual(source.count('UI_PANEL_RADIUS'), 4)

    def test_native_curve_writes_observed_once(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            self.prepare(root)
            instrument(root)
            source = (root / FILES[2]).read_text()
            self.assertEqual(source.count('first_original_dv_curve_row(v,kick,top,bottom,col);'), 1)
            self.assertEqual(source.count('first_finish(before,"envelope-body",13,top,0);'), 1)

    def test_amp_plot_axes_observed_as_axis_only(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            self.prepare(root)
            instrument(root)
            source = (root / FILES[1]).read_text()
            self.assertEqual(source.count('first_finish(axis_before,"axis",x0,y0,0);'), 2)
            # The curve snapshot starts after both axes, so axis ink is never curve ink.
            self.assertLess(source.rindex('first_finish(axis_before'), source.index('first_snapshot(envelope_before)'))

    def test_missing_or_duplicate_curve_renderer_rejected(self):
        anchor = 'static UI_DRAW void dv_curve_row(drum_curve_t *v,int kick,int top,int bottom,uint16_t col)'
        for replacement in ('', anchor + anchor):
            with self.subTest(replacement=replacement), tempfile.TemporaryDirectory() as temp:
                root = Path(temp)
                self.prepare(root)
                path = root / FILES[2]
                source = path.read_text()
                self.assertEqual(source.count(anchor), 1)
                path.write_text(source.replace(anchor, replacement))
                with self.assertRaises(ValueError):
                    instrument(root)

    def test_owner_curve_and_shared_steps_observed(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp);self.prepare(root);instrument(root)
            source=(root/FILES[2]).read_text()
            self.assertEqual(source.count('first_finish(stage_before,"envelope-curve",13,top-off,0);'),1)
            self.assertEqual(source.count('first_finish(before,"step",sx,step_y,0);'),1)

    def test_owner_stage_missing_duplicate_rejected(self):
        anchor='static UI_DRAW void dv_bound_stage(drum_curve_t *v,int stage,int top,int off,uint16_t col)'
        for replacement in ('',anchor+anchor):
            with self.subTest(replacement=replacement),tempfile.TemporaryDirectory() as temp:
                root=Path(temp);self.prepare(root);p=root/FILES[2];source=p.read_text()
                self.assertEqual(source.count(anchor),1);p.write_text(source.replace(anchor,replacement))
                with self.assertRaises(ValueError):instrument(root)

    def test_missing_or_duplicate_panel_rejected(self):
        anchor = 'rbox(3, by, 234, body_h, UI_PANEL_RADIUS, SOUND_FACE);'
        for replacement in ('', anchor + anchor):
            with self.subTest(replacement=replacement), tempfile.TemporaryDirectory() as temp:
                root = Path(temp)
                self.prepare(root)
                path = root / FILES[2]
                source = path.read_text()
                self.assertEqual(source.count(anchor), 1)
                path.write_text(source.replace(anchor, replacement))
                with self.assertRaises(ValueError):
                    instrument(root)

if __name__ == '__main__':
    unittest.main()
