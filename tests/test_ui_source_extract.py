#!/usr/bin/env python3
"""Boundary/annotation negatives plus the real pad-mode wiring guard."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from ui_source_extract import function, autosave
ROOT=Path(__file__).resolve().parents[1]
UI=(ROOT/'firmware/src/app/ui.c').read_text()

class Extract(unittest.TestCase):
    signatures=('static void key_event(int k, int down)', 'static void leds(void)', 'void ui_frame(void)')
    def test_actual_annotated_and_plain_functions_match(self):
        for signature in self.signatures:
            with self.subTest(signature=signature):
                actual=function(UI,signature)
                self.assertIn('UI_DRAW',actual.split('{',1)[0])
                self.assertEqual(function(UI.replace('UI_DRAW ',''),signature),actual.replace('UI_DRAW ',''))
                self.assertTrue(actual.rstrip().endswith('}'))
    def test_all_three_missing_and_duplicate_definitions_rejected(self):
        for signature in self.signatures:
            body=function(UI,signature)
            for mutated in (UI.replace(body.rstrip(),'',1),UI+'\n'+body):
                with self.subTest(signature=signature):
                    with self.assertRaisesRegex(ValueError,'exactly one definition'):function(mutated,signature)
    def test_comments_strings_and_nested_braces_are_not_boundaries(self):
        text='static UI_DRAW void probe(void)\n{ const char *s="} {"; /* } */\n if(1){ char c=\'}\'; } // }\n}\nvoid after(void){bad();}\n'
        result=function(text,'static void probe(void)')
        self.assertNotIn('after',result);self.assertIn("char c='}'",result)
        with self.assertRaisesRegex(ValueError,'unterminated'):function(text.split('\n}\n')[0],'static void probe(void)')
    def test_autosave_is_exact_function_not_the_following_ui(self):
        result=autosave(UI)
        self.assertTrue(result.endswith(function(UI,'static void autosave(void)')))
        self.assertNotIn('void ui_frame(',result);self.assertNotIn('void ui_say(',result)
        for anchor in ('#define AUTOSAVE_QUIET','#define AUTOSAVE_GAP','static void autosave(void)'):
            with self.subTest(anchor=anchor):
                with self.assertRaises(ValueError):autosave(UI.replace(anchor,anchor+'_removed',1))
                original=next(line for line in UI.splitlines() if line.startswith(anchor))
                duplicate=original+'\n' if anchor.startswith('#') else function(UI,anchor)
                with self.assertRaises(ValueError):autosave(UI+'\n'+duplicate)
    def test_actual_padcal_guard_mutation_is_rejected(self):
        signature='static void key_event(int k, int down)';body=function(UI,signature)
        guard='smk_clip_mode_on() || smk_padcal_on()';self.assertEqual(body.count(guard),1)
        with tempfile.TemporaryDirectory(prefix='padcal-guard-') as d:
            path=Path(d)/'ui.c';path.write_text(UI.replace(body.rstrip(),body.replace(guard,'0').rstrip(),1))
            p=subprocess.run([sys.executable,str(ROOT/'tests/smk_padcal_wiring_test.py')],env=dict(os.environ,SMK_PADCAL_UI_SOURCE=str(path)),capture_output=True,text=True,timeout=10)
            self.assertEqual(p.returncode,1,p.stdout+p.stderr)
            self.assertIn('FAIL UI: no keys in the mode',p.stdout)
            self.assertNotIn('Traceback',p.stderr)
if __name__=='__main__':unittest.main(verbosity=2)
