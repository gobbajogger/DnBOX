#!/usr/bin/env python3
"""Read-only evidence checks. No scheduler, network, filesystem-write or retry API.

A matching service result proves a recorded outcome exists, not its correctness,
worker liveness, user delivery or a completed product. A structured stage receipt
is validated only structurally; its cited evidence still needs actual inspection.
"""
import argparse
from datetime import datetime, timezone, timedelta
import json
import re

def _time(value):
    if not isinstance(value, str):
        raise ValueError('timestamp absent')
    d = datetime.fromisoformat(value.replace('Z', '+00:00'))
    if d.tzinfo is None:
        raise ValueError('timestamp requires timezone')
    return d.astimezone(timezone.utc)

def _valid_run_id(value):
    return isinstance(value, str) and bool(value.strip())

def extract(value):
    """Handle actual connector wrappers without modifying the supplied object."""
    for _ in range(12):
        if not isinstance(value, dict):
            raise ValueError('object result required')
        if value.get('isError') is True:
            raise ValueError('error wrapper cannot provide successful records')
        if isinstance(value.get('automations'), list):
            if any(not isinstance(row, dict) for row in value['automations']):
                raise ValueError('automation records must be objects')
            return value['automations']
        if 'structuredContent' in value:
            value = value['structuredContent']
        elif isinstance(value.get('content'), str):
            value = json.loads(value['content'])
        else:
            raise ValueError('automation array absent')
    raise ValueError('wrapper depth exceeded')

def inspect(automation, observed_at, overdue_grace_seconds=1200):
    now = _time(observed_at)
    if overdue_grace_seconds < 0:
        raise ValueError('negative grace')
    a = automation if isinstance(automation, dict) else {}
    issues = []
    out = dict(automation_id=a.get('id'), firing='unknown', matched_result=False,
               verified_product_progress=False, worker_liveness_proven=False,
               missed_run_proven=False, source_heads_live_verified=False,
               mentioned_source_heads=[], delivery='unknown', issues=issues,
               next_action='Recover the exact run receipt through supported records; record no verified progress meanwhile. No automatic retry or ownership change.')
    if not isinstance(automation, dict):
        issues.append('automation_object_required')
        return out
    if a.get('isError') is True:
        issues.append('error_wrapper')
        return out
    if a.get('is_enabled') is False:
        issues.append('automation_disabled')
    if a.get('is_enabled') and a.get('timing_mode') == 'exact_schedule' and a.get('next_run_time'):
        try:
            if now > _time(a['next_run_time']) + timedelta(seconds=overdue_grace_seconds):
                issues.append('next_occurrence_overdue_check')
        except ValueError:
            issues.append('invalid_next_timestamp')
    details = a.get('run_details') or {}
    run = details.get('last_run')
    if not isinstance(run, dict):
        issues.append('run_history_missing')
        return out
    out['run_id'] = run.get('id')
    out['firing'] = 'recorded_' + str(run.get('recorded_status', 'unknown'))
    if run.get('recorded_status') == 'failed':
        issues.append('recorded_failure')
    time_binding_errors = []
    scheduled = end = None
    try:
        start = _time(run.get('started_at'))
        scheduled = _time(run.get('scheduled_for'))
        if start > now:
            time_binding_errors.append('run_start_after_observation')
        out['start_minus_scheduled_seconds'] = (start - scheduled).total_seconds()
        if run.get('completed_at'):
            end = _time(run['completed_at'])
            if end < start:
                time_binding_errors.append('completion_predates_start')
            else:
                out['recorded_attempt_seconds'] = (end - start).total_seconds()
            if end > now:
                time_binding_errors.append('run_completion_after_observation')
    except (ValueError, TypeError):
        start = None
        issues.append('invalid_run_timestamp')
    result = details.get('latest_result')
    if not isinstance(result, dict):
        issues.append('result_missing')
        return out
    binding_errors = list(time_binding_errors)
    if not _valid_run_id(run.get('id')) or not _valid_run_id(result.get('run_id')) or result.get('run_id') != run.get('id'):
        binding_errors.append('result_run_mismatch')
    created = None
    try:
        created = _time(result.get('created_at'))
        if start is None:
            binding_errors.append('result_attempt_time_unbound')
        elif created < start:
            binding_errors.append('result_predates_attempt')
        if created > now:
            binding_errors.append('result_after_observation')
    except (ValueError, TypeError):
        binding_errors.append('invalid_result_timestamp')
    if 'scheduled_for' in result:
        try:
            result_scheduled = _time(result['scheduled_for'])
            if scheduled is None or result_scheduled != scheduled:
                binding_errors.append('result_occurrence_mismatch')
        except (ValueError, TypeError):
            binding_errors.append('invalid_result_occurrence')
    if result.get('completed_at') is not None:
        try:
            completed = _time(result['completed_at'])
            if completed > now:
                binding_errors.append('result_completion_after_observation')
            if created is None or completed < created:
                binding_errors.append('result_completion_predates_result')
            if start is None or completed < start:
                binding_errors.append('result_completion_attempt_unbound')
        except (ValueError, TypeError):
            binding_errors.append('invalid_result_completion')
    if end is not None and created is not None and created > end:
        binding_errors.append('result_after_run_completion')
    text = result.get('content_text')
    if not isinstance(text, str) or not text.strip():
        binding_errors.append('result_empty')
    out['matched_result'] = not binding_errors
    issues.extend(binding_errors)
    if isinstance(text, str):
        out['mentioned_source_heads'] = sorted(set(re.findall(r'(?<![0-9a-f])[0-9a-f]{40}(?![0-9a-f])', text)))
    if result.get('content_truncated') is True:
        issues.append('result_truncated')
    if result.get('is_failure_message') is True:
        issues.append('result_failure_message')
    if result.get('requires_user_action') is True:
        issues.append('requires_user_action')
    out['next_action'] = ('Inspect the matching exact-head result and cited stage evidence; record actual owner/next action. Do not infer delivery.'
                          if out['matched_result'] else
                          'Recover the exact run receipt through supported records; record no verified progress meanwhile. No automatic retry or ownership change.')
    return out

def check_stage_receipt(receipt, expected_run):
    issues = []
    outcomes = {'source_preserved', 'consumer_intake', 'blocked', 'no_verified_progress'}
    if not isinstance(receipt, dict):
        return {'valid_structure': False, 'issues': ['receipt_object_required'],
                'evidence_independently_verified': False, 'product_delivered': False}
    if not _valid_run_id(expected_run) or not _valid_run_id(receipt.get('run_id')) or receipt.get('run_id') != expected_run:
        issues.append('run_mismatch')
    outcome = receipt.get('outcome')
    if not isinstance(outcome, str) or outcome not in outcomes:
        issues.append('unknown_outcome')
    head = receipt.get('source_head')
    if outcome != 'no_verified_progress' and not (isinstance(head, str) and re.fullmatch('[0-9a-f]{40}', head)):
        issues.append('exact_source_head_required')
    for field in ('owner', 'next_action'):
        if not isinstance(receipt.get(field), str) or not receipt[field].strip():
            issues.append(field + '_required')
    evidence = receipt.get('evidence')
    if not isinstance(evidence, list) or any(not isinstance(x, str) or not x.strip() for x in evidence):
        issues.append('evidence_list_required')
    elif outcome in ('source_preserved', 'consumer_intake') and not evidence:
        issues.append('stage_evidence_required')
    return {'valid_structure': not issues, 'issues': issues,
            'evidence_independently_verified': False, 'product_delivered': False}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('snapshot')
    parser.add_argument('--observed-at', required=True)
    args = parser.parse_args()
    with open(args.snapshot, encoding='utf-8') as f:
        data = json.load(f)
    try:
        if isinstance(data, dict) and data.get('isError') is True:
            raise ValueError('error wrapper cannot provide successful records')
        if isinstance(data, dict) and 'id' in data:
            rows = [data]
        else:
            try:
                rows = extract(data)
            except ValueError:
                # Only a plain named collection may fall back; never unwrap a
                # rejected service/error wrapper through a different path.
                if not isinstance(data, dict) or any(k in data for k in
                    ('isError', 'structuredContent', 'automations', 'content')):
                    raise
                rows = [a for v in data.values() for a in extract(v)]
        output = [inspect(a, args.observed_at) for a in rows]
    except (ValueError, TypeError, AttributeError) as error:
        print(json.dumps({'classifications': [], 'issues': ['invalid_snapshot'],
                          'detail': str(error)}, indent=2))
        raise SystemExit(2)
    print(json.dumps(output, indent=2))
if __name__ == '__main__':
    main()
