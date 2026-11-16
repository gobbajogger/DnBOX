# SPDX-License-Identifier: GPL-3.0-only
import json,pathlib,shutil,subprocess,sys,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[3]
class Pins(unittest.TestCase):
 def check_rejection(self,mutate,expected):
  with tempfile.TemporaryDirectory(prefix='contrast-negative-')as directory:
   root=pathlib.Path(directory);src=root/'source';src.mkdir()
   for name in ('firmware','host','tests','tools','assets','experiments'):shutil.copytree(ROOT/name,src/name,symlinks=True,ignore=shutil.ignore_patterns('__pycache__'))
   mutate(src);run=subprocess.run([sys.executable,str(src/'tests/ui_defects/contrast_triage/run.py'),'--source',str(src),'--output',str(root/'output')],capture_output=True,text=True)
   self.assertNotEqual(run.returncode,0);self.assertIn(expected,run.stdout+run.stderr);self.assertFalse((root/'output/ordinary.log').exists())
 def test_changed_source_rejected(self):
  def mutate(src):
   p=src/'firmware/src/app/ui.c';p.write_bytes(p.read_bytes()+b'\n')
  self.check_rejection(mutate,'Source differs from reviewed observer input')
 def test_bad_patch_rejected(self):
  def mutate(src):
   p=src/'tests/ui_defects/contrast_triage/observer.patch';p.write_text(p.read_text().replace('@@ ','@broken ',1))
  self.check_rejection(mutate,'CalledProcessError')
 def test_wrong_instrumented_hash_rejected(self):
  def mutate(src):
   p=src/'tests/ui_defects/contrast_triage/observer-inputs.json';r=json.loads(p.read_text());r['firmware/src/gfx.c']['afterSHA256']='0'*64;p.write_text(json.dumps(r))
  self.check_rejection(mutate,'Instrumented source drift')
if __name__=='__main__':unittest.main()
