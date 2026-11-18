#!/usr/bin/env python3
"""Exact original-law restoration and real arithmetic corruption controls."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / 'firmware/src/dsp/break_retrigger_lut.h'
SOURCE = ROOT / 'tests/host/break_retrigger_lut_test.c'

class ProbabilityLaw(unittest.TestCase):
    def build(self, folder, header=None, generate=False):
        source = SOURCE.read_text().replace('break_retrigger_reference.inc', str(ROOT / 'tests/host/break_retrigger_reference.inc'))
        source = source.replace('../../firmware/src/dsp/fastmath.h', str(ROOT / 'firmware/src/dsp/fastmath.h'))
        if header is not None:
            path = folder / 'table.h'
            path.write_text(header)
            source = source.replace('../../firmware/src/dsp/break_retrigger_lut.h', str(path))
        path = folder / 'test.c'
        path.write_text(source)
        binary = folder / 'test'
        cmd = [os.environ.get('CC', 'cc'), '-O1', '-std=c99', '-ffp-contract=off', '-I'+str(ROOT / 'firmware/src/dsp')]
        if generate:
            cmd += ['-DGENERATE']
        subprocess.run(cmd + [str(path), '-lm', '-o', str(binary)], check=True, capture_output=True, timeout=30)
        return subprocess.run([str(binary)], capture_output=True, timeout=10)

    def test_generator_and_original_binary32_law(self):
        with tempfile.TemporaryDirectory() as d:
            folder = Path(d)
            generated = self.build(folder, generate=True)
            self.assertEqual(generated.returncode, 0)
            self.assertEqual(generated.stdout, (ROOT / 'tests/host/break_retrigger_reference.inc').read_bytes())
            result = self.build(folder, HEADER.read_text())
            self.assertEqual(result.returncode, 0)
            self.assertIn(b'1536 bit-exact comparisons PASS', result.stdout)

    def test_wrong_slice_ticks_rejected_at_runtime(self):
        source = HEADER.read_text()
        self.assertIn('{3,6,12,24,48,96}', source)
        with tempfile.TemporaryDirectory() as d:
            result = self.build(Path(d), source.replace('{3,6,12,24,48,96}', '{4,6,12,24,48,96}', 1))
            self.assertEqual(result.returncode, 1)
            self.assertIn(b'PROBABILITY_BITS_MISMATCH',result.stderr)

    def test_wrong_probability_scale_rejected_at_runtime(self):
        source = HEADER.read_text()
        self.assertEqual(source.count('0.01f'), 1)
        with tempfile.TemporaryDirectory() as d:
            result = self.build(Path(d), source.replace('0.01f', '0.02f'))
            self.assertEqual(result.returncode, 1)
            self.assertIn(b'PROBABILITY_BITS_MISMATCH',result.stderr)

if __name__ == '__main__':
    unittest.main()
