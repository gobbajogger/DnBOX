# SPDX-License-Identifier: GPL-3.0-only
"""Actual-source hook regression; not rendered geometry acceptance."""
import importlib.util
import shutil
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
SPEC = importlib.util.spec_from_file_location('geometry_instrument', ROOT / 'tests/ui_geometry/instrument.py')
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)
LABEL = '        text_c(cx, vc(&FONT_XS, mix_kit >= 0 && i == 0 ? 66 : 13, 10), &FONT_XS, caps_of(pref_name(r)), touched ? C_WHITE : C_GRAY);'

class InstrumentSourceTests(unittest.TestCase):
    def exercise(self, mutation=None, stage_mutation=None, return_stage=False):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for name in ('firmware/src/gfx.c', 'firmware/src/app/ui.c',
                         'firmware/src/app/ui_drum_voice.inc',
                         'tests/ui_defects/first_slice/instrument.py'):
                target = root / name
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(ROOT / name, target)
            ui = root / 'firmware/src/app/ui.c'
            if mutation:
                ui.write_text(ui.read_text().replace(LABEL, mutation))
            stage = root / 'firmware/src/app/ui_drum_voice.inc'
            if stage_mutation is not None:
                stage.write_text(stage.read_text().replace('    cv_poly_aa(points,used,col);', stage_mutation))
            MODULE.instrument(root)
            return stage.read_text() if return_stage else ui.read_text()

    def test_actual_flat_strip_is_instrumented(self):
        output = self.exercise()
        self.assertEqual(output.count('if(!getenv("PRESENCE_MUTATE_STRIP"))' + LABEL.strip()), 1)

    def test_bound_stage_tracks_real_curve_pixels(self):
        output = self.exercise(return_stage=True)
        self.assertEqual(output.count('uint16_t stage_before[CV_MAX];pixel_snapshot(stage_before);'), 1)
        self.assertIn('stage_before[yy*cv_w+xx]!=cv_px[yy*cv_w+xx]', output)
        self.assertIn('ENVELOPE_MUTATE_CURVE', output)
        self.assertIn('pixel_tile_owner[yy*cv_w+xx]=stage==1', output)

    def test_missing_duplicate_bound_curve_rejected(self):
        anchor = '    cv_poly_aa(points,used,col);'
        for replacement in ('/* missing actual curve */', anchor+'\n'+anchor):
            with self.subTest(replacement=replacement), self.assertRaisesRegex(ValueError, 'expected exactly one hook'):
                self.exercise(stage_mutation=replacement)

    def test_absent_or_duplicate_strip_hook_rejected(self):
        for mutation in ('/* missing strip label */', LABEL + '\n' + LABEL):
            with self.subTest(mutation=mutation), self.assertRaisesRegex(ValueError, 'expected exactly one hook'):
                self.exercise(mutation)

if __name__ == '__main__':
    unittest.main()
