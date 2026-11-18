#!/usr/bin/env python3
"""Owner's forbidden on-screen copy must not return through literal UI strings."""
from pathlib import Path
import re,unittest
S=(Path(__file__).resolve().parents[2]/'firmware/src/app/ui.c').read_text()
def unapproved_knob_prefix(source):
 # Physical envelope-stage identity is explicitly required, unlike prose
 # knob instructions. Permit exactly the slot-mapped constructor in its owner.
 line='char *end=put_i(put_s(label,"K"),UI_FIRST_KNOB+e.slot[k]);'
 start=source.find('static UI_DRAW void draw_adsr_inline(int p,int active,int y,int h,int off)\n{')
 end=source.find('\nstatic ',start+1) if start>=0 else -1
 if start<0 or end<0 or source[start:end].count(line)!=1 or source.count(line)!=1:
  return True
 return 'put_s(label,"K")' in source.replace(line,'',1)
class Copy(unittest.TestCase):
 def test_instruction_literals_absent(self):
  for text in ('C/D COUNT C#/D# PAGE','A KEYS ','? (NO CYCLE COUNTER)','NO IMAGE',': RUN PERF TEST',': RUN IT AGAIN','KNOBS ',': YES',': NO'):
   with self.subTest(text=text):self.assertFalse('"'+text+'"' in S,text)
 def test_button_prefixes_absent(self):
  for text in ('UI_CAP_SEL "  YES"','UI_CAP_HOME "  NO"'):
   with self.subTest(text=text):self.assertFalse(text in S,text)
 def test_required_stage_identity_is_narrowly_allowed(self):
  self.assertFalse(unapproved_knob_prefix(S))
 def test_unmapped_or_duplicate_prefix_rejected(self):
  self.assertTrue(unapproved_knob_prefix(S+'\nput_s(label,"K");'))
  self.assertTrue(unapproved_knob_prefix(S.replace('UI_FIRST_KNOB+e.slot[k]','UI_FIRST_KNOB+k')))
  self.assertTrue(unapproved_knob_prefix(S.replace('put_s(label,"K")','put_s(label,"")')))
if __name__=='__main__':unittest.main()
