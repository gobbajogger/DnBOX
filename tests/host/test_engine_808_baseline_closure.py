#!/usr/bin/env python3
"""Real Git/compiler regression for baseline textual include isolation; no DSP claim."""
import pathlib, subprocess, tempfile, unittest

SCRIPT = pathlib.Path(__file__).with_name('engine_808_scratch_alias_test.sh')
PREFIX = SCRIPT.read_text().split("U=$(python3", 1)[0]

class BaselineClosure(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = pathlib.Path(self.temp.name)
        (self.root/'tests/host').mkdir(parents=True)
        self.app = self.root/'firmware/src/app'
        self.app.mkdir(parents=True)
        self.git('init', '-q')
        self.git('config', 'user.name', 'fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
    def git(self, *args):
        return subprocess.check_output(['git', '-C', str(self.root), *args], stderr=subprocess.DEVNULL).decode().strip()
    def commit(self):
        self.git('add', '.')
        self.git('commit', '-qm', 'fixture')
        return self.git('rev-parse', 'HEAD')
    def setup_versions(self, include=True):
        (self.app/'engine.c').write_text('static int old_owner=7;\n#include "original_presets.inc"\nint main(void){return result()!=7;}\n')
        if include: (self.app/'original_presets.inc').write_text('static int result(void){return old_owner;}\n')
        base=self.commit()
        (self.app/'engine.c').write_text('static int new_owner=9;\n#include "original_presets.inc"\nint main(void){return result()!=9;}\n')
        (self.app/'original_presets.inc').write_text('static int result(void){return new_owner;}\n')
        self.commit()
        return base
    def stage(self, base):
        p=self.root/'tests/host/prefix.sh';p.write_text(PREFIX)
        import os
        return subprocess.run(['sh',str(p)], env={**os.environ,'BASE_REF':base}, capture_output=True)
    def compile(self, path, name):
        return subprocess.run(['cc','-Werror','-I'+str(self.app),str(path),'-o',str(self.root/name)],capture_output=True)
    def test_matching_historical_include_and_current_remain_separate(self):
        base=self.setup_versions(); self.assertEqual(self.stage(base).returncode,0)
        d=self.root/'build/host/engine-808-alias'
        self.assertEqual((d/'original_presets.inc').read_bytes(),self.git('show',base+':firmware/src/app/original_presets.inc').encode()+b'\n')
        for p,n in [(d/'engine-base.c','old'),(self.app/'engine.c','new')]:
            c=self.compile(p,n);self.assertEqual(c.returncode,0,c.stderr.decode());self.assertEqual(subprocess.run([str(self.root/n)]).returncode,0)
    def test_old_engine_with_current_include_reproduces_compile_failure(self):
        self.assertEqual(self.stage(self.setup_versions()).returncode,0)
        d=self.root/'build/host/engine-808-alias';(d/'original_presets.inc').unlink()
        c=self.compile(d/'engine-base.c','bad');self.assertNotEqual(c.returncode,0);self.assertIn(b'new_owner',c.stderr)
    def test_missing_historical_include_fails_instead_of_current_fallback(self):
        self.assertNotEqual(self.stage(self.setup_versions(False)).returncode,0)

class BaselineFallbacks(unittest.TestCase):
    def test_unavailable_baseline_calls_abort_and_strong_current_calls_override(self):
        text = SCRIPT.with_name('engine_output_window_test.c').read_text()
        block = text[text.index('#pragma weak engine_queue_available_locked'):text.index('/* CPU2 modes run:')]
        pre = '#include <stdlib.h>\ntypedef struct {int unused;} lane_t; typedef struct {int unused;} sound_t;\n'
        calls = ['engine_queue_available_locked()', 'engine_set_from_remote(0,0,0,0)', 'engine_bass_note_capture(0,0,0,0,0)']
        strong = ['unsigned engine_queue_available_locked(void){return 7;}',
                  'void engine_set_from_remote(int t,int v,int p,int value){}',
                  'int engine_bass_note_capture(lane_t*l,int p,int v,int s,const sound_t*x){return 1;}']
        with tempfile.TemporaryDirectory() as td:
            r=pathlib.Path(td); (r/'weak.c').write_text(pre+block)
            for n,call in enumerate(calls):
                (r/'main.c').write_text(pre+block+'int main(void){'+call+';return 0;}')
                subprocess.run(['cc',str(r/'main.c'),'-o',str(r/'test')],check=True)
                result=subprocess.run([str(r/'test')],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
                self.assertEqual(result.returncode,-6)
                (r/'strong.c').write_text(pre+strong[n])
                subprocess.run(['cc',str(r/'main.c'),str(r/'strong.c'),'-o',str(r/'test')],check=True)
                self.assertEqual(subprocess.run([str(r/'test')]).returncode,0)

if __name__=='__main__': unittest.main()
