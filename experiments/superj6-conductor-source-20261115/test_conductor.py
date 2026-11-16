import unittest,math,struct
F=lambda x:struct.unpack("<f",struct.pack("<f",x))[0]
from conductor import Record,plan_main_pass
CAPS=frozenset({'control_apply','delay_enable_prepare','deferred_volume'})
class MainPass(unittest.TestCase):
 def test_first_record_in_original_control_order_not_json_order(self):
  records=(Record('neoncutoffLayer1',.4),Record('neonvolumeControl',1.32),Record('neonvolumeControl',0.))
  r=plan_main_pass(records,b'raw-not-parsed',7,True,512,CAPS,math.pow)
  self.assertEqual(r.status,'OK_MAIN_PASS_ONLY')
  self.assertEqual((r.commands[0].id,r.commands[0].value),(0,F(1.32)))
  self.assertEqual((r.commands[1].id,r.commands[1].value),(7,F(.4)))
  self.assertEqual(r.records,records);self.assertEqual(r.raw,b'raw-not-parsed')
 def test_capacity_failure_has_no_partial_commands(self):
  r=plan_main_pass((),b'x',1,True,0,CAPS,math.pow)
  self.assertEqual(r.status,'CAPACITY');self.assertEqual(r.commands,())
 def test_missing_math_and_executor_capabilities_fail_closed(self):
  r=plan_main_pass((),b'x',1,True,512,CAPS,None)
  self.assertEqual(r.status,'MISSING_POW');self.assertEqual(r.commands,())
  r=plan_main_pass((),b'x',1,True,512,frozenset(),math.pow)
  self.assertEqual(r.status,'MISSING_CAPABILITY');self.assertEqual(r.commands,())
 def test_volume_deferral_is_only_flag_false(self):
  a=plan_main_pass((),b'x',9,True,512,CAPS,math.pow)
  b=plan_main_pass((),b'x',9,False,512,CAPS,math.pow)
  self.assertEqual(a.status,'OK_MAIN_PASS_ONLY');self.assertEqual(b.status,'OK_MAIN_PASS_ONLY')
  self.assertEqual(a.commands[0].op,'apply_control');self.assertEqual(b.commands[0].op,'defer_volume')
  self.assertEqual(b.commands[0].delay_seconds,F(.15))
  self.assertTrue(all(c.generation==9 and c.order==i for i,c in enumerate(b.commands)))
if __name__=='__main__':unittest.main()
