#!/usr/bin/env python3
import importlib.util,json,subprocess,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('roadmap',ROOT/'tools/generate_roadmap.py'); roadmap=importlib.util.module_from_spec(spec);spec.loader.exec_module(roadmap)
def item(status='working',sha=''):
 return dict(id='L21',title='Truthful roadmap',move='M5',status=status,merge_sha=sha,owner='integ',dependencies=[],next_action='Run checks')
def state(items): return '# Source state\n\n```json\n'+json.dumps({'items':items})+'\n```\n'
class Tests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name)
  self.git('init','-q');self.git('config','user.email','test@example.invalid');self.git('config','user.name','Test')
  self.git('commit','--allow-empty','-qm','base');self.base=self.git('rev-parse','HEAD')
  self.git('commit','--allow-empty','-qm','unmerged');self.unmerged=self.git('rev-parse','HEAD')
 def tearDown(self): self.tmp.cleanup()
 def git(self,*args): return subprocess.check_output(['git','-C',str(self.root),*args],text=True).strip()
 def test_ancestry(self):
  roadmap.verify_merges([item('merged',self.base)],self.root,self.base)
  for sha in ['0'*40,self.unmerged]:
   with self.assertRaises(ValueError):roadmap.verify_merges([item('merged',sha)],self.root,self.base)
 def test_shallow_positive_proof(self):
  (self.root/'.git/shallow').write_text(self.base+'\n')
  roadmap.verify_merges([item('merged',self.base)],self.root,self.unmerged)
  with self.assertRaises(ValueError):
   roadmap.verify_merges([item('merged',self.unmerged)],self.root,self.base)
 def test_requires_full_integration_sha(self):
  for sha in ['',None,'HEAD',self.base[:8],'0'*40]:
   with self.assertRaises(ValueError):roadmap.verify_merges([],self.root,sha)
 def test_malformed(self):
  for text in ['',state([item(),item()]),state([item('complete')]),state([item('merged')])]:
   with self.assertRaises(ValueError):roadmap.read_items(text)
 def test_generate_check(self):
  (self.root/'CONSOLIDATED-STATE.md').write_text(state([item('merged',self.base)]))
  cmd=[sys.executable,str(ROOT/'tools/generate_roadmap.py'),'--root',str(self.root),'--integration-sha',self.base]
  subprocess.run(cmd,check=True);subprocess.run(cmd+['--check'],check=True)
  (self.root/'docs/roadmap/Complete.md').write_text('false')
  self.assertNotEqual(subprocess.run(cmd+['--check'],capture_output=True).returncode,0)
 def test_invalid_merge_does_not_write(self):
  (self.root/'CONSOLIDATED-STATE.md').write_text(state([item('merged',self.unmerged)]))
  cmd=[sys.executable,str(ROOT/'tools/generate_roadmap.py'),'--root',str(self.root),'--integration-sha',self.base]
  self.assertNotEqual(subprocess.run(cmd,capture_output=True).returncode,0)
  self.assertFalse((self.root/'docs/roadmap').exists())
 def test_escape(self):
  row=item();row['title']='one|two\nthree';self.assertIn('one\\|two three',roadmap.render([row])['Working.md'])
if __name__=='__main__':unittest.main()
