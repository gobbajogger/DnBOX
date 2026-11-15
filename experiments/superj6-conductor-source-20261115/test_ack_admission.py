import unittest,math
from dataclasses import replace
from conductor import CutoffState,cutoff_ack
class AckAdmission(unittest.TestCase):
 def setUp(self):self.s=CutoffState(1,0.,0.,True,1,True)
 def invalid(self,s,g,t,v):
  r=cutoff_ack(s,g,t,v);self.assertEqual(r.status,'INVALID');self.assertIs(r.state,s);self.assertEqual(r.commands,())
 def test_actual_values(self):
  for v in [1e300,-1e300,float('nan'),float('inf'),True,None,10**10000]:
   with self.subTest(type=type(v).__name__):self.invalid(self.s,1,1,v)
 def test_state_words(self):
  for key in ['current','target']:
   for v in [1e300,-1e300,float('nan'),float('inf'),True,None,10**10000]:
    with self.subTest(key=key,type=type(v).__name__):self.invalid(replace(self.s,**{key:v}),1,1,0.)
 def test_ids(self):
  for v in [True,False,1.,-1,1<<64,None,'1']:
   self.invalid(self.s,v,1,0.);self.invalid(self.s,1,v,0.)
   self.invalid(replace(self.s,generation=v),1,1,0.);self.invalid(replace(self.s,token=v),1,1,0.)
 def test_flags(self):
  for key in ['active','awaiting']:
   for v in [0,1,None,'yes']:self.invalid(replace(self.s,**{key:v}),1,1,0.)
 def test_extreme_difference(self):
  mx=float.fromhex('0x1.fffffep127')
  for s,v in [(replace(self.s,current=mx,target=-mx),0.),(replace(self.s,current=mx,target=mx),-mx)]:
   r=cutoff_ack(s,1,1,v);self.assertEqual(r.status,'OK_STATE');self.assertTrue(r.state.active);self.assertFalse(r.state.awaiting)
 def test_valid_stale_and_high_ids(self):
  r=cutoff_ack(self.s,2,1,0.);self.assertEqual(r.status,'STALE_ACK');self.assertIs(r.state,self.s)
  s=replace(self.s,generation=(1<<64)-1,token=(1<<64)-1);r=cutoff_ack(s,s.generation,s.token,-0.);self.assertEqual(r.status,'OK_STATE');self.assertFalse(r.state.active)
