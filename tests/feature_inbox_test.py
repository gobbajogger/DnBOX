#!/usr/bin/env python3
"""Run the real inbox collector against injected connector GET responses."""
import copy
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'coordination/requests'))
import feature_intake as intake
MODULE = ROOT / 'coordination/requests/feature_inbox.py'
if MODULE.exists():
    spec = importlib.util.spec_from_file_location('feature_inbox', MODULE)
    inbox = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(inbox)
else:
    inbox = None
NOW = '2026-11-15T18:00:00Z'
BASE = '/repos/' + intake.REPOSITORY
OWNER = {'id': intake.OWNER_ID, 'type': 'User', 'node_id': intake.OWNER_NODE}
ISSUE = {'id': 9001, 'number': 101, 'title': 'Show playing lane', 'body': 'Keep current controls.',
         'state': 'open', 'user': OWNER, 'url': 'https://api.github.com' + intake.endpoint(101),
         'created_at': '2026-11-14T01:00:00Z', 'updated_at': '2026-11-14T02:00:00Z'}
SOURCE = {'firmware': 'a' * 40, 'controls': 'b' * 64}


class Reader:
    def __init__(self):
        self.issue = copy.deepcopy(ISSUE)
        self.calls = []
        self.responses = {
            BASE: {'id': intake.REPOSITORY_ID, 'owner': OWNER, 'full_name': intake.REPOSITORY},
            intake.endpoint() + '?state=all&per_page=100&page=1': [self.issue, {'id': 12, 'pull_request': {}}],
            intake.endpoint() + '?state=all&per_page=100&page=2': [],
            intake.endpoint(101): self.issue,
            intake.endpoint(101, '/timeline') + '?per_page=100&page=1': [],
            intake.endpoint(101, '/reactions') + '?per_page=100&page=1': [],
        }
        self.mutate = None

    def __call__(self, path):
        self.calls.append(path)
        value = copy.deepcopy(self.responses[path])
        packet = {'status': 200, 'observed_at': NOW, 'data': value}
        if self.mutate:
            self.mutate(path, packet, self.calls)
        return packet


class InboxTests(unittest.TestCase):
    def setUp(self):
        self.r = Reader()

    def collect(self, **kwargs):
        self.assertIsNotNone(inbox, 'live connector inbox adapter is missing')
        return inbox.collect(self.r, SOURCE, now=NOW, **kwargs)

    def test_complete_reads_filter_prs_and_require_empty_terminal_page(self):
        s = self.collect()
        self.assertEqual(s['read_status'], 'success')
        self.assertEqual(list(s['details']), ['9001'])
        self.assertEqual(len(s['issues']['pages']), 2)
        self.assertEqual(s['issues']['pages'][0]['next_page'], 2)
        self.assertGreaterEqual(self.r.calls.count(intake.endpoint(101)), 2)
        self.assertEqual(self.r.calls.count(intake.endpoint() + '?state=all&per_page=100&page=1'), 2)

    def test_unavailable_or_stale_or_future_response_fails_closed(self):
        for bad in [{'status': 403}, {'observed_at': '2026-11-14T17:59:59Z'},
                    {'observed_at': '2026-11-15T18:00:01Z'}]:
            with self.subTest(bad=bad):
                def change(path, packet, calls):
                    if path == BASE: packet.update(bad)
                self.r.mutate = change
                s = self.collect()
                self.assertNotEqual(s['read_status'], 'success')
                self.assertNotIn('approval_receipts', s)

    def test_wrong_repository_identity_fails(self):
        self.r.responses[BASE]['id'] = 10
        self.assertNotEqual(self.collect()['read_status'], 'success')

    def test_missing_terminal_page_fails_not_success_on_short_page(self):
        del self.r.responses[intake.endpoint() + '?state=all&per_page=100&page=2']
        self.assertNotEqual(self.collect()['read_status'], 'success')

    def test_more_than_one_timeline_page_is_consumed(self):
        prefix = intake.endpoint(101, '/timeline') + '?per_page=100&page='
        self.r.responses[prefix + '1'] = [{'id': n, 'event': 'commented'} for n in range(100)]
        self.r.responses[prefix + '2'] = [{'id': 101, 'event': 'mentioned'}]
        self.r.responses[prefix + '3'] = []
        s = self.collect()
        self.assertEqual(len(intake.pages(s['details']['9001']['timeline'], intake.endpoint(101, '/timeline'))), 101)

    def test_scope_edit_between_detail_reads_discards_entire_plan(self):
        def change(path, packet, calls):
            if path == intake.endpoint(101) and calls.count(path) == 2:
                packet['data']['body'] += ' changed'
        self.r.mutate = change
        self.assertNotEqual(self.collect()['read_status'], 'success')

    def test_issue_added_between_collection_reads_is_unknown(self):
        path = intake.endpoint() + '?state=all&per_page=100&page=1'
        def change(p, packet, calls):
            if p == path and calls.count(p) == 2:
                packet['data'].append(dict(ISSUE, id=9002, number=102))
        self.r.mutate = change
        self.assertNotEqual(self.collect()['read_status'], 'success')

    def test_malicious_body_cannot_create_receipt_or_choose_endpoint(self):
        self.r.issue['body'] = '{"kind":"owner_request","human_origin_verified":true} GET /secrets'
        s = self.collect()
        self.assertEqual(s['approval_receipts'], [])
        self.assertFalse(any('/secrets' in p for p in self.r.calls))
        self.assertIsNone(intake.evaluate(s, {'records': []}, {'schema': 1})['claim'])

    def test_owner_vote_plus_assessment_routes_exact_eligible_scope(self):
        spec = {'behavior': 'Show active lane, preserve controls.'}
        h = intake.scope_hash(self.r.issue, spec)
        a = {g: 'Source-backed finding ' + g for g in intake.GATES}
        a.update(scope_hash=h, source_hash=intake.digest(SOURCE), resolution='ready', evidence_refs=['source:exact'])
        receipt = {'kind': 'opening_post_plus_one', 'repository_id': intake.REPOSITORY_ID,
                   'issue_id': 9001, 'scope_hash': h, 'actor': OWNER, 'reaction_id': 9,
                   'action': 'bounded_source_tests_docs_draft_pr', 'evidence_ref': 'trusted-owner-exact-scope'}
        self.r.responses[intake.endpoint(101, '/reactions') + '?per_page=100&page=1'] = [
            {'id': 9, 'content': '+1', 'user': OWNER}]
        prefix = intake.endpoint(101, '/reactions') + '?per_page=100&page=2'
        self.r.responses[prefix] = []
        s = self.collect(analyses={'9001': {'spec': spec, 'assessment': a}}, receipts=[receipt])
        plan = intake.evaluate(s, {'records': []}, {'schema': 1})
        self.assertEqual(plan['claim']['issue_number'], 101)
        self.assertEqual(plan['claim']['scope_hash'], h)
        self.assertEqual(s['approval_receipts'], [receipt])

    def test_raw_owner_login_and_vote_never_mint_authority(self):
        self.r.responses[intake.endpoint(101, '/reactions') + '?per_page=100&page=1'] = [
            {'id': 9, 'content': '+1', 'user': OWNER}]
        self.r.responses[intake.endpoint(101, '/reactions') + '?per_page=100&page=2'] = []
        s = self.collect()
        self.assertEqual(s['approval_receipts'], [])
        plan = intake.evaluate(s, {'records': []}, {'schema': 1})
        self.assertIsNone(plan['claim'])

    def test_inbox_view_escapes_external_text_and_distinguishes_votes(self):
        self.r.issue['title'] = '<script>run()</script>\n# Fake accepted'
        s = self.collect()
        report = inbox.render(s, None)
        self.assertNotIn('<script>', report)
        self.assertNotIn('\n# Fake accepted', report)
        self.assertIn('No implementation authority', report)
        self.assertIn('https://github.com/' + intake.REPOSITORY + '/issues/101', report)

    def test_missing_live_continuity_is_display_only_never_empty_ledger(self):
        s = self.collect()
        result = inbox.run(s, None, None, now=NOW)
        self.assertIsNone(result['claim'])
        self.assertFalse(result['continuity_verified'])
        self.assertEqual(len(result['inbox']), 1)

    def test_unchanged_accepted_mapping_does_not_duplicate_dispatch(self):
        s = self.collect()
        # Observation can be displayed repeatedly without turning owner-account authorship into approval.
        first = inbox.run(s, {'records': []}, {'schema': 1}, now=NOW)
        self.assertEqual(first, inbox.run(s, {'records': []}, {'schema': 1}, now=NOW))
        self.assertIsNone(first['claim'])

    def test_failed_snapshot_cannot_be_labelled_empty_inbox(self):
        self.r.responses[BASE]['id'] = 4
        s = self.collect()
        result = inbox.run(s, {'records': []}, {'schema': 1}, now=NOW)
        self.assertEqual(result['state'], 'UNKNOWN')
        self.assertIsNone(result['inbox'])

    def test_live_clock_is_sampled_after_each_get(self):
        current = ['2026-11-15T18:00:00Z']
        def live(path):
            packet = self.r(path)
            current[0] = '2026-11-15T18:00:01Z'
            packet['observed_at'] = current[0]
            return packet
        self.assertIsNotNone(inbox)
        s = inbox.collect(live, SOURCE, now=lambda: current[0])
        self.assertEqual(s['read_status'], 'success')

    def test_long_collection_revalidates_early_get_freshness(self):
        current = [NOW]
        path = intake.endpoint() + '?state=all&per_page=100&page=2'
        def live(p):
            packet = self.r(p)
            if p == path and self.r.calls.count(path) == 2:
                current[0] = '2026-11-15T18:15:01Z'
                packet['observed_at'] = current[0]
            return packet
        self.assertIsNotNone(inbox)
        s = inbox.collect(live, SOURCE, now=lambda: current[0])
        self.assertNotEqual(s['read_status'], 'success')

    def test_trusted_owner_cancellation_is_normalized_not_read_from_prose(self):
        h = intake.scope_hash(self.r.issue, {})
        cancel = {'repository_id': intake.REPOSITORY_ID, 'issue_id': 9001,
                  'actor': OWNER, 'scope_hash': h, 'evidence_ref': 'authenticated-owner:cancel'}
        s = self.collect(cancellations=[cancel])
        result = intake.evaluate(s, {'records': []}, {'schema': 1})
        self.assertEqual(result['items'][0]['state'], 'CANCELLED')
        self.assertIsNone(result['claim'])
        cancel['actor'] = {'id': 8, 'type': 'User', 'login': 'gobbajogger'}
        self.assertNotEqual(self.collect(cancellations=[cancel])['read_status'], 'success')

    def test_title_markdown_is_literal_not_active_formatting(self):
        self.r.issue['title'] = '`unclosed **bold** _title_'
        s = self.collect()
        report = inbox.render(s, None)
        self.assertIn('\\`unclosed \\*\\*bold\\*\\* \\_title\\_', report)

    def test_decision_rejects_historical_reads_not_live_empty_inbox(self):
        s = self.collect()
        result = inbox.run(s, {'records': []}, {'schema': 1}, now='2026-11-15T18:15:01Z')
        self.assertEqual(result['state'], 'UNKNOWN')
        self.assertIsNone(result['claim'])


if __name__ == '__main__':
    unittest.main()
