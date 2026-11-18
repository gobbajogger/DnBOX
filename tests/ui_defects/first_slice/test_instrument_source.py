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
            for role in ('control-banks', 'active-bank', 'voice-body'):
                self.assertEqual(source.count('first_panel("'+role+'"'), 1)
            self.assertEqual(source.count('UI_PANEL_RADIUS'), 3)

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
