#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PREP = ROOT / 'tests/host/prepare_clip_chord_graphics.py'
SOURCE = (ROOT / 'tests/host/smk_clip_chord_display_test.c').read_text()

class ExtractionGuard(unittest.TestCase):
    def test_duplicate_anchors_leave_host_unchanged(self):
        for anchor in ['static char chord_draw', 'static void text_c', 'static void fresh(void)', '#include "actual-clip-key-renderers.inc"', 'HOST_TEXT_C']:
            with self.subTest(anchor=anchor), tempfile.TemporaryDirectory() as temp:
                root = Path(temp)
                (root / 'tests/host').mkdir(parents=True)
                (root / 'out').mkdir()
                text = SOURCE + ('\n' + anchor if not anchor.startswith('#include') and anchor != 'HOST_TEXT_C' else '')
                (root / 'tests/host/smk_clip_chord_display_test.c').write_text(text)
                host = '#include "actual-clip-key-renderers.inc"\nstatic void text_c(\n'
                if anchor == 'HOST_TEXT_C':
                    host += 'static void text_c('
                if anchor.startswith('#include'):
                    host += anchor
                p = root / 'out/actual-clip-key-host.inc'
                p.write_text(host)
                result = subprocess.run([sys.executable, str(PREP), 'out'], cwd=root, capture_output=True, text=True)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('expected one extraction anchor', result.stderr)
                self.assertEqual(p.read_text(), host)
                self.assertFalse((root / 'out/chord-real.c').exists())

if __name__ == '__main__':
    unittest.main()
