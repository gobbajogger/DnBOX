# SPDX-License-Identifier: GPL-3.0-only
"""Actual-source hook regression; not rendered geometry acceptance."""
import importlib.util
import os
import shlex
import shutil
import subprocess
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
                source = ui.read_text()
                self.assertEqual(source.count(LABEL), 1, 'strip mutation must alter the production hook')
                ui.write_text(source.replace(LABEL, mutation, 1))
            stage = root / 'firmware/src/app/ui_drum_voice.inc'
            if stage_mutation is not None:
                stage.write_text(stage.read_text().replace('    cv_poly_aa(points,used,col);', stage_mutation))
            MODULE.instrument(root)
            return stage.read_text() if return_stage else ui.read_text()

    def test_board_caption_default_is_a_c_constant(self):
        ui = (ROOT / 'firmware/src/app/ui.c').read_text()
        start = ui.index('#ifndef X0X_SMK_CAPTIONS')
        stop = ui.index('#if X0X_SMK_CAPTIONS', start)
        define = ui[start:stop]
        cc = shlex.split(os.environ.get('CC') or 'cc')
        for flags, expected in (
            ([], 0), (['-DBOARD_SMK37=0'], 0),
            (['-DBOARD_SMK37=1'], 1),
            (['-DBOARD_SMK37=1', '-DX0X_SMK_CAPTIONS=0'], 0),
            (['-DX0X_SMK_CAPTIONS=1'], 1),
        ):
            with self.subTest(flags=flags):
                result = subprocess.run(
                    [*cc, *flags, '-std=c11', '-fsyntax-only', '-x', 'c', '-'],
                    input=define + f'_Static_assert(X0X_SMK_CAPTIONS == {expected}, "caption mode");\n',
                    text=True, capture_output=True,
                )
                self.assertEqual(result.returncode, 0, result.stderr)

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
