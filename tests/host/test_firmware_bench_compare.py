import copy
import unittest
from firmware_bench_compare import compare, parse_budget

def receipt():
    shot = dict(duration_ms=8000, shift_ms=0, events=922, strict=1, mhz=360,
                scene_sha256='a'*64, pcm_sha256='b'*64, pcm_bytes=1411764,
                halves=645, max_us=11561, raw_late=0, audio_xruns=0)
    return dict(model='pro', emulator_sha256='c'*64, seed_sha256='d'*64,
                fwsc_sha256='e'*64, elf_sha256='f'*64, passed=True,
                capture_health={'passed':True}, all_active=True, shot=shot)

class Compare(unittest.TestCase):
    def setUp(self):
        self.a=receipt()
        self.b=copy.deepcopy(self.a)
        self.mem=(1036,1244)
    def run_pair(self, **kw):
        return compare(self.a,self.b,self.mem,self.mem,'pro',**kw)
    def test_unchanged(self):
        r=self.run_pair()
        self.assertTrue(r['guards_passed'])
        self.assertEqual(r['base']['deadline_margin_us'],48)
        self.assertEqual(r['delta']['peak_us'],0)
    def test_better(self):
        self.b['shot']['max_us']-=109
        self.assertEqual(self.run_pair()['delta']['peak_us'],-109)
    def test_late_rejected(self):
        self.b['shot']['raw_late']=1
        self.assertFalse(self.run_pair()['guards_passed'])
    def test_equal_limit_rejected(self):
        self.b['shot']['max_us']=11609
        self.assertFalse(self.run_pair()['guards_passed'])
    def test_audio_difference_fails_default(self):
        self.b['shot']['pcm_sha256']='9'*64
        self.assertFalse(self.run_pair()['guards_passed'])
        self.assertTrue(self.run_pair(report_audio_change=True)['guards_passed'])
        self.assertFalse(self.run_pair(report_audio_change=True)['audio_bit_exact'])
    def test_workload_mismatch(self):
        self.b['shot']['events']+=1
        with self.assertRaisesRegex(ValueError,'events'):
            self.run_pair()
    def test_seed_mismatch_requires_matching_repacked_state(self):
        self.b['seed_sha256']='1'*64
        with self.assertRaisesRegex(ValueError,'seed'):
            self.run_pair()
        self.assertTrue(self.run_pair(state_a='2'*64,state_b='2'*64)['guards_passed'])
    def test_capture_health(self):
        self.b['capture_health']['passed']=False
        self.assertFalse(self.run_pair()['guards_passed'])
    def test_budget(self):
        self.assertEqual(parse_budget("pro: XIP free: 1036 B (min 1024); pool margin: 1244 B (min 512)",'pro'),self.mem)
        self.assertEqual(parse_budget("pro: XIP free: 1036 B (min 0); pool margin: 1244 B (min 512)",'pro'),self.mem)
        r=compare(self.a,self.b,self.mem,(0,511),'pro')  # pool below POOL_MIN=512; XIP_MIN is 0
        self.assertFalse(r['guards_passed'])
    def test_mismatched_model(self):
        self.b['model']='elite'
        with self.assertRaisesRegex(ValueError,'model'):
            self.run_pair()

if __name__=='__main__':
    unittest.main()
