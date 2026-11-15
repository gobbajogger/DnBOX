#!/usr/bin/env python3
"""Collect the GitHub suggestion inbox through a trusted read-only connector.

This module never authenticates from issue prose and never writes remote state.
The reader callable supplies fresh supported connector GETs, not arbitrary URLs.
The current sole coordinator authenticates receipts and serializes acceptance.
"""
import argparse
from collections import defaultdict, deque
from datetime import datetime, timezone
import html
import json
import re
from pathlib import Path

import feature_intake as intake

BASE = '/repos/' + intake.REPOSITORY
MAX_PAGES = 1000
FINGERPRINT = ('id', 'number', 'url', 'title', 'body', 'state', 'user', 'updated_at')


def stamp(value):
    parsed = datetime.fromisoformat(value.replace('Z', '+00:00'))
    if parsed.tzinfo is None:
        raise ValueError('timestamp has no timezone')
    return parsed.astimezone(timezone.utc)


def fingerprint(issue):
    return intake.digest({key: issue.get(key) for key in FINGERPRINT})


def collect(read, source_closure, *, now, analyses=None, receipts=(), cancellations=(), max_age_seconds=900):
    """Finish all reads before returning a usable observation.

    ``read(path)`` returns status, observed_at, data from an authenticated connector
    GET. It must not infer status/completeness from an excerpt. Every collection
    is walked through an empty terminal page, even when its last page is short.
    Two collection scans and two detail reads detect changing scope/membership.
    This detects observed races, not a transactional GitHub snapshot.
    ``receipts`` and ``analyses`` are separate trusted coordinator inputs, never
    extracted from issue title/body/comment text. Old exact-scope receipts retain
    their authority only under feature_intake's continuity/cancellation checks.
    """
    reads = []
    analyses = analyses or {}
    try:
        clock = now if callable(now) else lambda: now
        stamp(clock())
        if not 0 < max_age_seconds <= 900:
            raise ValueError('freshness limit must be between 1 and 900 seconds')

        def get(path):
            packet = read(path)
            if packet.get('status') != 200:
                raise ValueError('required GET failed: ' + path)
            age = (stamp(clock()) - stamp(packet['observed_at'])).total_seconds()
            if age < 0 or age > max_age_seconds:
                raise ValueError('stale/future GET: ' + path)
            data = packet['data']
            reads.append({'path': path, 'observed_at': packet['observed_at'],
                          'sha256': intake.digest(data)})
            return data

        def pages(path, query=''):
            env = {'endpoint': path, 'read_status': 'success', 'pages': []}
            for p in range(1, MAX_PAGES + 1):
                data = get(path + '?' + query + 'per_page=100&page=' + str(p))
                if not isinstance(data, list):
                    raise ValueError('non-list page: ' + path)
                env['pages'].append({'page': p, 'status': 200, 'items': data,
                                     'next_page': p + 1 if data else None})
                if not data:
                    return env
            raise ValueError('pagination limit reached: ' + path)

        def issues(env):
            result = {}
            numbers = set()
            for issue in intake.pages(env, intake.endpoint()):
                if 'pull_request' in issue:
                    continue
                if type(issue.get('id')) is not int or type(issue.get('number')) is not int:
                    raise ValueError('missing stable issue identity')
                if issue['id'] in result or issue['number'] in numbers:
                    raise ValueError('duplicate issue across pages; collection changed')
                if issue.get('url') != 'https://api.github.com' + intake.endpoint(issue['number']):
                    raise ValueError('issue endpoint identity mismatch')
                result[issue['id']] = issue
                numbers.add(issue['number'])
            return result

        repo = get(BASE)
        if (repo.get('id') != intake.REPOSITORY_ID or not intake.owner(repo.get('owner', {}))
                or repo.get('full_name') != intake.REPOSITORY):
            raise ValueError('repository/owner identity mismatch')
        listing = pages(intake.endpoint(), 'state=all&')
        listing['query'] = {'state': 'all'}
        first = issues(listing)
        details = {}
        for iid, listed in sorted(first.items()):
            number = listed['number']
            before = get(intake.endpoint(number))
            timeline = pages(intake.endpoint(number, '/timeline'))
            reactions = pages(intake.endpoint(number, '/reactions'))
            after = get(intake.endpoint(number))
            if fingerprint(before) != fingerprint(after) or fingerprint(before) != fingerprint(listed):
                raise ValueError('issue changed during collection: ' + str(number))
            if 'pull_request' in before:
                raise ValueError('issue detail changed into PR')
            analysis = analyses.get(str(iid), {})
            details[str(iid)] = {'read_status': 'success', 'issue': after,
                                 'timeline': timeline, 'reactions': reactions,
                                 'spec': analysis.get('spec', {}),
                                 'assessment': analysis.get('assessment', {})}
        second = issues(pages(intake.endpoint(), 'state=all&'))
        if ({i: fingerprint(v) for i, v in first.items()}
                != {i: fingerprint(v) for i, v in second.items()}):
            raise ValueError('issue collection changed during read')
        # Trusted owner-conversation cancellations are separate from raw GitHub
        # events; retain their exact identity and provenance, never parse prose.
        normalized_cancellations = []
        for event in cancellations:
            iid = str(event.get('issue_id'))
            if (event.get('repository_id') != intake.REPOSITORY_ID or iid not in details
                    or not intake.owner(event.get('actor', {}))
                    or not re.fullmatch(r'[0-9a-f]{64}', event.get('scope_hash', ''))
                    or not isinstance(event.get('evidence_ref'), str) or not event['evidence_ref']):
                raise ValueError('invalid trusted owner cancellation identity')
            normalized = dict(event, event='owner_cancelled')
            details[iid]['timeline']['pages'][-1]['items'].append(normalized)
            normalized_cancellations.append(intake.digest(normalized))
        ended = clock()
        freshness(reads, ended, max_age_seconds)
        # Deep copy separate trusted inputs so later caller mutation cannot alter
        # the observation identity. No raw issue field becomes an approval.
        return json.loads(json.dumps({'schema': 1, 'read_status': 'success',
            'observed_at': ended, 'repository': repo, 'source_closure': source_closure,
            'issues': listing, 'details': details, 'approval_receipts': list(receipts),
            'read_evidence': reads, 'trusted_cancellation_sha256': normalized_cancellations}))
    except (KeyError, TypeError, ValueError, AttributeError, OSError) as error:
        return {'schema': 1, 'read_status': 'failed', 'reason': str(error), 'read_evidence': reads}


def freshness(reads, now, max_age_seconds=900):
    current = stamp(now() if callable(now) else now)
    if not reads:
        raise ValueError('missing read-time evidence')
    for record in reads:
        age = (current - stamp(record['observed_at'])).total_seconds()
        if age < 0 or age > max_age_seconds:
            raise ValueError('stale/future observation at decision: ' + record['path'])


def run(snapshot, ledger, checkpoint, *, now):
    """Expose inbox state even when continuity is unavailable; never reset it."""
    result = {'schema': 1, 'state': 'UNKNOWN', 'claim': None, 'inbox': None,
              'continuity_verified': False, 'observation_sha256': intake.digest(snapshot)}
    if snapshot.get('read_status') != 'success':
        result['reason'] = snapshot.get('reason', 'required reads unavailable')
        return result
    try:
        freshness(snapshot.get('read_evidence', []), now)
    except (TypeError, ValueError, KeyError, AttributeError) as error:
        result['reason'] = str(error)
        return result
    result['inbox'] = []
    for _, d in sorted(snapshot['details'].items(), key=lambda pair: pair[1]['issue']['number']):
        issue = d['issue']
        reactions = intake.pages(d['reactions'], intake.endpoint(issue['number'], '/reactions'))
        result['inbox'].append({'issue_number': issue['number'], 'issue_id': issue['id'],
            'title': issue['title'], 'issue_state': issue['state'],
            'url': 'https://github.com/' + intake.REPOSITORY + '/issues/' + str(issue['number']),
            'owner_opening_plus_one': [r['id'] for r in reactions
                if r.get('content') == '+1' and intake.owner(r.get('user', {}))],
            'authority': 'No implementation authority established by inbox display'})
    if ledger is None or checkpoint is None:
        result['state'] = 'DISPLAY_ONLY'
        result['reason'] = 'Live ledger/checkpoint unavailable; existing continuity must be recovered by its sole owner'
        return result
    decision = intake.evaluate(snapshot, ledger, checkpoint)
    result.update(state=decision['state'], reason=decision['reason'], decision=decision,
                  claim=decision.get('claim'), continuity_verified='proposed_checkpoint' in decision)
    return result


def safe(text):
    text = html.escape(' '.join(str(text).split()), quote=True).replace('[', '&#91;').replace(']', '&#93;')
    return re.sub(r'([\\`*_{}()#+.!|~-])', r'\\\1', text)


def render(snapshot, result):
    """Readable local report. Publishing/sending this report needs actual authority."""
    result = result if result is not None else run(snapshot, None, None, now=snapshot.get('observed_at', ''))
    lines = ['# DnBOX suggestion inbox', '', safe(result['state']) + ': ' + safe(result['reason']), '',
             'Opening-post owner +1 counts below are observations, not approval receipts.',
             'No implementation authority follows from owner-account authorship or vote counts.', '']
    if result['inbox'] is None:
        lines.append('Inbox contents unknown; a failed read is not an empty inbox.')
    for item in result['inbox'] or []:
        lines.extend([f"## #{item['issue_number']}: {safe(item['title'])}", item['url'],
                      f"State: {item['issue_state']}. Owner opening-post +1: {len(item['owner_opening_plus_one'])}.",
                      safe(item['authority']), ''])
    return '\n'.join(lines) + '\n'


class RecordedReader:
    """Replay exact previously recorded GET responses without network/auth state.

    Repeated endpoints require separate records. Reusing a single record cannot
    masquerade as a second live read. Capture integrity is the caller's boundary.
    """
    def __init__(self, records):
        self.records = defaultdict(deque)
        for record in records:
            self.records[record['path']].append(record)

    def __call__(self, path):
        if not self.records[path]:
            raise ValueError('missing separate recorded GET: ' + path)
        return self.records[path].popleft()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('capture', type=Path)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--now', required=True, help='UTC read time; old captures remain historical')
    parser.add_argument('--ledger', type=Path)
    parser.add_argument('--checkpoint', type=Path)
    parser.add_argument('--trusted-analysis', type=Path)
    parser.add_argument('--trusted-receipts', type=Path)
    parser.add_argument('--trusted-cancellations', type=Path)
    parser.add_argument('--markdown', action='store_true')
    args = parser.parse_args()
    try:
        def load(path):
            return json.loads(path.read_text()) if path else None
        snapshot = collect(RecordedReader(load(args.capture)), load(args.source), now=args.now,
                           analyses=load(args.trusted_analysis), receipts=load(args.trusted_receipts) or [],
                           cancellations=load(args.trusted_cancellations) or [])
        result = run(snapshot, load(args.ledger), load(args.checkpoint), now=args.now)
        output = render(snapshot, result) if args.markdown else json.dumps(
            {'observation': snapshot, 'result': result}, sort_keys=True, ensure_ascii=False, indent=2)
        print(output)
        return 2 if result['state'] in ('UNKNOWN', 'DISPLAY_ONLY', 'BLOCKED') else 0
    except (ValueError, OSError, TypeError, KeyError) as error:
        print(json.dumps({'state': 'UNKNOWN', 'claim': None, 'reason': str(error)}))
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
