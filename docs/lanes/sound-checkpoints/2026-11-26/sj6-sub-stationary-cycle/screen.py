#!/usr/bin/env python3
"""Reject gross signed source mismatches; this cannot certify parity."""
import argparse
import json
import math
from pathlib import Path

WINDOWS = ('strike-0-50ms', 'early-body-100-500ms', 'blind-loop-head',
           'blind-loop-tail', 'actual-loop-boundary-before', 'actual-loop-boundary-after')
RAW = ('onset', 'first_hold', 'first_release', 'chord', 'final_release', 'full')
ZONES = tuple(range(30, 97, 6))
PIN = '6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
COLS = ['signed_dbr', 'level_db', 'signed_correlation']

def score(row):
    if (not isinstance(row, list) or len(row) != 3
            or not all(type(x) in (float, int) and math.isfinite(x) for x in row)
            or not -1.000001 <= row[2] <= 1.000001):
        raise ValueError(f'invalid signed score: {row!r}')
    return dict(zip(COLS, row))

def screen(doc, variant):
    if doc.get('source_pin') != PIN or doc.get('columns') != COLS:
        raise ValueError('source pin or metric schema mismatch')
    branch = doc['variations'][variant]
    source, raw = branch['raw_source_zone_metrics'], branch['actual_current_sub_source_replay']
    if set(source) != {str(i) for i in ZONES} or set(raw) != set(RAW):
        raise ValueError('missing or unexpected zone or Raw holdout')
    rejects = []
    for i in ZONES:
        blind = source[str(i)]['blind']
        if set(blind) != set(WINDOWS):
            raise ValueError(f'incomplete blind source windows for zone {i}')
        for w in WINDOWS:
            metrics = score(blind[w])
            if metrics['signed_dbr'] >= 0:
                rejects.append({'source_zone_root': i, 'window': w, **metrics})
    return {'variant': variant, 'reject_as_bad_as_silence': rejects,
            'raw_holdouts': {w:score(raw[w]) for w in RAW},
            'passes_release_gate': False}

if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('scores', type=Path)
    p.add_argument('variant')
    p.add_argument('--witness', action='store_true',
                   help='assert the stored stationary-cycle negative control')
    args = p.parse_args()
    result = screen(json.loads(args.scores.read_text()), args.variant)
    bad = result['reject_as_bad_as_silence']
    if args.witness:
        assert args.variant in ('1', '4')
        assert {row['source_zone_root'] for row in bad} == {78, 84, 90, 96}
        assert {row['source_zone_root'] for row in bad
                if row['window'] == 'actual-loop-boundary-before'} == {84, 90, 96}
    print(json.dumps(result, indent=2))
    if bad and not args.witness:
        raise SystemExit(1)
