#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Compiler-input binding regression; compiler calls are deliberately observed stubs."""
from pathlib import Path
import contextlib, io, runpy, shutil, subprocess, sys, tempfile, unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
class BindingTest(unittest.TestCase):
    def exercise(self, replacement=None):
        with tempfile.TemporaryDirectory(prefix='dnbox-binding-test-') as temp:
            root=Path(temp);tests=root/'tests';live=tests/'host/dnr2_reference';live.mkdir(parents=True)
            names=['remote_core.c','remote_core.h','remote_wire.c','remote_wire.h']
            originals={n:(ROOT/'tests/host/dnr2_reference'/n).read_bytes() for n in names}
            for name,data in originals.items():(live/name).write_bytes(data)
            harness=(ROOT/'tests/host/remote_native_annotations_test.c').read_bytes()
            live_fixture=tests/'host/remote_native_annotations_test.c';live_fixture.write_bytes(harness)
            script=(ROOT/'tests/remote_native_annotations_test.py').read_text()
            if replacement:
                old,new=replacement;self.assertEqual(script.count(old),1);script=script.replace(old,new)
            runner=tests/'remote_native_annotations_test.py';runner.write_text(script)
            calls=[]
            def observe(command, **kwargs):
                if not calls:
                    for name in names:(live/name).write_bytes(originals[name]+b'\n/* after capture */\n')
                    live_fixture.write_bytes(harness+b'\n/* after capture */\n')
                calls.append(command)
                for arg in command:
                    if arg.startswith('-I'):
                        directory=Path(arg[2:]);self.assertNotEqual(directory,live,'live compiler include')
                        for name in ('remote_core.h','remote_wire.h'):self.assertEqual((directory/name).read_bytes(),originals[name])
                    elif arg.endswith('.c'):
                        source=Path(arg);self.assertFalse(source==live_fixture or source.parent==live,'live compiler input')
                        data=source.read_bytes()
                        expected=harness if source.name=='differential.c' else originals[source.name]
                        self.assertEqual(data.replace(b'__attribute__((noinline)) ',b''),expected)
                return subprocess.CompletedProcess(command,0)
            with patch.object(sys,'argv',[str(runner),'--mode','normal']), patch('subprocess.run',observe), contextlib.redirect_stdout(io.StringIO()):
                runpy.run_path(str(runner),run_name='__main__')
            self.assertEqual(len(calls),4)
            self.assertNotEqual((live/'remote_core.c').read_bytes(),originals['remote_core.c'])
    def test_captured_project_bytes_survive_live_mutation(self):self.exercise()
    def test_live_original_regression_rejected(self):
        with self.assertRaisesRegex(AssertionError,'live compiler input'):self.exercise(('str(original_dir/name)','str(SRC/name)'))
    def test_live_header_regression_rejected(self):
        with self.assertRaisesRegex(AssertionError,'live compiler include'):self.exercise(("'-I'+str(original_dir)","'-I'+str(SRC)"))
    def test_live_fixture_regression_rejected(self):
        with self.assertRaisesRegex(AssertionError,'live compiler input'):self.exercise(('str(fixture)',"str(ROOT/'tests/host/remote_native_annotations_test.c')"))
if __name__=='__main__':unittest.main()
