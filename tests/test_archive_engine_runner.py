#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Actual shell helper must not turn a failed audio process into parity success."""
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parent / 'host/engine_808_scratch_alias_test.sh'

class AudioExitContract(unittest.TestCase):
    def helper(self):
        source = SCRIPT.read_text()
        matches = re.findall(r'^run_audio_checked\(\) \{\n.*?^\}', source, re.M | re.S)
        self.assertEqual(len(matches), 1)
        self.assertEqual(source.count('run_audio_checked "$D/'), 2,
                         'both baseline/current and partial runs must use checked helper')
        self.assertNotRegex(source, r'ASAN_OPTIONS=.*\$D/h-.*\|',
                            'audio status must not be hidden behind a pipeline')
        return matches[0]

    def execute(self, helper, code, marker=True, payload="partial"):
        with tempfile.TemporaryDirectory(prefix='archive-audio-exit-') as d:
            d = Path(d)
            audio = d / 'audio.sh'
            audio.write_text(('printf ' + payload + ' > "$1"\n' if payload is not None else '') + ('echo "PASS engine output windows: 600 blocks;"\n' if marker else '') + 'exit ' + str(code) + '\n')
            script = helper + '\nset -e\nrun_audio_checked "$1/log" "$1/pcm" 600 7 sh "$1/audio.sh"\necho PARITY_CONTINUED\n'
            result = subprocess.run(['sh', '-c', script, 'test', str(d)],
                                    capture_output=True, text=True)
            return result, (d / 'pcm').read_bytes() if (d / 'pcm').exists() else None

    def assert_failure_preserved(self, helper):
        result, pcm = self.execute(helper, 23)
        self.assertEqual(pcm, b'partial')
        self.assertEqual(result.returncode, 23, 'audio failure exit was swallowed')
        self.assertNotIn('PARITY_CONTINUED', result.stdout)

    def test_failed_audio_stops_before_comparison(self):
        self.assert_failure_preserved(self.helper())

    def test_success_continues(self):
        result, _ = self.execute(self.helper(), 0)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('PARITY_CONTINUED', result.stdout)

    def test_missing_marker_stops(self):
        result, _ = self.execute(self.helper(), 0, marker=False)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("missing complete audio marker", result.stderr)
        self.assertNotIn("PARITY_CONTINUED", result.stdout)

    def test_missing_pcm_stops(self):
        result, pcm = self.execute(self.helper(), 0, payload=None)
        self.assertIsNone(pcm)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("missing readable audio bytes", result.stderr)
        self.assertNotIn("PARITY_CONTINUED", result.stdout)

    def test_truncated_success_stops(self):
        result, _ = self.execute(self.helper(), 0, payload="tiny")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("incomplete audio bytes", result.stderr)
        self.assertNotIn("PARITY_CONTINUED", result.stdout)

    def test_early_pcm_mismatch_stops_all_later_iterations(self):
        source = SCRIPT.read_text()
        blocks = re.findall(r'^ (cmp [^\n]+)\n (echo [^\n]+)', source, re.M)
        self.assertEqual(len(blocks), 2, 'both PCM comparisons must stand alone')
        for compare, echo in blocks:
            self.assertNotIn('&&', compare, 'cmp must propagate failure')
            with self.subTest(compare=compare), tempfile.TemporaryDirectory(prefix='archive-pcm-compare-') as d:
                d = Path(d)
                for mode in ('normal', 'san'):
                    for suffix in ('base', 'new', 'm3', 'm1'):
                        (d / (mode + '-1-' + suffix + '.pcm')).write_bytes(b'1234567')
                suffix = 'new' if '-base.pcm' in compare else 'm1'
                (d / ('normal-1-' + suffix + '.pcm')).write_bytes(b'7654321')
                script = 'set -e\nD=$1; dw=1\nfor mode in normal san; do\n' + compare + '\n' + echo + '\ndone\necho ALL_CONTINUED\n'
                result = subprocess.run(['sh', '-c', script, 'test', str(d)], capture_output=True, text=True)
                self.assertNotEqual(result.returncode, 0, 'early PCM mismatch was swallowed')
                self.assertNotIn('ALL_CONTINUED', result.stdout)
                self.assertNotIn('BITEXACT', result.stdout)
                self.assertNotIn('PARTIAL=CLEAN', result.stdout)

    def test_swallowed_exit_mutant_is_rejected(self):
        helper = self.helper()
        self.assertIn('return "$rc"', helper)
        with self.assertRaisesRegex(AssertionError, 'audio failure exit was swallowed'):
            self.assert_failure_preserved(helper.replace('return "$rc"', 'return 0'))

if __name__ == '__main__':
    unittest.main()
