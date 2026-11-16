#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import hashlib
import importlib.util
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[3]
spec=importlib.util.spec_from_file_location('attack_run',Path(__file__).with_name('run.py'))
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
HEADER=(ROOT/'firmware/src/app/ui_glyph_renderer.h').read_text()
GLYPH=(ROOT/'firmware/src/app/ui_drum_voice.inc').read_text()
class Guards(unittest.TestCase):
    def test_actual_aa_passthrough(self):
        out=m.instrument_header(HEADER)
        self.assertIn('cv_poly_aa_actual(xy,n,col);',out)
        self.assertIn('for(int i=1;i<n;i++)cv_line_aa(xy[2*i-2],xy[2*i-1],xy[2*i],xy[2*i+1],col);',out)
    def test_changed_or_duplicate_renderer_rejected(self):
        with self.assertRaises(ValueError):m.instrument_header(HEADER+HEADER)
        with self.assertRaises(ValueError):m.instrument_header(HEADER.replace('i<n;i++','i<=n;i++'))
    def test_attack_anchors(self):
        for anchor in ['    if (role == DG_ATTACK) {','    if (role == DG_DECAY || role == DG_NTAIL)']:
            with self.subTest(anchor=anchor):
                with self.assertRaises(ValueError):m.attack_block(GLYPH+anchor)
                with self.assertRaises(ValueError):m.attack_block(GLYPH.replace(anchor,''))
    def test_authentic_negative_source(self):
        p=Path(__file__).with_name('historical-f254-ui_drum_voice.inc');raw=p.read_bytes()
        self.assertEqual(hashlib.sha1(b'blob '+str(len(raw)).encode()+b'\0'+raw).hexdigest(),'7b573423e2291523b747d21b127055f19d26d8d6')
        _,_,old=m.attack_block(raw.decode());_,_,new=m.attack_block(GLYPH)
        self.assertIn('float c1x = 8.0f + 10.0f * q',old)
        self.assertIn('float end=1.f+30.f*q',new)
if __name__=='__main__':unittest.main()
