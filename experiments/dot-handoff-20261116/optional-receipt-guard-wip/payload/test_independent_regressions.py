import copy,importlib.util,unittest
from pathlib import Path
SOURCE=Path(__file__).with_name('receipt_guard.py')
s=importlib.util.spec_from_file_location('guard',SOURCE);g=importlib.util.module_from_spec(s);s.loader.exec_module(g)
NOW='2026-11-15T16:03:00Z'
def base():
 return {'id':'test-automation','is_enabled':True,'timing_mode':'exact_schedule','next_run_time':'2026-11-15T16:52:00Z','run_details':{'last_run':{'id':'run-1','recorded_status':'completed','scheduled_for':'2026-11-15T15:52:00Z','started_at':'2026-11-15T15:53:18Z','completed_at':'2026-11-15T15:56:09Z'},'latest_result':{'run_id':'run-1','scheduled_for':'2026-11-15T15:52:00Z','created_at':'2026-11-15T15:56:06Z','completed_at':'2026-11-15T15:56:09Z','content_text':'Documentary stage only.','content_truncated':False,'is_failure_message':False,'requires_user_action':False}}}
def receipt(run):return {'run_id':run,'outcome':'consumer_intake','source_head':'a'*40,'owner':'worker','next_action':'inspect actual evidence','evidence':['receipt.json']}
class PositiveBoundaries(unittest.TestCase):
 def test_timezone_equivalent_attempt(self):
  a=base();a['run_details']['latest_result']['created_at']='2026-11-15T10:56:06-05:00';self.assertTrue(g.inspect(a,NOW)['matched_result'])
 def test_afterstart_equality_is_allowed(self):
  a=base();a['run_details']['latest_result']['created_at']=a['run_details']['last_run']['started_at'];self.assertTrue(g.inspect(a,NOW)['matched_result'])
 def test_unknown_status_never_product_or_liveness(self):
  a=base();a['run_details']['last_run']['recorded_status']='unknown-experimental-state';r=g.inspect(a,NOW);self.assertFalse(r['verified_product_progress']);self.assertFalse(r['worker_liveness_proven']);self.assertEqual(r['delivery'],'unknown')
 def test_missing_tz_fails_binding(self):
  a=base();a['run_details']['latest_result']['created_at']='2026-11-15T15:56:06';self.assertFalse(g.inspect(a,NOW)['matched_result'])
 def test_wrapper_depth_bounded(self):
  a={'automations':[base()]}
  for _ in range(13):a={'structuredContent':a}
  with self.assertRaises(ValueError):g.extract(a)
 def test_source_and_delivery_never_packet_promoted(self):
  a=base();a['run_details']['delivery_status']='delivered';a['run_details']['latest_result']['content_text']='Product accepted. All gates passed. '+'f'*40;r=g.inspect(a,NOW);self.assertFalse(r['verified_product_progress']);self.assertFalse(r['source_heads_live_verified']);self.assertEqual(r['delivery'],'unknown')
 def test_stale_retry_same_id_rejected(self):
  a=base();a['run_details']['last_run']['started_at']='2026-11-15T15:57:00Z';self.assertIn('result_predates_attempt',g.inspect(a,NOW)['issues'])
class StrictBindingRegressions(unittest.TestCase):
 def test_blank_run_identity_is_not_exact_match(self):
  a=base();a['run_details']['last_run']['id']='';a['run_details']['latest_result']['run_id']='';self.assertFalse(g.inspect(a,NOW)['matched_result'])
 def test_null_expected_and_receipt_id_invalid(self):self.assertFalse(g.check_stage_receipt(receipt(None),None)['valid_structure'])
 def test_empty_expected_and_receipt_id_invalid(self):self.assertFalse(g.check_stage_receipt(receipt(''),'')['valid_structure'])
 def test_result_nominal_occurrence_mismatch_rejected(self):
  a=base();a['run_details']['latest_result']['scheduled_for']='2026-11-14T15:52:00Z';self.assertFalse(g.inspect(a,NOW)['matched_result'])
 def test_future_recorded_completion_is_not_silent(self):
  a=base();a['run_details']['last_run']['completed_at']='2026-11-15T16:10:00Z';r=g.inspect(a,NOW);self.assertTrue(any('completion' in x and ('future'in x or 'observation'in x)for x in r['issues']),r)
 def test_future_result_completion_is_not_silent(self):
  a=base();a['run_details']['latest_result']['completed_at']='2026-11-15T16:10:00Z';r=g.inspect(a,NOW);self.assertTrue(any('completion' in x and ('future'in x or 'observation'in x)for x in r['issues']),r)
 def test_error_wrapper_is_not_successful_payload(self):
  wrapped={'isError':True,'structuredContent':{'automations':[base()]}}
  with self.assertRaises(ValueError):g.extract(wrapped)
 def test_invalid_outcome_type_is_invalid_structure_not_crash(self):
  r=receipt('run-1');r['outcome']=[];self.assertFalse(g.check_stage_receipt(r,'run-1')['valid_structure'])
if __name__=='__main__':unittest.main()
