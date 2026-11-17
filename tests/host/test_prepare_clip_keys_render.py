# SPDX-License-Identifier: GPL-3.0-only
import pathlib,subprocess,sys,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[2]
class Anchors(unittest.TestCase):
 def test_duplicate_anchors_reject(self):
  groups={'tests/host/smk_clip_knob_test.c':['#define C_BLACK','enum { O_NONE','int main('], 'firmware/src/app/ui.c':['static uint16_t blend(','static uint16_t dim(','/* the radius that suits a box'], 'firmware/src/app/main_fm1.c':['/* ------- clip mode screen:','/* every main loop pass']}
  for path,anchors in groups.items():
   for anchor in anchors:
    with self.subTest(anchor=anchor),tempfile.TemporaryDirectory() as d:
     base=pathlib.Path(d);out=base/'out';out.mkdir()
     for f in groups:
      p=base/f;p.parent.mkdir(parents=True,exist_ok=True);p.write_text((ROOT/f).read_text()+(('\n'+anchor+'\n') if f==path else ''))
     r=subprocess.run([sys.executable,str(ROOT/'tests/host/prepare_clip_keys_render.py'),str(out)],cwd=base,capture_output=True,text=True)
     self.assertNotEqual(r.returncode,0);self.assertIn('AssertionError',r.stderr);self.assertFalse((out/'actual-clip-key-host.inc').exists())
if __name__=='__main__':unittest.main()
