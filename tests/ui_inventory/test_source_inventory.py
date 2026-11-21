# SPDX-License-Identifier: GPL-3.0-only
import tempfile, unittest
from pathlib import Path
from source_inventory import inventory, functions

class SourceInventoryTests(unittest.TestCase):
    def test_lexical_braces_and_declarations(self):
        text='static void declared(void);\nstatic void paint(void) { /* } */ cv_text("}"); }\n'
        rows=functions('ui.c',text)
        self.assertEqual([r['symbol'] for r in rows],['paint'])
        self.assertIn('cv_text',rows[0]['calls'])

    def test_new_route_is_independent_of_walker_output(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td);app=root/'firmware/src/app';app.mkdir(parents=True)
            p=app/'ui.c';p.write_text('static void leaf(void) { cv_rect(0); }\nstatic void screen(void) { leaf(); }\n')
            before=inventory(root)
            p.write_text('\n\n'+p.read_text()+'#if UNKNOWN_BOARD\nstatic void uncovered(void) { cv_text(0); }\n#endif\n')
            after=inventory(root)
            self.assertEqual({r['id'] for r in after['renderers']}-{r['id'] for r in before['renderers']},{'firmware/src/app/ui.c::uncovered'})
            self.assertNotEqual(before['sources'],after['sources'])
            self.assertEqual(after['status'],'REVIEW_REQUIRED')
            self.assertFalse(after['visualApproval'])

    def test_empty_inventory_rejected(self):
        with tempfile.TemporaryDirectory() as td:
            with self.assertRaises(ValueError):inventory(Path(td))

    def test_unknown_function_grammar_remains_a_gap(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td);app=root/'firmware/src/app';app.mkdir(parents=True)
            (app/'ui.c').write_text('static void known(void) { cv_rect(0); }\nstatic widget_t future(void) { cv_text(0); }\n')
            audit=inventory(root)
            self.assertEqual([r['symbol'] for r in audit['unparsedPaintSites']],['cv_text'])

    def test_ck_annotations_do_not_hide_native_painters(self):
        rows=functions('main_fm1.c','CK_FN static void ck_kbd(void) { cv_rect(0); }\n')
        self.assertEqual([r['symbol'] for r in rows],['ck_kbd'])

if __name__=='__main__':unittest.main()
