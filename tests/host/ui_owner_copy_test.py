#!/usr/bin/env python3
"""Owner's forbidden on-screen copy must not return through literal UI strings."""
from pathlib import Path
import re,unittest
S=(Path(__file__).resolve().parents[2]/'firmware/src/app/ui.c').read_text()
class Copy(unittest.TestCase):
 def test_instruction_literals_absent(self):
  for text in ('C/D COUNT C#/D# PAGE','A KEYS ','? (NO CYCLE COUNTER)','NO IMAGE',': RUN PERF TEST',': RUN IT AGAIN','KNOBS ',': YES',': NO'):
   with self.subTest(text=text):self.assertFalse('"'+text+'"' in S,text)
 def test_button_prefixes_absent(self):
  for text in ('UI_CAP_SEL "  YES"','UI_CAP_HOME "  NO"','put_s(label,"K")'):
   with self.subTest(text=text):self.assertFalse(text in S,text)
if __name__=='__main__':unittest.main()
