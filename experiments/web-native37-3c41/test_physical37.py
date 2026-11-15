"""Actual SMK-37 transport -> current native UI -> engine observation, not a piano clone."""
import unittest
from native_session import NativeSession
class Physical37Tests(unittest.TestCase):
 def setUp(self):self.s=NativeSession()
 def tearDown(self):self.s.close()
 def test_every_physical_key_on_three_current_synth_families(self):
  for fixture in (4,7,8):
   self.s.event('fixture',fixture)
   for physical in range(37):
    f=self.s.event('physical',physical,100)
    self.assertEqual(f['physical']['held'],1<<physical)
    voice=1 if fixture==7 else 0
    self.assertEqual(f['live'][voice]['gate'],1,(fixture,physical,f['live']))
    self.assertEqual(f['live'][voice]['pitch'],36+physical)
    self.assertTrue(f['proof']['native_bytes_equal'])
    f=self.s.event('physical',physical,0)
    self.assertEqual(f['physical']['held'],0);self.assertEqual(f['live'][voice]['gate'],0)
 def test_outer_press_restores_overlapped_central_hold(self):
  self.s.event('fixture',4);self.s.event('physical',17,90)
  for outer in (0,1,2,3,4,32,33,34,35,36):
   f=self.s.event('physical',outer,100);self.assertEqual(f['live'][0]['pitch'],36+outer)
   f=self.s.event('physical',outer,0);self.assertEqual(f['physical']['held'],1<<17);self.assertEqual(f['live'][0]['gate'],1);self.assertEqual(f['live'][0]['uiHeld'],1<<17);self.assertEqual(f['live'][0]['uiHeldCount'],1);self.assertEqual(f['live'][0]['uiPitch'],53)
  f=self.s.event('physical',17,0);self.assertEqual(f['live'][0]['gate'],0)
 def test_all_37_simultaneous_release_and_fixture_reset(self):
  self.s.event('fixture',4)
  for p in range(37):self.s.event('physical',p,100)
  f=self.s.event('snapshot');self.assertEqual(f['physical']['held'],(1<<37)-1);self.assertEqual(f['live'][0]['uiHeld'],(1<<37)-1);self.assertEqual(f['live'][0]['uiHeldCount'],37)
  f=self.s.event('release');self.assertEqual(f['physical']['held'],0);self.assertTrue(all(x['gate']==0 for x in f['live']))
  self.s.event('physical',36,100);f=self.s.event('fixture',8)
  self.assertEqual(f['physical']['held'],0);self.assertTrue(all(x['gate']==0 for x in f['live']))
 def test_view_owner_loss_release_cannot_retrigger(self):
  self.s.event('fixture',4);self.s.event('physical',0,100)
  self.s.event('button',5,1);f=self.s.event('button',5,0)
  self.assertEqual(f['physical']['held'],1)
  self.assertEqual(f['live'][0]['gate'],0)
  f=self.s.event('physical',0,0);self.assertEqual(f['physical']['held'],0);self.assertEqual(f['live'][0]['gate'],0)
 def test_pad_and_physical_central_overlap_preserves_owner(self):
  self.s.event('fixture',0);self.s.event('physical',5,100);self.s.event('pad',0,1)
  f=self.s.event('physical',5,0);self.assertEqual(f['state']['keys']&1,1);self.assertEqual(f['physical']['padHeld'],1);self.assertEqual(f['physical']['held'],0)
  f=self.s.event('pad',0,0);self.assertEqual(f['state']['keys']&1,0);self.assertEqual(f['physical']['padHeld'],0)
 def test_break_outer_count_keys_use_real_smk_frame_caller(self):
  self.s.event('fixture',10);before=self.s.event('snapshot')
  self.assertIsNotNone(before.get('break'),'actual BREAK caller state is not observed')
  target=before['break']['target'];start=before['break']['count'][target]
  lower=self.s.event('physical',0,100);self.s.event('physical',0,0)
  self.assertEqual(lower['break']['count'][target],max(0,start-1))
  upper=self.s.event('physical',2,100);self.s.event('physical',2,0)
  self.assertEqual(upper['break']['count'][target],min(3,max(0,start-1)+1))
  # E3 has no assigned outer BREAK edit; normal anatomy must not invent an action.
  unchanged=self.s.event('physical',4,100);self.s.event('physical',4,0)
  self.assertEqual(unchanged['break'],upper['break'])
 def test_release_ack_consumes_engine_cleanup_across_host_block_phases(self):
  for phase in range(12):
   with self.subTest(phase=phase):
    self.s.event('fixture',4);self.s.event('physical',36,100)
    self.s.event('button',5,1);self.s.event('button',5,0);self.s.event('tick',50)
    self.s.event('button',5,1);self.s.event('button',5,0);self.s.event('tick',50)
    self.s.event('physical',36,0);self.s.event('physical',36,100);self.s.event('tick',phase)
    released=self.s.event('release')
    self.assertEqual(released['physical']['held'],0);self.assertEqual(released['live'][0]['uiHeld'],0)
    self.assertEqual(released['live'][0]['gate'],0,'host release acknowledged before source engine consumed cleanup')
 def test_juno_fixture_reentry_after_real_outer_note_and_release(self):
  for repeat in range(3):
   chosen=self.s.event('fixture',8);self.assertIn('JUNO',chosen['text']);self.s.event('physical',36,100);self.s.event('physical',36,0);self.s.event('release');self.s.event('tick',10)
  self.assertEqual(self.s.event('snapshot')['physical']['held'],0)
 def test_out_of_range_fails_before_mutation(self):
  self.s.event('fixture',4)
  for args in ((-1,100),(37,100),(0,128),(True,100)):
   with self.assertRaises(ValueError):self.s.event('physical',*args)
  self.assertEqual(self.s.event('snapshot')['physical']['held'],0)
if __name__=='__main__':unittest.main()
