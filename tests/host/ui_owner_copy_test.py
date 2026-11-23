#!/usr/bin/env python3
"""Owner's forbidden on-screen copy must not return through literal UI strings."""
from pathlib import Path
import re,unittest
S=(Path(__file__).resolve().parents[2]/'firmware/src/app/ui.c').read_text()
def unapproved_knob_prefix(source):
 # Only the shared percentage stages and the Synth One physical-unit stages
 # may emit K-slot labels. Both constructors must bind their mapped slot.
 line='char *end=put_i(put_s(label,"K"),UI_FIRST_KNOB+slot);'
 signature='static UI_DRAW void physical_stage_label(char *label,int slot,char stage,int raw)\n{'
 start=source.find(signature);end=source.find('\nstatic ',start+1) if start>=0 else -1
 adsr='physical_stage_label(label,e.slot[k],"ADSR"[k],matrix_value(pg.r[p][e.slot[k]]));'
 amp="int raw=matrix_value(r);\n    physical_stage_label(label,0,'D',raw);"
 if start<0 or end<0 or source[start:end].count(line)!=1 or source.count(line)!=1:return True
 if source.count(adsr)!=1 or source.count(amp)!=1 or source.count('physical_stage_label(')!=3:return True
 # Synth One and SuperJ6 physical-unit stages share draw_adsr_inline's
 # stage-unit branch; it must bind each stage's own knob slot and value.
 s1_signature='static UI_DRAW void draw_adsr_inline(int p,int active,int y,int h,int off)\n{'
 s1_line='char *end = put_i(put_s(label, "K"), UI_FIRST_KNOB + e.slot[k]);'
 s1_stage='*end++ = \' \'; *end++ = "ADSR"[k]; *end++ = \' \';'
 start=source.find(s1_signature)
 end=source.find('\n}',start) if start>=0 else -1
 s1=source[start:end] if end>=0 else ''
 if (s1.count(s1_line)!=1 or source.count(s1_line)!=1 or s1.count(s1_stage)!=1 or
     s1.count('pref_t r = pg.r[p][e.slot[k]];')!=1 or s1.count('pref_value_of(r, matrix_value(r), num, unit);')!=1):
  return True
 return 'put_s(label,"K")' in source.replace(line,'',1).replace(s1_line,'',1)
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
  self.assertTrue(unapproved_knob_prefix(S.replace('physical_stage_label(label,e.slot[k]', 'physical_stage_label(label,k')))
  self.assertTrue(unapproved_knob_prefix(S.replace('put_s(label,"K")','put_s(label,"")')))
 def test_synthone_requires_own_stage_slot_and_value(self):
  for old,new in (('UI_FIRST_KNOB + e.slot[k]);','UI_FIRST_KNOB);'),
                  ('*end++ = "ADSR"[k]','*end++ = "ADSR"[0]'),
                  ('pref_t r = pg.r[p][e.slot[k]];','pref_t r = pg.r[p][0];'),
                  ('pref_value_of(r, matrix_value(r), num, unit);','put_s(num,"0");')):
   with self.subTest(old=old):self.assertTrue(unapproved_knob_prefix(S.replace(old,new)))
if __name__=='__main__':unittest.main()
