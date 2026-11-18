import importlib.util,json,tempfile,unittest
from pathlib import Path
from PIL import Image
HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('screen_walk',HERE/'screen_walk.py');walk=importlib.util.module_from_spec(spec);spec.loader.exec_module(walk)

class ScreenWalk(unittest.TestCase):
    def test_owner_review_routes_required(self):
        expected=walk.expected_inventory()
        self.assertIn("home-playing",expected)
        self.assertIn("modal-clear-pattern",expected)
        for kind in range(4):
            for part in (2,3):
                for page in range(3):
                    self.assertIn(f"engine-seq-{kind}-part-{part}-page-{page}",expected)

    def fixture(self,root):
        root.mkdir(exist_ok=True)
        import io
        buf=io.BytesIO();image=Image.new('RGB',(240,240),'black');image.putpixel((20,20),(255,255,255));image.save(buf,format='PNG')
        rows=[]
        for name,family in walk.expected_inventory().items():
            (root/(name+'.png')).write_bytes(buf.getvalue())
            rows.append({'kind':'screen','id':name,'family':family})
        return rows
    def log(self,rows,complete=True):
        events=list(rows)
        if complete:events.append({'kind':'complete','screens':len(rows)})
        return '\n'.join('WALK '+json.dumps(row) for row in events)
    def test_valid_actual_frames(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);rows=self.fixture(root);self.assertEqual(walk.validate(self.log(rows),root,0)['screens'],len(rows))
    def test_empty_truncated_duplicate_or_failed_renderer(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);rows=self.fixture(root)
            for log,rc in [(self.log([]),0),('',0),(self.log(rows,False),0),(self.log(rows+rows[:1]),0),(self.log(rows),1),(self.log(rows)+ '\nWALK '+json.dumps({'kind':'complete','screens':len(rows)}),0)]:
                with self.subTest(log=log[:60],rc=rc),self.assertRaises(ValueError):walk.validate(log,root,rc)
    def test_missing_blank_wrong_size_or_linked_frame(self):
        for problem in ('missing','blank','size','symlink'):
            with self.subTest(problem=problem),tempfile.TemporaryDirectory() as d:
                root=Path(d);rows=self.fixture(root);p=root/(rows[0]['id']+'.png')
                if problem=='missing':p.unlink()
                elif problem=='blank':Image.new('RGB',(240,240)).save(p)
                elif problem=='size':Image.new('RGB',(1,1)).save(p)
                else:p.unlink();p.symlink_to(root/(rows[1]['id']+'.png'))
                with self.assertRaises(ValueError):walk.validate(self.log(rows),root,0)
    def test_missing_family_and_path_escape(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);rows=self.fixture(root)
            for candidate in [rows[1:],[dict(rows[0],id='../escape')]+rows[1:]]:
                with self.assertRaises(ValueError):walk.validate(self.log(candidate),root,0)
    def test_malformed_records_and_count(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);rows=self.fixture(root)
            for tail in ('not-json', '[]', '{"kind":"complete","screens":0}', '{"kind":"complete","screens":true}'):
                with self.subTest(tail=tail),self.assertRaises(ValueError):
                    walk.validate(self.log(rows,False)+'\nWALK '+tail,root,0)
    def test_corrupt_frame(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);rows=self.fixture(root)
            (root/(rows[0]['id']+'.png')).write_bytes(b'not a PNG')
            with self.assertRaises(ValueError):walk.validate(self.log(rows),root,0)
    def test_short_walk_cannot_self_certify_coverage(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);rows=self.fixture(root)
            one_per_family=list({r['family']:r for r in rows}.values())
            no_engine_bank=[r for r in rows if not r['id'].startswith('engine-3-')]
            no_page=[r for r in rows if r['id']!='view-00-part-00-voice-00-page-00']
            renamed=[dict(r,id='invented-page') if i==0 else r for i,r in enumerate(rows)]
            (root/'invented-page.png').write_bytes((root/(rows[0]['id']+'.png')).read_bytes())
            for candidate in (one_per_family,no_engine_bank,no_page,renamed):
                with self.subTest(count=len(candidate)),self.assertRaisesRegex(ValueError,'coverage mismatch'):
                    walk.validate(self.log(candidate),root,0)
    def test_missing_drum_source_is_failure(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaises(FileNotFoundError):walk.require_drum_source(Path(d))

if __name__=='__main__':unittest.main()
