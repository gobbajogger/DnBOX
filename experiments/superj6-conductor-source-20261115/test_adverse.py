import unittest,math
from conductor import *
class Adverse(unittest.TestCase):
 def test_huge_integer_record_is_status_not_exception(self):
  self.assertEqual(plan_main_pass((Record('unused',10**10000),),b'x',1,True,200,frozenset({'control_apply'}),math.pow).status,'INVALID')
 def test_invalid_capability_type_is_rejected_without_state_change(self):
  s=CutoffState(1,.1,.9,True)
  r=cutoff_step(s,.01,2,None,math.exp)
  self.assertEqual(r.status,'INVALID');self.assertIs(r.state,s)
 def test_clock_bad_residual_rejected_even_at_duplicate_beat(self):
  s=BeatState(1,0.,.5)
  self.assertEqual(beat_catchup(s,0.,120.,True,0).status,'INVALID')
 def test_nonfinite_block_duration_rejected(self):
  with self.assertRaises(ValueError):free_block_time(1024,5e-324)
 def test_note_retrigger_fields_must_be_boolean(self):
  s=NoteState((0,0,0),retriggers=('yes',False,False))
  self.assertEqual(enter_note(s,0).status,'INVALID')
 def test_provider_range_failure_is_not_a_valid_equation(self):
  r=plan_main_pass((),b'x',1,True,200,frozenset({'control_apply'}),lambda a,b:2.)
  self.assertEqual(r.status,'MATH_FAILURE')
  s=CutoffState(1,.1,.9,True)
  self.assertEqual(cutoff_step(s,.01,2,frozenset({'cutoff_apply','global_dispatch'}),lambda x:-1.).status,'MATH_FAILURE')
if __name__=='__main__':unittest.main()
