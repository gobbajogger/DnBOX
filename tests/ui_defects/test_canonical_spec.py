# SPDX-License-Identifier: GPL-3.0-only
"""Spec identity tests only, never per-page visual acceptance."""
import hashlib,importlib.util,json,os,shutil,subprocess,sys,tempfile,unittest
from pathlib import Path
from PIL import Image
MODULE=Path(__file__).with_name('canonical_spec.py').resolve()
s=importlib.util.spec_from_file_location('canonical',MODULE)
c=importlib.util.module_from_spec(s);s.loader.exec_module(c)
REL='docs/ui/UI-SOURCE-OF-TRUTH.md'
OWNER=(Path(__file__).resolve().parents[2]/REL).read_bytes()
PIN='868957f911acd3d2de67f61db41f2f49a552d0f023d69be67df96a637149add6'
class CanonicalTests(unittest.TestCase):
 def fixture(self,p):
  q=p/REL;q.parent.mkdir(parents=True);q.write_bytes(OWNER)
  return {'path':REL,'sha256':PIN,'bytes':2891}
 def test_document_drift_rejected(self):
  with tempfile.TemporaryDirectory() as t:
   p=Path(t);d=self.fixture(p);self.assertEqual(c.authenticate(p,d),PIN)
   (p/REL).write_text('changed text\n')
   with self.assertRaises(ValueError):c.authenticate(p,d)
 def test_outside_repository_rejected(self):
  with tempfile.TemporaryDirectory() as t:
   with self.assertRaises(ValueError):c.authenticate(Path(t),{'path':'../outside.md','sha256':'a'*64,'bytes':0})
 def test_missing_or_wrong_report_binding_blocks(self):
  self.assertTrue(c.report_binding({},'a'*64))
  self.assertTrue(c.report_binding({'canonicalSpecSHA256':'b'*64},'a'*64))
  self.assertEqual(c.report_binding({'canonicalSpecSHA256':'a'*64},'a'*64),[])
 def test_alias_copy_is_not_canonical_path(self):
  with tempfile.TemporaryDirectory() as t:
   p=Path(t);d=self.fixture(p);(p/'docs/owner-copy.md').write_bytes(OWNER)
   (p/REL).unlink();d['path']='docs/owner-copy.md'
   with self.assertRaises(ValueError):c.authenticate(p,d)
 def test_fifo_document_rejected_without_blocking(self):
  with tempfile.TemporaryDirectory() as t:
   p=Path(t);d=self.fixture(p);(p/REL).unlink();os.mkfifo(p/REL)
   code='import importlib.util,json,sys\nfrom pathlib import Path\ns=importlib.util.spec_from_file_location("canonical",'+repr(str(MODULE))+');c=importlib.util.module_from_spec(s);s.loader.exec_module(c)\nprint("READY",flush=True)\ntry:c.authenticate(Path('+repr(str(p))+'),json.loads('+repr(json.dumps(d))+'))\nexcept ValueError as e:print(str(e));sys.exit(0)\nsys.exit(1)'
   try:r=subprocess.run([sys.executable,'-c',code],capture_output=True,text=True,timeout=1)
   except subprocess.TimeoutExpired:self.fail('canonical FIFO read blocks instead of rejecting')
   self.assertEqual(r.returncode,0,r.stdout+r.stderr);self.assertIn('regular',r.stdout.lower())
 def test_full_cli_alias_cannot_replace_missing_or_drifted_owner_document(self):
  # Deliberately synthetic all-PASS reports calibrate only this CLI tripwire.
  # They are never a product run or visual-acceptance receipt.
  with tempfile.TemporaryDirectory() as t:
   root=Path(t)/'repo';code=root/'tests/ui_defects';code.mkdir(parents=True)
   for name in ['gate.py','canonical_spec.py']:shutil.copy2(MODULE.with_name(name),code/name)
   d=self.fixture(root);(code/'canonical-spec.json').write_text(json.dumps(d))
   evidence=root/'evidence';evidence.mkdir();png=evidence/'calibration.png';Image.new('RGB',(1,1)).save(png)
   h='a'*64;a={'path':str(png),'sha256':hashlib.sha256(png.read_bytes()).hexdigest(),'sourceClosureSha256':h,'kind':'synthetic-unit-calibration'}
   report={'sourceClosureSha256':h,'canonicalSpecSHA256':PIN,'revision':'1'*40,
    'checks':[{'id':i,'status':'PASS','evidence':[a]} for i in range(1,25)],
    'screens':[{**a,'id':'calibration','width':1,'height':1,'score':10,'defects':[]}],
    'coverageInventory':[{'id':'calibration'}],'coverageComplete':True,
    'run':{'exit':0,'firstAttempt':True,'sourceClosureSha256':h}}
   rp=evidence/'report.json';rp.write_text(json.dumps(report));receipt=evidence/'readback.json';receipt.write_text(json.dumps({'object':{'sha':'1'*40}}))
   args=[sys.executable,str(code/'gate.py'),'verify']
   for scope in ['firmware','web']:args+=['--'+scope+'-report',str(rp),'--'+scope+'-source',h,'--'+scope+'-pushed-receipt',str(receipt)]
   run=lambda more:subprocess.run(args+more,capture_output=True,text=True,timeout=3)
   baseline=run([]);self.assertEqual(baseline.returncode,0,baseline.stdout+baseline.stderr)
   (root/'docs/owner-copy.md').write_bytes(OWNER);d['path']='docs/owner-copy.md';alt=root/'alternate.json';alt.write_text(json.dumps(d))
   (root/REL).unlink();self.assertEqual(run(['--canonical-spec',str(alt)]).returncode,1)
   (root/REL).write_text('drifted');self.assertEqual(run(['--canonical-spec',str(alt)]).returncode,1)
   fifo=root/'descriptor-fifo';os.mkfifo(fifo)
   blocked=run(['--canonical-spec',str(fifo)]);self.assertEqual(blocked.returncode,1);self.assertIn('canonical',blocked.stdout.lower())
if __name__=='__main__':unittest.main()
