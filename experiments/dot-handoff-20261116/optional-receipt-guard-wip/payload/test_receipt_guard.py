import copy, importlib.util, unittest
from pathlib import Path
p=Path(__file__).with_name('receipt_guard.py')
m=None
if p.exists():
 s=importlib.util.spec_from_file_location('guard',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
NOW='2026-11-15T16:03:00Z'
def base():
 return {'id':'qa','is_enabled':True,'timing_mode':'exact_schedule','next_run_time':'2026-11-15T16:52:00Z','run_details':{'delivery_status':'unknown','last_run':{'id':'r1','recorded_status':'completed','scheduled_for':'2026-11-15T15:52:00Z','started_at':'2026-11-15T15:53:18Z','completed_at':'2026-11-15T15:56:09Z'},'latest_result':{'run_id':'r1','created_at':'2026-11-15T15:56:06Z','completed_at':'2026-11-15T15:56:09Z','content_text':'Read f89f4b2e59145e7ec1aef8f9c7f708ea3d98b7ec; no firmware transition.','content_truncated':False,'is_failure_message':False,'requires_user_action':False}}}
class ReceiptGuard(unittest.TestCase):
 def setUp(self):self.assertIsNotNone(m,'Receipt classification implementation is missing')
 def check(self,a):return m.inspect(a,NOW)
 def test_completed_is_not_delivery(self):
  r=self.check(base());self.assertTrue(r['matched_result']);self.assertEqual(r['delivery'],'unknown');self.assertFalse(r['verified_product_progress'])
 def test_null_result_never_success(self):
  a=base();a['run_details']['latest_result']=None;r=self.check(a);self.assertFalse(r['matched_result']);self.assertIn('result_missing',r['issues'])
 def test_wrong_run_result_rejected(self):
  a=base();a['run_details']['latest_result']['run_id']='old';self.assertIn('result_run_mismatch',self.check(a)['issues'])
 def test_old_retry_output_rejected(self):
  a=base();a['run_details']['latest_result']['created_at']='2026-11-15T15:50:00Z';self.assertIn('result_predates_attempt',self.check(a)['issues'])
 def test_future_output_rejected(self):
  a=base();a['run_details']['latest_result']['created_at']='2026-11-15T16:10:00Z';self.assertIn('result_after_observation',self.check(a)['issues'])
 def test_empty_result_not_progress(self):
  a=base();a['run_details']['latest_result']['content_text']=' ';self.assertIn('result_empty',self.check(a)['issues'])
 def test_truncation_not_full_evidence(self):
  a=base();a['run_details']['latest_result']['content_truncated']=True;self.assertIn('result_truncated',self.check(a)['issues'])
 def test_recorded_failure_not_success(self):
  a=base();a['run_details']['last_run']['recorded_status']='failed';r=self.check(a);self.assertIn('recorded_failure',r['issues']);self.assertFalse(r['verified_product_progress'])
 def test_result_failure_independent(self):
  a=base();a['run_details']['latest_result']['is_failure_message']=True;self.assertIn('result_failure_message',self.check(a)['issues'])
 def test_user_action_preserved(self):
  a=base();a['run_details']['latest_result']['requires_user_action']=True;self.assertIn('requires_user_action',self.check(a)['issues'])
 def test_in_progress_not_liveness(self):
  a=base();a['run_details']['last_run']['recorded_status']='in_progress';r=self.check(a);self.assertFalse(r['worker_liveness_proven'])
 def test_no_history_unknown_not_never_ran(self):
  a=base();a['run_details']=None;r=self.check(a);self.assertEqual(r['firing'],'unknown');self.assertIn('run_history_missing',r['issues'])
 def test_late_next_time_is_check_not_confirmed_miss(self):
  a=base();a['next_run_time']='2026-11-15T15:30:00Z';r=self.check(a);self.assertIn('next_occurrence_overdue_check',r['issues']);self.assertFalse(r['missed_run_proven'])
 def test_disabled_not_auto_resumed(self):
  a=base();a['is_enabled']=False;self.assertIn('automation_disabled',self.check(a)['issues'])
 def test_no_input_mutation(self):
  a=base();old=copy.deepcopy(a);self.check(a);self.assertEqual(a,old)
 def test_nested_wrapper(self):
  a=base();r=m.extract({'structuredContent':{'structuredContent':{'automations':[a]}}});self.assertEqual(r,[a])
 def test_invalid_timestamp_is_unknown_not_pass(self):
  a=base();a['run_details']['last_run']['started_at']='yesterday';self.assertIn('invalid_run_timestamp',self.check(a)['issues'])
 def test_mentioned_head_is_not_verified(self):
  r=self.check(base());self.assertEqual(len(r['mentioned_source_heads']),1);self.assertFalse(r['source_heads_live_verified'])
 def test_early_start_relative_schedule_is_not_rejected(self):
  a=base();a['run_details']['last_run']['scheduled_for']='2026-11-15T15:56:00Z';self.assertTrue(self.check(a)['matched_result'])
 def test_receipt_requires_owner_next_and_evidence(self):
  a={'run_id':'r1','outcome':'consumer_intake','source_head':'a'*40};self.assertFalse(m.check_stage_receipt(a,'r1')['valid_structure'])
 def test_receipt_binds_exact_run(self):
  a={'run_id':'old','outcome':'blocked','source_head':'a'*40,'owner':'program','next_action':'check lease','evidence':['proof.json']};self.assertFalse(m.check_stage_receipt(a,'r1')['valid_structure'])
 def test_structure_never_proves_evidence_itself(self):
  a={'run_id':'r1','outcome':'consumer_intake','source_head':'a'*40,'owner':'program','next_action':'integrate','evidence':['proof.json']};r=m.check_stage_receipt(a,'r1');self.assertTrue(r['valid_structure']);self.assertFalse(r['evidence_independently_verified'])
 def test_explicit_no_progress_does_not_require_fake_head(self):
  a={'run_id':'r1','outcome':'no_verified_progress','source_head':None,'owner':'program','next_action':'resolve exact input','evidence':[]};self.assertTrue(m.check_stage_receipt(a,'r1')['valid_structure'])
 def test_missing_result_has_concrete_next_action(self):
  a=base();a['run_details']['latest_result']=None;self.assertIn('Recover',self.check(a)['next_action'])
 def test_unknown_history_has_concrete_next_action(self):
  a=base();a['run_details']=None;self.assertIn('Recover',self.check(a)['next_action'])
if __name__=='__main__':unittest.main()
