#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Protect the distinction between noisy ratios and a missed absolute deadline."""
import csv
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


class TimingRepetitions(unittest.TestCase):
    def check_pairs(self, outlier_ns):
        with tempfile.TemporaryDirectory() as td:
            results = []
            for repeat, peak in enumerate((outlier_ns, 900_000, 925_000, 950_000)):
                path = Path(td) / f'{repeat}.csv'
                with path.open('w', newline='') as stream:
                    writer = csv.writer(stream)
                    writer.writerow(('cpu_ns', 'chicago_cpu_ns', 'miami_cpu_ns'))
                    for block in range(256):
                        writer.writerow((peak if block == 100 else 800_000, 45_000, 60_000))
                results.append(f'0,1:{path}')
            return subprocess.run(
                [sys.executable, str(ROOT/'tools/render_timing_check.py'),
                 '--baseline', str(ROOT/'tests/host/render_timing_baseline.json'), *results],
                capture_output=True, text=True, check=False)

    def test_noisy_ratio_uses_repeated_measurements(self):
        result = self.check_pairs(3_000_000)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('0 late blocks', result.stdout)

    def test_deadline_miss_fails_even_in_a_nonwinning_run(self):
        result = self.check_pairs(4_000_000)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('1 late blocks over 3869.7 us', result.stdout)
        self.assertNotIn('REGRESSED', result.stdout)


if __name__ == '__main__':
    unittest.main()
