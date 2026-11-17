"""Control-boundary tests; mocks are not a real LeakSanitizer qualification."""
import importlib.util
import contextlib
import io
import types
import os
from pathlib import Path
import subprocess
import unittest
from unittest.mock import patch

SPEC = importlib.util.spec_from_file_location('compact_programs', Path(__file__).with_name('program_values_compact_test.py'))
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)
REPORT = 'ERROR: LeakSanitizer: detected memory leaks\nDirect leak of 123 byte(s) in 1 object(s) allocated from:\nSUMMARY: AddressSanitizer: 123 byte(s) leaked in 1 allocation(s).\n'

class SanitizerControl(unittest.TestCase):
    def test_inherited_disabling_options_are_overridden(self):
        with patch.dict(os.environ, {'ASAN_OPTIONS':'detect_leaks=0:exitcode=0:allocator_may_return_null=1','LSAN_OPTIONS':'detect_leaks=0:exitcode=0:suppressions=/fixture/suppressions','UBSAN_OPTIONS':'halt_on_error=0'}, clear=True):
            env = MODULE.sanitizer_environment()
        for key in ('ASAN_OPTIONS', 'LSAN_OPTIONS'):
            pairs = dict(x.split('=',1) for x in env[key].split(':'))
            self.assertEqual(pairs['detect_leaks'], '1')
            self.assertEqual(pairs['exitcode'], '23')
        self.assertNotIn('allocator_may_return_null', env['ASAN_OPTIONS'])
        self.assertNotIn('suppressions', env['LSAN_OPTIONS'])
        self.assertIn('halt_on_error=1', env['UBSAN_OPTIONS'])

    def test_control_needs_exit_and_real_expected_report(self):
        MODULE.require_leak_control(subprocess.CompletedProcess([], 23, '', REPORT))
        for code, text in [(0,REPORT),(23,''),(23,'LeakSanitizer fatal error: ptrace'),(23,REPORT.replace('123','124')),(1,REPORT)]:
            with self.subTest(code=code,text=text):
                with self.assertRaises(RuntimeError):
                    MODULE.require_leak_control(subprocess.CompletedProcess([], code, '', text))

    def test_sanitized_runs_receive_forced_environment(self):
        with patch.object(MODULE.subprocess,'run',return_value=subprocess.CompletedProcess([],0)) as run:
            MODULE.run_sanitized(Path('/fixture/test'))
        self.assertTrue(run.call_args.kwargs['check'])
        self.assertIn('detect_leaks=1',run.call_args.kwargs['env']['ASAN_OPTIONS'])
        self.assertIn('exitcode=23',run.call_args.kwargs['env']['LSAN_OPTIONS'])

    def test_main_wires_real_control_and_sanitized_program(self):
        calls = []
        def fake_run(command, **kwargs):
            calls.append((command, kwargs))
            if len(command) > 1 and command[1].endswith('gen_program_values_compact.py'):
                Path(command[2]).write_bytes((MODULE.ROOT/'experiments/original_synth/program_values_compact.h').read_bytes())
            name = Path(command[0]).name
            if name == 'leak-control':
                self.assertIn('detect_leaks=1', kwargs['env']['ASAN_OPTIONS'])
                return subprocess.CompletedProcess(command,23,'',REPORT)
            return subprocess.CompletedProcess(command, 1 if name == 'mutant' else 0, '', '')
        with patch.object(MODULE.subprocess, 'run', side_effect=fake_run), contextlib.redirect_stdout(io.StringIO()):
            MODULE.main()
        test_runs = [(c,k) for c,k in calls if Path(c[0]).name == 'test']
        self.assertEqual(len(test_runs), 2)
        self.assertTrue(test_runs[1][1]['check'])
        self.assertIn('detect_leaks=1', test_runs[1][1].get('env', {}).get('ASAN_OPTIONS', ''))
        leak_index = next(i for i,(c,k) in enumerate(calls) if Path(c[0]).name == 'leak-control')
        program_index = next(i for i,(c,k) in enumerate(calls) if Path(c[0]).name == 'test')
        self.assertLess(leak_index, program_index)

    def test_main_stops_when_leak_control_is_not_genuine(self):
        calls = []
        def fake_run(command, **kwargs):
            calls.append(command)
            if len(command) > 1 and command[1].endswith('gen_program_values_compact.py'):
                Path(command[2]).write_bytes((MODULE.ROOT/'experiments/original_synth/program_values_compact.h').read_bytes())
            return subprocess.CompletedProcess(command, 23 if Path(command[0]).name == 'leak-control' else 0, '', 'ptrace failure')
        with patch.object(MODULE.subprocess, 'run', side_effect=fake_run), contextlib.redirect_stdout(io.StringIO()):
            with self.assertRaisesRegex(RuntimeError, 'LeakSanitizer control failed'):
                MODULE.main()
        self.assertFalse(any(Path(c[0]).name == 'test' for c in calls))

    def test_unsafe_mutations_are_rejected(self):
        global MODULE
        original = MODULE
        source = Path(SPEC.origin).read_text()
        mutations = [
            ('        require_leak_control(control)', '        pass', 'test_main_stops_when_leak_control_is_not_genuine'),
            ("detect_leaks=1", "detect_leaks=0", 'test_inherited_disabling_options_are_overridden'),
            ("exitcode=23", "exitcode=0", 'test_inherited_disabling_options_are_overridden'),
            ("result.returncode != 23", "False", 'test_control_needs_exit_and_real_expected_report'),
            ("or not re.search(r'123 byte", "or not re.search(r'[0-9]+ byte", 'test_control_needs_exit_and_real_expected_report'),
            ("run_sanitized(exe)\n", "subprocess.run([str(exe)], check=True)\n", 'test_main_wires_real_control_and_sanitized_program'),
        ]
        for before,after,method in mutations:
            with self.subTest(mutation=before):
                self.assertIn(before, source)
                mutant = types.ModuleType('compact_mutant')
                mutant.__file__ = SPEC.origin
                exec(compile(source.replace(before, after), SPEC.origin, 'exec'), mutant.__dict__)
                MODULE = mutant
                try:
                    result = unittest.TextTestRunner(stream=io.StringIO()).run(unittest.TestSuite([SanitizerControl(method)]))
                    self.assertFalse(result.wasSuccessful(), 'unsafe mutation escaped')
                    self.assertFalse(result.errors, 'negative control must fail assertions')
                    self.assertTrue(result.failures)
                finally:
                    MODULE = original

if __name__ == '__main__':
    unittest.main()
