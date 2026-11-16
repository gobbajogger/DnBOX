# SPDX-License-Identifier: GPL-3.0-only
import pathlib,unittest
class ClipText(unittest.TestCase):
 def test_controls_not_instructions(self):
  s=(pathlib.Path(__file__).resolve().parents[2]/'firmware/src/app/main_fm1.c').read_text();s=s[s.index('static void ck_page_draw(void)\n{'):s.index('/* every main loop pass')]
  for banned in ('KNOB2 FIELD','PLAY KEYS'):self.assertNotIn(banned,s)
  for retained in ('CK_PAGE_FIELDS','ck_val_txt','ck_kbd','CK_BIT(cp->keys'):self.assertIn(retained,s)
if __name__=='__main__':unittest.main()
