#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""The gate cannot pass a source tree whose required renderer is absent."""
import contextlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import judge


class JudgeAdmissionTest(unittest.TestCase):
    def test_missing_required_screen_fails_without_claiming_render(self):
        calls = []

        def command(args, **kwargs):
            calls.append(args)
            if args[:2] == ['git', 'archive']:
                return subprocess.CompletedProcess(args, 0, stdout=b'')
            if args[:2] == ['tar', '-x']:
                # Model an otherwise valid exported source tree without the
                # required R22 screen. Do not substitute a fake rendered trace.
                return subprocess.CompletedProcess(args, 0)
            self.fail('renderer must not run on missing source: ' + repr(args))

        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / 'judged'
            with patch.object(sys, 'argv', ['judge.py', '--output', str(out)]), \
                 patch.object(judge.subprocess, 'check_output', return_value=str(Path(tmp))), \
                 patch.object(judge.subprocess, 'run', side_effect=command), \
                 contextlib.redirect_stdout(io.StringIO()):
                result = judge.main()
            self.assertEqual(result, 1, 'missing required renderer must fail the gate')
            status = json.loads((out / 'judgement.json').read_text())
            self.assertEqual(status['status'], 'FAIL')
            self.assertEqual(status['screens'], 0)
            self.assertIn('ui_drum_voice.inc', status['reason'])
            self.assertEqual(len(calls), 2)


if __name__ == '__main__':
    unittest.main()
