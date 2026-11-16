#!/usr/bin/env python3
"""Read-only v2 suggestion inbox. Observed account votes are NOT action approval.
Input is a fresh export from the authorized GitHub connector, not issue text.
This program neither authenticates that export nor sends requests or executes work.
"""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path

REPOSITORY = 'gobbajogger/X0X37-for-Loopy-Pro'
OWNER_ID = 255244738


def instant(s):
    value = datetime.fromisoformat(s.replace('Z', '+00:00'))
    if value.tzinfo is None:
        raise ValueError('timestamp needs timezone')
    return value


def pages(envelope, endpoint, now):
    if envelope['endpoint'] != endpoint or envelope['read_status'] != 'success':
        raise ValueError('failed or wrong endpoint')
    age = (now - instant(envelope['observed_at'])).total_seconds()
    if age < 0 or age > 300:
        raise ValueError('stale endpoint observation')
    rows = []
    for index, page in enumerate(envelope['pages'], 1):
        if page['page'] != index or page['status'] != 200 or not isinstance(page['items'], list):
            raise ValueError('incomplete pages')
        final = index == len(envelope['pages'])
        if page['next_page'] != (None if final else index + 1):
            raise ValueError('pagination not complete')
        rows.extend(page['items'])
    if not envelope['pages']:
        raise ValueError('missing terminal page')
    return rows


def evaluate(snapshot, now):
    result = {'schema': 'dnbox-inbox-report/2', 'status': 'BLOCKED', 'items': [],
              'authorization': 'NONE: observed GitHub account votes are advisory, not permission',
              'actions_performed': []}
    try:
        if snapshot['schema'] != 'dnbox-inbox-observation/2' or snapshot['repository'] != REPOSITORY:
            raise ValueError('wrong schema or repository')
        age = (now - instant(snapshot['observed_at'])).total_seconds()
        if age < 0 or age > 300:
            raise ValueError('stale or future observation; reread connector')
        endpoint = '/repos/' + REPOSITORY + '/issues'
        if snapshot['issues']['query'] != {'state': 'open'}:
            raise ValueError('wrong suggestion query')
        issues = pages(snapshot['issues'], endpoint, now)
        seen = set()
        numbers = set()
        items = []
        for issue in issues:
            if 'pull_request' in issue:
                continue
            iid, number = issue['id'], issue['number']
            if type(iid) is not int or type(number) is not int or iid <= 0 or number <= 0 or iid in seen or number in numbers:
                raise ValueError('invalid or duplicate issue identity')
            seen.add(iid)
            numbers.add(number)
            if issue['state'] != 'open' or not isinstance(issue['title'], str):
                raise ValueError('issue changed or malformed')
            votes = pages(snapshot['reactions'][str(iid)], endpoint + '/' + str(number) + '/reactions', now)
            owner_votes = set()
            reaction_ids = set()
            for vote in votes:
                rid = vote['id']
                if type(rid) is not int or rid <= 0 or rid in reaction_ids:
                    raise ValueError('invalid or duplicate reaction identity')
                reaction_ids.add(rid)
                actor = vote['user']
                if actor.get('id') == OWNER_ID and actor.get('type') == 'User' and vote['content'] in ('+1', '-1'):
                    owner_votes.add(vote['content'])
            state = 'conflicting' if len(owner_votes) == 2 else ('yes' if '+1' in owner_votes else 'no' if '-1' in owner_votes else 'none')
            items.append({'issue_id': iid, 'number': number, 'title': issue['title'],
                          'url': f'https://github.com/{REPOSITORY}/issues/{number}',
                          'owner_account_vote': state, 'source_updated_at': issue['updated_at']})
        result.update(status='OBSERVED', observed_at=snapshot['observed_at'], items=sorted(items, key=lambda i: i['number']))
    except (KeyError, TypeError, ValueError, AttributeError) as error:
        result.update(reason=str(error), items=[])
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('snapshot', type=Path)
    args = parser.parse_args()
    result = evaluate(json.loads(args.snapshot.read_text()), datetime.now(timezone.utc))
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0 if result['status'] == 'OBSERVED' else 2


if __name__ == '__main__':
    raise SystemExit(main())
