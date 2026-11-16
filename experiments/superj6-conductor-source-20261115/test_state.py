import unittest,math,struct
import numpy as np
from conductor import *
F=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
class StateTests(unittest.TestCase):
 def test_cutoff_emit_before_snap_and_requires_ack(self):
  s=CutoffState(4,F(.4999),F(.5),True)
  r=cutoff_step(s,.001,2,frozenset({'cutoff_apply','global_dispatch'}),math.exp)
  self.assertEqual(r.status,'PENDING_COMMAND_ACK')
  self.assertEqual(r.state.current,F(.5));self.assertTrue(r.state.active)
  self.assertNotEqual(r.commands[0].value,F(.5));self.assertEqual(r.commands[1].value,r.commands[0].value)
  self.assertEqual(cutoff_step(r.state,.001,2,frozenset({'cutoff_apply','global_dispatch'}),math.exp).status,'AWAITING_ACK')
  self.assertEqual(cutoff_ack(r.state,3,r.state.token,.5).status,'STALE_ACK')
  ack=cutoff_ack(r.state,4,r.state.token,.5)
  self.assertEqual(ack.status,'OK_STATE');self.assertFalse(ack.state.active);self.assertFalse(ack.state.awaiting)
 def test_cutoff_capacity_failure_preserves_state(self):
  s=CutoffState(1,.1,.9,True)
  r=cutoff_step(s,.01,1,frozenset({'cutoff_apply','global_dispatch'}),math.exp)
  self.assertEqual(r.status,'CAPACITY');self.assertIs(r.state,s);self.assertEqual(r.commands,())
 def test_render_block_is_not_voice_chunk_or_fixed80hz(self):
  self.assertEqual(free_block_time(64,48000.),F(np.float32(64)/np.float32(48000)))
  self.assertEqual(free_block_time(512,48000.),F(np.float32(512)/np.float32(48000)))
  self.assertNotEqual(free_block_time(64,48000.),.0125)
 def test_beat_catchup_strict_drop_and_capacity_atomicity(self):
  s=BeatState(2,0.)
  r=beat_catchup(s,.05,120.,True,2)
  self.assertEqual(r.status,'OK_CLOCK');self.assertEqual(len(r.commands),2)
  self.assertTrue(all(c.value==F(.0125) for c in r.commands))
  bad=beat_catchup(s,.05,120.,True,1)
  self.assertEqual(bad.status,'CAPACITY');self.assertIs(bad.state,s)
  drop=beat_catchup(s,.4000001,120.,True,99)
  self.assertEqual(drop.status,'OK_CLOCK');self.assertEqual(drop.commands,());self.assertEqual(drop.state.accumulator,0.)
 def test_duplicate_beat_preserves_existing_fractional_accumulator(self):
  s=BeatState(1,2.,.006)
  r=beat_catchup(s,2.,120.,True,5)
  self.assertEqual(r.status,'OK_CLOCK');self.assertEqual(r.state.accumulator,.006)
 def test_only_first_held_note_resets_delay(self):
  s=NoteState((0,2,3),delay_finished=(True,True,True),delay_passed=(.3,.4,.5))
  a=enter_note(s,0);b=enter_note(s,1)
  self.assertEqual(a.status,'OK_STATE');self.assertEqual(b.status,'OK_STATE')
  self.assertEqual(a.state.delay_passed,(0.,0.,0.));self.assertEqual(b.state.delay_passed,s.delay_passed)
  self.assertTrue(a.state.new_note);self.assertFalse(b.state.new_note)
  self.assertEqual(a.state.retriggers,(False,True,False));self.assertTrue(a.state.retrigger4)
if __name__=='__main__':unittest.main()
