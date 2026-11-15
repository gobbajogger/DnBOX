import unittest,math
from conductor import *
class Tokens(unittest.TestCase):
 def test_ack_token_is_on_every_emitted_command(self):
  r=cutoff_step(CutoffState(3,.2,.8,True,9),.01,2,frozenset({'cutoff_apply','global_dispatch'}),math.exp)
  self.assertEqual(tuple(getattr(x,'token',None)for x in r.commands),(10,10))
