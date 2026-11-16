#!/usr/bin/env python3
import shutil,subprocess,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
class Authentication(unittest.TestCase):
 def check_tamper(self,kind):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);dest=root/'tests/host/reference';dest.mkdir(parents=True)
   shutil.copy(ROOT/'tests/host/composed_ui_identity_test.sh',dest.parent)
   for p in (ROOT/'tests/host/reference').glob('ui_r22_d13_header_reference.*'):shutil.copy(p,dest/p.name)
   p=dest/('ui_r22_d13_header_reference.inc' if kind=='oracle' else 'ui_r22_d13_header_reference.json')
   text=p.read_text()
   if kind=='oracle':text+='\n'
   elif kind=='receipt':text=text.replace('d13f410936596b6931e574e6418e35ea67af3368','0'*40)
   elif kind=='source':text=text.replace('01f0b4b4d2d099fef2c93fa6190d6bf2f25b4cd179495118dbbfd0a6c32cabbf','0'*64)
   else:text=text.replace('0f9b0c750b99cdbe5816216ad4d077db80b8bb6fb0b7131645d524dc31e46b2d','0'*64)
   p.write_text(text)
   result=subprocess.run(['sh','tests/host/composed_ui_identity_test.sh'],cwd=root,capture_output=True,text=True)
   self.assertNotEqual(result.returncode,0);self.assertIn('reference authentication failed',result.stderr)
 def test_oracle(self):self.check_tamper('oracle')
 def test_receipt(self):self.check_tamper('receipt')
 def test_source_metadata(self):self.check_tamper('source')
 def test_function_metadata(self):self.check_tamper('function')
if __name__=='__main__':unittest.main()
