import json,tempfile,unittest
from pathlib import Path
from capture_source import stage_source
from run import closure

class CaptureSourceTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup)
  self.root=Path(self.tmp.name);self.src=self.root/'source';self.src.mkdir()
  (self.src/'sample.c').write_text('actual source bytes\n')
 def test_preserves_input_and_records_actual_bytes(self):
  (self.src/'SOURCE-PROFILE.json').write_text('{"historical":"preserved"}')
  before=closure(self.src);out=self.root/'receipt';work=stage_source(self.src,out)
  self.assertEqual(closure(self.src),before)
  self.assertEqual(json.loads((out/'input-source-closure.json').read_text()),before)
  self.assertEqual((out/'input-SOURCE-PROFILE.json').read_bytes(),(self.src/'SOURCE-PROFILE.json').read_bytes())
  self.assertEqual(json.loads((work/'SOURCE-PROFILE.json').read_text())['inputClosureSha256'],before['sha256'])
  self.assertEqual((work/'sample.c').read_bytes(),(self.src/'sample.c').read_bytes())
 def test_rejects_output_inside_input(self):
  with self.assertRaises(ValueError):stage_source(self.src,self.src/'output')
 def test_rejects_mismatched_expected_head(self):
  with self.assertRaises(ValueError):stage_source(self.src,self.root/'out','bad-head')
 def test_preserves_previous_output(self):
  out=self.root/'out';out.mkdir();(out/'important').write_text('keep')
  with self.assertRaises(FileExistsError):stage_source(self.src,out)
  self.assertEqual((out/'important').read_text(),'keep')
if __name__=='__main__':unittest.main()
