import json, subprocess, sys, tempfile, unittest
from pathlib import Path
from test_receipt_guard import m, base, NOW
class Consequences(unittest.TestCase):
 def test_whitespace_identity_rejects(self):
  a=base();a['run_details']['last_run']['id']=' ';a['run_details']['latest_result']['run_id']=' ';self.assertFalse(m.inspect(a,NOW)['matched_result'])
 def test_blank_expected_receipt_rejects(self):
  self.assertFalse(m.check_stage_receipt({'run_id':' '},' ')['valid_structure'])
 def test_nonobject_receipt_is_invalid(self):
  for v in [None,[],0,'x']:self.assertFalse(m.check_stage_receipt(v,'r1')['valid_structure'])
 def test_unhashable_outcome_is_invalid(self):
  self.assertFalse(m.check_stage_receipt({'run_id':'r1','outcome':{}},'r1')['valid_structure'])
 def test_occurrence_timezone_equivalent(self):
  a=base();a['run_details']['latest_result']['scheduled_for']='2026-11-15T10:52:00-05:00';self.assertTrue(m.inspect(a,NOW)['matched_result'])
 def test_supplied_null_occurrence_rejects(self):
  a=base();a['run_details']['latest_result']['scheduled_for']=None;self.assertFalse(m.inspect(a,NOW)['matched_result'])
 def test_completion_before_result_rejects(self):
  a=base();a['run_details']['latest_result']['completed_at']='2026-11-15T15:54:00Z';self.assertFalse(m.inspect(a,NOW)['matched_result'])
 def test_run_completion_before_attempt_rejects(self):
  a=base();a['run_details']['last_run']['completed_at']='2026-11-15T15:50:00Z';self.assertFalse(m.inspect(a,NOW)['matched_result'])
 def test_future_start_rejects(self):
  a=base();a['run_details']['last_run']['started_at']='2026-11-15T16:10:00Z';self.assertFalse(m.inspect(a,NOW)['matched_result'])
 def test_completion_timezone_equivalent(self):
  a=base();a['run_details']['latest_result']['completed_at']='2026-11-15T10:56:09-05:00';self.assertTrue(m.inspect(a,NOW)['matched_result'])
 def cli(self,payload):
  with tempfile.TemporaryDirectory()as d:
   f=Path(d)/'snapshot.json';f.write_text(json.dumps(payload));return subprocess.run([sys.executable,str(Path(__file__).with_name('receipt_guard.py')),str(f),'--observed-at',NOW],text=True,capture_output=True)
 def test_error_wrapper_cli_never_fallback(self):
  r=self.cli({'isError':True,'structuredContent':{'automations':[base()]}});self.assertEqual(r.returncode,2);self.assertEqual(json.loads(r.stdout)['classifications'],[]);self.assertNotIn('Traceback',r.stderr)
 def test_collection_error_prevents_partial_success(self):
  r=self.cli({'good':{'automations':[base()]},'bad':{'isError':True,'structuredContent':{'automations':[base()]}}});self.assertEqual(r.returncode,2);self.assertEqual(json.loads(r.stdout)['classifications'],[])
 def test_direct_error_automation_is_unknown(self):
  a=base();a['isError']=True;r=m.inspect(a,NOW);self.assertFalse(r['matched_result']);self.assertEqual(r['firing'],'unknown')
 def test_nonobject_automation_is_unknown(self):
  r=m.inspect([],NOW);self.assertFalse(r['matched_result']);self.assertIn('automation_object_required',r['issues'])
 def test_error_flag_checked_before_automation_records(self):
  with self.assertRaises(ValueError):m.extract({'isError':True,'automations':[base()]})
 def test_malformed_record_array_rejects(self):
  with self.assertRaises(ValueError):m.extract({'automations':[base(),None]})
if __name__=='__main__':unittest.main()
