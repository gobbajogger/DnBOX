# SPDX-License-Identifier: GPL-3.0-only
import importlib.util,pathlib,unittest,tempfile,hashlib
R=pathlib.Path(__file__).resolve().parents[3]
def module(path,name):
 s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
runner=module(R/'tests/ui_defects/glyph_text/run.py','observer');bounds=module(R/'tests/ui_defects/text_bounds/run.py','bounds')
class Observer(unittest.TestCase):
 def setUp(self):self.raw=[(R/p).read_text() for p in ['firmware/src/gfx.c','firmware/src/app/ui.c','firmware/src/app/ui_drum_voice.inc']]
 def test_instrumentation(self):
  g,u,d=runner.instrument(*self.raw,bounds);self.assertIn('fwui_glyph_after',g);self.assertIn('dv_glyph_original(r,ev,ox,oy,sx,sy,col);',d);self.assertIn('fwui_text_commit_origin(x,y)',u)
 def test_duplicate_anchors_rejected(self):
  for index,anchor in [(0,'static void cv_begin(uint32_t w, uint32_t h, uint16_t bg)\n{'),(0,'                a = (gx & 1u) ? (a & 15u) : (a >> 4);'),(2,'static UI_DRAW void dv_glyph('),(2,'static UI_DRAW void dv_value_text(')]:
   with self.subTest(anchor=anchor):
    raw=self.raw.copy();raw[index]+='\n'+anchor
    with self.assertRaises(ValueError):runner.instrument(*raw,bounds)
 def test_missing_anchors_rejected(self):
  raw=self.raw.copy();raw[2]=raw[2].replace('static UI_DRAW void dv_value_text(','static UI_DRAW void missing(')
  with self.assertRaises(ValueError):runner.instrument(*raw,bounds)
 def test_captured_helper_ignores_later_mutation(self):
  with tempfile.TemporaryDirectory()as directory:
   p=pathlib.Path(directory)/'helper.py';p.write_bytes(b'value=1\n');code=p.read_bytes();pin=hashlib.sha256(code).hexdigest();p.write_bytes(b'value=2\n')
   self.assertEqual(runner.load_captured(code,pin,p).value,1)
   with self.assertRaises(ValueError):runner.load_captured(p.read_bytes(),pin,p)
if __name__=='__main__':unittest.main()
