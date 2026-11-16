#!/usr/bin/env python3
"""Focused registration/shared-generator policy regression; no host build launched."""
from pathlib import Path
import subprocess, unittest
ROOT=Path(__file__).resolve().parents[1]
TEXT=(ROOT/'tests/run_tests.sh').read_text()
NAMES=('fwui-light-theme','ui-song-text','ui-list-unit')
def validate(text):
    start=text.index('test_class() {')
    end=text.index('\n}\n',start)+3
    fn=text[start:end]
    for name in NAMES:
        result=subprocess.run(['sh','-c',fn+'\ntest_class "$1"; printf "%s" "$TEST_CLASS"','sh',name],capture_output=True,text=True,check=True)
        if result.stdout!='G': raise ValueError(name+' must serialize shared generation')
        if sum(line.startswith('run '+name+' ') for line in text.splitlines())!=1: raise ValueError(name+' registration')
    line='run smk-ck-page-layout-negative python3 tests/smk_ck_page_layout_negative_test.py'
    if text.splitlines().count(line)!=1: raise ValueError('clip negative registration')
class Policy(unittest.TestCase):
    def test_current(self): validate(TEXT)
    def test_missing_g_rejected(self):
        for name in NAMES:
            with self.subTest(name=name), self.assertRaises(ValueError):
                validate(TEXT.replace('    fwui-light-theme|ui-song-text|ui-list-unit) TEST_CLASS=G ;;','    '+ '|'.join(n for n in NAMES if n!=name)+') TEST_CLASS=G ;;'))
    def test_missing_selector_rejected(self):
        for name in NAMES:
            with self.subTest(name=name), self.assertRaises(ValueError):
                validate('\n'.join(l for l in TEXT.splitlines() if not l.startswith('run '+name+' ')))
    def test_missing_clip_negative_rejected(self):
        with self.assertRaises(ValueError):validate(TEXT.replace('run smk-ck-page-layout-negative python3 tests/smk_ck_page_layout_negative_test.py',''))
if __name__=='__main__': unittest.main()
