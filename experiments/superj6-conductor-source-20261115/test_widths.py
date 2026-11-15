import unittest,math
from conductor import *
class Widths(unittest.TestCase):
 def test_unknown_record_outside_binary32_admission_is_retained_by_caller_not_coerced(self):
  records=(Record('unknown',1e100),)
  self.assertEqual(plan_main_pass(records,b'raw',1,True,256,frozenset({'control_apply','delay_enable_prepare'}),math.pow).status,'INVALID')
  self.assertEqual(records[0].value,1e100)
 def test_state_uint64_and_boolean_domains(self):
  for s in (CutoffState(True,.1,.2,True),CutoffState(1,.1,.2,True,False),CutoffState(1,.1,.2,1)):
   self.assertEqual(cutoff_step(s,.01,2,frozenset({'cutoff_apply','global_dispatch'}),math.exp).status,'INVALID')
