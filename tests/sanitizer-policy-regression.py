#!/usr/bin/env python3
"""Static control and real-shell exit propagation tests, not sanitizer execution."""
import importlib.util, pathlib, subprocess, tempfile, unittest
ROOT = pathlib.Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('policy', ROOT/'tools/dnbox-sanitizer-policy.py')
policy = importlib.util.module_from_spec(spec); spec.loader.exec_module(policy)
class Controls(unittest.TestCase):
    def check(self, source='echo test', env=None):
        with tempfile.TemporaryDirectory() as tmp:
            root=pathlib.Path(tmp); (root/'tests/host').mkdir(parents=True); (root/'tools/mboot').mkdir(parents=True); (root/'tests/run_tests.sh').write_text('echo runner')
            (root/'tests/host/fixture.sh').write_text(source)
            return policy.inspect(root, env or {})[1]
    def test_enabled(self): self.assertFalse(self.check('ASAN_OPTIONS=detect_leaks=1 test'))
    def test_source_disabled(self): self.assertTrue(self.check('ASAN_OPTIONS=detect_leaks=0 test'))
    def test_env_disabled(self): self.assertTrue(self.check(env={'ASAN_OPTIONS':'detect_leaks=0'}))
    def test_env_suppression(self): self.assertTrue(self.check(env={'LSAN_OPTIONS':'suppressions=fixture.txt'}))
    def test_env_failure_hidden(self): self.assertTrue(self.check(env={'LSAN_OPTIONS':'exitcode=0'}))
    def test_disable_api(self): self.assertTrue(self.check('__lsan_disable();'))
    def test_exit_check_disabled(self): self.assertTrue(self.check('ASAN_OPTIONS=leak_check_at_exit=0 test'))
    def test_ignore_api(self): self.assertTrue(self.check('__lsan_ignore_object(p);'))
    def test_source_suppression(self): self.assertTrue(self.check('LSAN_OPTIONS=suppressions=fixture.txt test'))
    def test_source_zero_exit(self): self.assertTrue(self.check('LSAN_OPTIONS=exitcode=0 test'))
    def test_missing_required_paths(self):
        import shutil
        for missing in ('tests/run_tests.sh','tests/host','tools/mboot'):
            with tempfile.TemporaryDirectory() as tmp:
                root=pathlib.Path(tmp);(root/'tests/host').mkdir(parents=True);(root/'tools/mboot').mkdir(parents=True);(root/'tests/run_tests.sh').write_text('echo runner');(root/'tests/other.py').write_text('pass')
                p=root/missing
                if p.is_dir():shutil.rmtree(p)
                else:p.unlink()
                self.assertTrue(policy.inspect(root,{})[1],missing)
    def test_empty(self):
        with tempfile.TemporaryDirectory() as tmp: self.assertTrue(policy.inspect(pathlib.Path(tmp),{})[1])
class Pipeline(unittest.TestCase):
    def test_actual_script_propagates_both_executable_failures(self):
        source=(ROOT/'tests/host/engine_808_scratch_alias_test.sh').read_text()
        lines=[line.strip() for line in source.splitlines() if 'if ASAN_OPTIONS=detect_leaks=1' in line]
        self.assertEqual(len(lines),2)
        with tempfile.TemporaryDirectory() as tmp:
            root=pathlib.Path(tmp)
            for code in (0,7):
                for name in ('h-new','h-m3'):
                    p=root/name; p.write_text(f'#!/bin/sh\nprintf "%s\\n" "$ASAN_OPTIONS"\nexit {code}\n'); p.chmod(0o755)
                for line in lines:
                    r=subprocess.run(['sh','-c',f'D={tmp};mode=san;dw=1;v=new;r=3;'+line],capture_output=True,text=True)
                    self.assertEqual(r.returncode==0,code==0)
                    self.assertIn('detect_leaks=1',r.stdout)
    def test_old_pipeline_reproduces_false_success(self):
        r=subprocess.run(['sh','-c','set -e; (echo fixture; exit 7) | tail -1'],capture_output=True,text=True)
        self.assertEqual(r.returncode,0)
class GateGuard(unittest.TestCase):
    def test_actual_gate_blocks_missing_or_failed_policy(self):
        gate=(ROOT/'tools/dnbox-gate.sh').read_text(); tail=gate[gate.index('step clean-tree clean'):]
        prefix="""fail=0;void=0;SHA=fixture;LANE=integ;LOG=fixture
step() { shift; if "$@"; then :; else fail=1; fi; }
clean(){ :; }; setup(){ :; }; no_loss(){ :; }; limits(){ :; }
fw_build(){ :; };flashmap(){ :; };budget(){ :; };ui_defects(){ :; };drone(){ :; }
host_tests(){ touch host-ran; }
"""
        for outcome in (None,1,0):
            with tempfile.TemporaryDirectory() as tmp:
                r=pathlib.Path(tmp);(r/'tools').mkdir();(r/'tests').mkdir()
                (r/'tests/browser-sandbox-policy.py').write_text('raise SystemExit(0)')
                if outcome is not None:(r/'tools/dnbox-sanitizer-policy.py').write_text(f'raise SystemExit({outcome})')
                done=subprocess.run(['bash','-c',prefix+tail],cwd=r,capture_output=True,text=True)
                self.assertEqual((r/'host-ran').exists(),outcome==0)
                self.assertEqual(done.returncode==0,outcome==0)
if __name__ == '__main__': unittest.main()
