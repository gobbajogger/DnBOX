import unittest
from pathlib import Path
try:from native_session import NativeSession
except ModuleNotFoundError:NativeSession=None
class NativeTests(unittest.TestCase):
 def setUp(self):self.assertIsNotNone(NativeSession,'native-host input adapter not implemented yet');self.s=NativeSession()
 def tearDown(self):
  if hasattr(self,'s'):self.s.close()
 def test_native_tiled_commit_captures_all_three_strips(self):
  import json
  self.s.p.stdin.write("snapshot\n");self.s.p.stdin.flush();f=json.loads(self.s.p.stdout.readline())
  self.assertEqual([(s["y"],s["h"]) for s in f["sections"]],[(0,20),(20,138),(158,82)])
 def test_outstanding_event_and_concurrent_close_are_serialized(self):
  import threading,time
  entered=threading.Event();resume=threading.Event();released=threading.Event();results={}
  source_in=self.s.p.stdin;source_out=self.s.p.stdout
  class Input:
   def write(self,value):
    if value=='release\n':released.set()
    return source_in.write(value)
   def flush(self):return source_in.flush()
   def close(self):return source_in.close()
  class Output:
   def readline(self):
    if threading.current_thread().name=='outstanding-input':entered.set();resume.wait(3)
    return source_out.readline()
   def close(self):return source_out.close()
  self.s.p.stdin=Input();self.s.p.stdout=Output()
  def event():
   try:results['frame']=self.s.event('key',0,1)
   except Exception as error:results['error']=error
  a=threading.Thread(target=event,name='outstanding-input');a.start();self.assertTrue(entered.wait(2))
  b=threading.Thread(target=self.s.close,name='concurrent-close');b.start()
  premature=released.wait(.1)
  resume.set();a.join(4);b.join(4)
  self.assertFalse(a.is_alive() or b.is_alive(),'native lifecycle deadlocked')
  self.assertFalse(premature,'close wrote release before the outstanding response was consumed')
  self.assertNotIn('error',results);self.assertEqual(results['frame']['state']['keys']&1,1)
  self.s.close()
  with self.assertRaises(RuntimeError):self.s.event('snapshot')
 def test_raw_blank_stdin_fails_closed(self):
  import subprocess,json
  p=subprocess.run([str(Path(__file__).parent/'native-build/native-server')],input='\n   \n',text=True,capture_output=True,check=True)
  self.assertEqual([json.loads(x) for x in p.stdout.splitlines()],[{'error':'empty input'}]*2)
 def test_close_tolerates_broken_pipe_during_stdin_close(self):
  original=self.s.p.stdin
  class ClosingPipe:
   def write(self,value):return original.write(value)
   def flush(self):return original.flush()
   def close(self):
    original.close();raise BrokenPipeError('child exited during stream close')
  self.s.p.stdin=ClosingPipe()
  self.s.close();self.s.close();self.assertTrue(self.s.closed)
 def test_actual_c_input_and_original_release_after_menu(self):
  before=self.s.event('snapshot');held=self.s.event('pad',0,1)
  self.assertEqual(held['state']['keys']&1,1)
  self.s.event('button',5,1);self.s.event('button',5,0)
  released=self.s.event('pad',0,0)
  self.assertEqual(released['state']['keys'],0)
  self.assertTrue(released['proof']['native_bytes_equal'])
 def test_overlapping_key_and_pad_ownership(self):
  self.s.event('key',0,1);self.s.event('pad',0,1);p=self.s.event('pad',0,0)
  self.assertEqual(p['state']['keys']&1,1)
  p=self.s.event('key',0,0);self.assertEqual(p['state']['keys'],0)
 def test_settings_and_page_encoder_use_current_c(self):
  self.s.event('button',3,1);mix=self.s.event('button',3,0)
  old=mix['state']['page'];turned=self.s.event('encoder',0,1)
  self.assertNotEqual(turned['state']['page'],old)
  self.s.event('button',5,1);glo=self.s.event('button',5,0)
  self.assertEqual(glo['state']['view'],5)
  self.assertIn('GLOBAL',glo['text'])
 def test_unknown_input_and_outer_keys_rejected_without_mutation(self):
  before=self.s.event('snapshot')['state']
  for command,args in [('key',(37,1)),('pad',(16,1)),('memory',(0,)),('encoder',(0,999))]:
   with self.assertRaises(ValueError):self.s.event(command,*args)
  self.assertEqual(self.s.event('snapshot')['state']['keys'],before['keys'])
 def test_all_twenty_native_families_match_actual_frame_bytes(self):
  hashes=set()
  for index in range(20):
   p=self.s.event('fixture',index);self.assertTrue(p['proof']['native_bytes_equal']);hashes.add(p['proof']['native_sha256'])
  self.assertGreaterEqual(len(hashes),18)
if __name__=='__main__':unittest.main()
