# SPDX-License-Identifier: GPL-3.0-only
import unittest
from detectors import inspect_screen, largest_empty_rectangle


def glyph(ref, x=10, kind='tuning-fork', name='Tune', row=0, control=0):
    return {'kind': 'glyph', 'ref': ref, 'origin': [x, 40], 'bbox': [x, 40, x+20, 60],
            'glyphType': kind, 'name': name, 'row': row, 'control': control, 'pixels': 30}


def screen(events=()):
    return {'id': 'unit', 'events': list(events), 'panels': [], 'mask': ['0'*20 for _ in range(20)],
            'width': 20, 'height': 20}


class Detectors(unittest.TestCase):
    def test_clipped_passes_are_one_control_not_duplicate(self):
        g = glyph('1:0:0:0')
        self.assertFalse(inspect_screen(screen([g, dict(g)]))['checks']['1']['failures'])

    def test_distinct_positions_same_ref_rejected(self):
        r = inspect_screen(screen([glyph('1:0:0:0'), glyph('1:0:0:0', 70)]))
        self.assertTrue(r['checks']['1']['failures'])
        self.assertTrue(r['checks']['2']['failures'])

    def test_distinct_pitch_ref_must_not_be_tuning_fork(self):
        r = inspect_screen(screen([glyph('1:0:0:0'), glyph('1:0:0:5', 70, name='Pitch')]))
        self.assertFalse(r['checks']['1']['failures'])
        self.assertTrue(r['checks']['2']['failures'])

    def test_labels_are_actual_painted_not_parameter_name(self):
        g = glyph('1:0:0:0')
        r = inspect_screen(screen([g]))
        self.assertTrue(r['checks']['4']['failures'])
        label = {'kind':'text','text':'TUNE','row':0,'control':0,'pixels':9,'bbox':[1,1,12,10]}
        self.assertFalse(inspect_screen(screen([g, label]))['checks']['4']['failures'])
        label['text'] = 'P.DPTH'
        self.assertTrue(inspect_screen(screen([g, label]))['checks']['4']['failures'])

    def test_background_padding_does_not_count_as_content(self):
        s = screen(); s['panels'] = [{'id':'panel','bbox':[0,0,20,20]}]
        r = inspect_screen(s)
        self.assertEqual(r['measurements']['panels'][0]['contentBboxRatio'], 0)
        self.assertTrue(r['checks']['3']['failures'])

    def test_axes_cannot_substitute_for_missing_envelope_curve(self):
        body = {'kind':'envelope-body','origin':[13,30], 'pixels':800,
                'bbox':[13,30,228,90]}
        s=screen([body]);s['panels']=[{'id':'graph','bbox':[0,0,20,20]}]
        s['mask']=['1'*20 for _ in range(20)]
        failures=inspect_screen(s)['checks']['3']['failures']
        self.assertTrue(any(f['reason']=='missing-native-envelope-curve' for f in failures))
        s['events'].append({'kind':'envelope-curve','origin':[13,30],
                            'pixels':0,'bbox':None})
        failures=inspect_screen(s)['checks']['3']['failures']
        self.assertTrue(any(f['reason']=='missing-native-envelope-curve' for f in failures))
        s['events'][-1].update(pixels=200,bbox=[13,30,228,90])
        failures=inspect_screen(s)['checks']['3']['failures']
        self.assertFalse(any(f['reason']=='missing-native-envelope-curve' for f in failures))

    def test_largest_empty_rectangle_exact(self):
        self.assertEqual(largest_empty_rectangle([[False]*4 for _ in range(3)])['area'], 12)
        self.assertEqual(largest_empty_rectangle([[True]*4 for _ in range(3)])['area'], 0)
        self.assertEqual(largest_empty_rectangle([[False,False,True],[False,False,True]])['area'], 4)

    def test_actual_value_overlap_is_reported(self):
        a = dict(glyph('1:0:0:0'), kind='value')
        b = dict(glyph('1:0:0:5'), kind='value')
        self.assertTrue(inspect_screen(screen([a,b]))['overlaps'])

class FinalFrameProof(unittest.TestCase):
    def test_actual_control_named_sound_is_not_section_heading(self):
        events=[glyph('1:1:2:3',name='Sound',control=1),{'kind':'finaltext','text':'SOUND','row':0,'control':1,'pixels':18,'expectedOpaquePixels':18,'finalFrameMatchRatio':1.0}]
        self.assertFalse(inspect_screen(screen(events))['checks']['4']['failures'])

    def test_final_caption_can_survive_zero_incremental_band_paint(self):
        g=glyph('1:0:0:0')
        events=[g,{'kind':'text','text':'TUNE','row':0,'control':0,'pixels':0,'bbox':None},
                {'kind':'finaltext','text':'TUNE','row':0,'control':0,'pixels':18,
                 'expectedOpaquePixels':18,'finalFrameMatchRatio':1.0,'bbox':[1,1,22,12]}]
        self.assertFalse(inspect_screen(screen(events))['checks']['4']['failures'])

    def test_absent_final_caption_cannot_pass_via_incremental_text(self):
        events=[glyph('1:0:0:0'),{'kind':'text','text':'TUNE','row':0,'control':0,'pixels':20},
                {'kind':'finaltext','text':'TUNE','row':0,'control':0,'pixels':0,
                 'expectedOpaquePixels':18,'finalFrameMatchRatio':0.0}]
        self.assertTrue(inspect_screen(screen(events))['checks']['4']['failures'])

    def test_exact_seventy_percent_passes_not_below(self):
        s=screen();s['panels']=[{'id':'panel','bbox':[0,0,10,10]}]
        s['mask']=['1'*7+'0'*13 for _ in range(10)]+['0'*20 for _ in range(10)]
        self.assertEqual(inspect_screen(s)['measurements']['panels'][0]['contentBboxRatio'],.70)
        self.assertFalse(inspect_screen(s)['checks']['3']['failures'])
        s['mask']=['1'*6+'0'*14 for _ in range(10)]+['0'*20 for _ in range(10)]
        self.assertTrue(inspect_screen(s)['checks']['3']['failures'])

    def test_empty_rectangle_matches_bruteforce(self):
        import random
        rng=random.Random(451)
        for _ in range(30):
            mask=[[bool(rng.randrange(2)) for x in range(5)] for y in range(4)]
            best=0
            for y0 in range(4):
                for y1 in range(y0+1,5):
                    for x0 in range(5):
                        for x1 in range(x0+1,6):
                            if not any(mask[y][x] for y in range(y0,y1) for x in range(x0,x1)):
                                best=max(best,(y1-y0)*(x1-x0))
            self.assertEqual(largest_empty_rectangle(mask)['area'],best)

class FailClosed(unittest.TestCase):
    def test_nonempty_trace_tamper_is_rejected_against_actual_log(self):
        import json,copy
        from run import validate_trace_against_log,HERE
        expected=json.loads((HERE/'expected-screens.json').read_text())['screens']
        screens=[];lines=[]
        for identity in expected:
            g=glyph('1:0:0:0');f={'kind':'finaltext','text':'TUNE','expectedOpaquePixels':2,'pixels':2}
            panel={'kind':'panel','id':'panel','bbox':[0,0,20,20]}
            bits='1'+'0'*(240*240-1)
            item={**identity,'kind':'screen','width':240,'height':240,'events':[g,f],'panels':[panel],
                  'mask':[bits[i:i+240] for i in range(0,len(bits),240)]}
            screens.append(item)
            for e in [{**identity,'kind':'screen'},g,f,panel,{'kind':'mask','bits':bits}]:lines.append('FIRST:'+json.dumps(e))
        complete={'kind':'complete','screens':len(expected),'rendererExit':0};lines.append('FIRST:'+json.dumps(complete))
        raw={'screens':screens,'complete':complete};log='\n'.join(lines)
        validate_trace_against_log(log,raw)
        tampered=copy.deepcopy(raw);tampered['screens'][0]['events'][1]['text']='PITCH'
        with self.assertRaises(ValueError):validate_trace_against_log(log,tampered)

    def test_regression_receipt_cannot_overwrite_first_failure(self):
        import tempfile
        from pathlib import Path
        from regression import write_receipt
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'receipt.json';p.write_text('{"exit":1}\n')
            with self.assertRaises(FileExistsError):write_receipt(p,{'exit':0})
            self.assertEqual(p.read_text(),'{"exit":1}\n')

    def test_no_render_structure_cannot_pass(self):
        r=inspect_screen(screen())
        self.assertTrue(all(c['status']!='PASS' for c in r['checks'].values()))

    def test_complete_zero_screens_is_rejected(self):
        from run import parse_trace
        with self.assertRaises(ValueError):parse_trace('FIRST:{"kind":"complete","screens":0,"rendererExit":0}')

    def test_missing_expected_screen_is_rejected(self):
        from run import validate_screens
        with self.assertRaises(ValueError):validate_screens([],{'screens':0,'rendererExit':0})

    def test_actual_duplicate_section_caption_is_reported(self):
        a={'kind':'finaltext','text':'TUNE','row':0,'control':-1,'origin':[11,40],
           'bbox':[11,40,33,52],'pixels':18,'expectedOpaquePixels':18,'finalFrameMatchRatio':1.0}
        b={**a,'control':0,'origin':[22,40],'bbox':[22,40,44,52]}
        r=inspect_screen(screen([glyph('1:0:0:0'),a,b]))
        self.assertTrue(any(e['kind']=='text' for e in r['overlaps']))

if __name__ == '__main__': unittest.main()
