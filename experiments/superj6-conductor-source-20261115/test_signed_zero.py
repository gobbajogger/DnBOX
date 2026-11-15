import unittest,struct
from conductor import CutoffState,cutoff_step,cutoff_ack
class SignedZero(unittest.TestCase):
 def test_words(self):
  for word in (0,0x80000000):
   z=struct.unpack('<f',struct.pack('<I',word))[0]
   for dt in (0.,-1.):
    r=cutoff_step(CutoffState(1,z,z,True),dt,2,frozenset({'cutoff_apply','global_dispatch'}))
    self.assertEqual(r.status,'PENDING_COMMAND_ACK')
    for v in (r.state.current,r.state.target,*[c.value for c in r.commands]):self.assertEqual(struct.unpack('<I',struct.pack('<f',v))[0],word)
    a=cutoff_ack(r.state,1,r.state.token,z)
    self.assertFalse(a.state.active)
    self.assertEqual(struct.unpack('<I',struct.pack('<f',a.state.current))[0],word)
