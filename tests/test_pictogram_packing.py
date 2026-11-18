#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Compiled decoder must reproduce all 37 original 9x9 pictograms exactly."""
import hashlib,os,re,subprocess,tempfile,unittest
from pathlib import Path
SOURCE=Path(__file__).resolve().parents[1]/'firmware/src/app/ui_pictograms.h'
EXPECTED='d8572c8a64e63bdac18be679768da3893059add54254331328d3532194c0a5cc'
def decode(header):
    with tempfile.TemporaryDirectory(prefix='dnbox-atlas-') as tmp:
        d=Path(tmp);(d/'atlas.h').write_text(header)
        (d/'check.c').write_text('#include <stdio.h>\n#include "atlas.h"\nint main(void){if(UI_G_COUNT!=37)return 2;for(unsigned g=0;g<UI_G_COUNT;g++)for(unsigned r=0;r<9;r++){unsigned v=ui_pictogram_row(g,r);if(v>511)return 3;putchar(v&255);putchar(v>>8);}return 0;}\n')
        subprocess.run([os.environ.get('CC','cc'),'-std=c99','-Wall','-Wextra','-Werror',str(d/'check.c'),'-o',str(d/'check')],check=True,capture_output=True)
        data=subprocess.check_output([str(d/'check')]);
        if len(data)!=666:raise ValueError('row inventory')
        return hashlib.sha256(data).hexdigest()
class ExactAtlas(unittest.TestCase):
    def test_actual_decoder(self):self.assertEqual(decode(SOURCE.read_text()),EXPECTED)
    def test_low_bit_negative(self):
        s=SOURCE.read_text();old='UI_PICTOGRAM_LOW[i] |';self.assertEqual(s.count(old),1)
        self.assertNotEqual(decode(s.replace(old,'(UI_PICTOGRAM_LOW[i]^1u) |')),EXPECTED)
    def test_high_bit_negative(self):
        s=SOURCE.read_text();old='>>(i&7))';self.assertEqual(s.count(old),1)
        self.assertNotEqual(decode(s.replace(old,'>>((i+1)&7))')),EXPECTED)
if __name__=='__main__':unittest.main()
