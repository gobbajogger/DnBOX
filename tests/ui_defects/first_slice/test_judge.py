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




class JudgeScreenWalkTest(unittest.TestCase):
    """The full host screen walk is part of the gate: a walker CHECK failure (for
    example a page-count assumption at ui_screen_walk.c) must FAIL ui-defects instead
    of silently stopping after part of the inventory, as it did at 73749f55."""

    def test_walker_failure_fails_the_gate(self):
        def command(args, **kwargs):
            if args[:2] == ['git', 'archive']:
                return subprocess.CompletedProcess(args, 0, stdout=b'')
            if args[:2] == ['tar', '-x']:
                work = Path(args[3])
                (work / 'firmware/src/app').mkdir(parents=True)
                (work / 'firmware/src/app/ui_drum_voice.inc').write_text('')
                return subprocess.CompletedProcess(args, 0)
            self.fail('no renderer may run after the walk failed: ' + repr(args))

        def walk(work, out):
            raise ValueError('host screen walker failed: exit 1')

        def geometry(work, out):
            self.fail('geometry must not run after the walk failed')

        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / 'judged'
            with patch.object(sys, 'argv', ['judge.py', '--output', str(out)]), \
                 patch.object(judge.subprocess, 'check_output', return_value=str(Path(tmp))), \
                 patch.object(judge.subprocess, 'run', side_effect=command), \
                 patch.object(judge, 'screen_walk', side_effect=walk), \
                 patch.object(judge, 'full_geometry', side_effect=geometry), \
                 contextlib.redirect_stdout(io.StringIO()):
                result = judge.main()
            self.assertEqual(result, 1)
            status = json.loads((out / 'judgement.json').read_text())
            self.assertEqual(status['status'], 'FAIL')
            self.assertIn('screen walker', status['error'])

    def test_gate_judge_calls_the_walker(self):
        src = Path(judge.__file__).read_text()
        self.assertIn('screen_walk(work, out / \'screen-walk\')', src)


if __name__ == '__main__':
    unittest.main()
