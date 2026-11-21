#!/usr/bin/env python3
"""Source-free scalar S1 envelope hypothesis and exact numeric-packet checker.

Derived only from the supplied output observations, not original engine code.
This is an isolated component proposal, not a production renderer or full S1.
No acceptance tolerance, gain fit, time shift, or original executable is used.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import math
from pathlib import Path
import resource
import struct
import time

PACK = struct.Struct('<f')
def f32(value):
    return PACK.unpack(PACK.pack(value))[0]

def retention(seconds, sample_rate, attack=False):
    # Inferred from first samples of the three supplied static observations.
    # A nominal attack behaves as a 0.6*A exponential time constant, crossing
    # 0.99 before decay; D and R have their stated exponential time constants.
    seconds = f32(seconds)
    scale = 0.6 if attack else 1.0
    return f32(math.exp(-1.0 / (sample_rate * seconds * scale))) if seconds else 0.0

def render(settings, frames, note_off, event=None, mutant=None):
    p = [f32(v) for v in settings]
    y, a, stage, prior_gate = 0.0, 0.0, 0, False
    data = bytearray()
    event = event or {}
    for frame in range(frames):
        edited = frame == event.get('change_frame', -1)
        if edited:
            p[event['parameter']] = f32(event['value'])
        gate = frame < note_off and frame != event.get('gap_frame', -1)
        if gate != prior_gate:
            if gate:
                stage = 1
                a = retention(p[0], 44100, attack=mutant != 'nominal_attack')
                if mutant == 'reset_on_retrigger':
                    y = 0.0
            else:
                stage = 3
                a = retention(p[3], 44100)
            prior_gate = gate
        if stage == 1 and y >= f32(0.99):
            stage = 2
            a = retention(p[1], 44100)
        if mutant == 'refresh_running_rate' and edited:
            if stage == 1:
                a = retention(p[0], 44100, attack=True)
            elif stage == 2:
                a = retention(p[1], 44100)
            elif stage == 3:
                a = retention(p[3], 44100)
        target = 1.0 if stage == 1 else p[2] if stage == 2 else 0.0
        if mutant == 'snap_sustain' and edited and event['parameter'] == 2:
            y = target
        # Explicit binary32 multiply and add, with no FMA contraction.
        y = f32(f32(a * y) + f32(f32(1.0 - a) * target))
        data.extend(PACK.pack(y))
    return bytes(data)

def compare(reference, candidate):
    differences = 0
    first = None
    max_abs, square_error, finite = 0.0, 0.0, True
    for i, ((r,), (c,)) in enumerate(zip(struct.iter_unpack('<f', reference), struct.iter_unpack('<f', candidate))):
        if reference[4*i:4*i+4] != candidate[4*i:4*i+4]:
            differences += 1
            if first is None:
                first = i
        finite &= math.isfinite(c)
        delta = c-r
        max_abs = max(max_abs, abs(delta))
        square_error += delta*delta
    n = len(reference)//4
    return {'samples': n, 'different_words': differences,
            'first_different_frame': first, 'max_absolute_error': max_abs,
            'rms_error': math.sqrt(square_error/n), 'finite': finite,
            'candidate_sha256': hashlib.sha256(candidate).hexdigest()}

def checked_packet(path, expected):
    raw = path.read_bytes()
    actual = hashlib.sha256(raw).hexdigest()
    if actual != expected:
        raise ValueError(f'input hash mismatch for {path}: {actual}')
    return raw

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--static', type=Path, required=True)
    ap.add_argument('--live', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args=ap.parse_args()
    if args.output.exists():
        raise SystemExit('Refusing to overwrite a prior result')
    started=time.monotonic()
    static_meta=json.loads((args.static/'envelope_observations.json').read_text())
    live_meta=json.loads((args.live/'observations.json').read_text())
    static=checked_packet(args.static/'s1_envelope_observations.f32le',
        '9b164e9b7ebf3fe95665ceaa64afb39d51442e4167305c5d7bc55e115995ade4')
    live=checked_packet(args.live/'reference_observations.f32le',
        '2cafbc8eb4b7aba8e22e0820ce8116d9dc7a57e6f8c41ed162c44ed98c474cfe')
    assert len(static)==264600*4 and len(live)==114688*4
    results=[]
    for case in static_meta['cases']:
        offset=case['byte_offset']; count=case['frames']
        candidate=render(case['settings_ADSR'],count,case['note_off_frame'])
        results.append({'family':'static','id':case['case'], **compare(static[offset:offset+count*4],candidate)})
    for case in live_meta['cases']:
        offset=case['byte_offset']; count=live_meta['frames_per_case']
        candidate=render(live_meta['settings_ADSR'],count,live_meta['gate_off_frame'],case['stimulus'])
        results.append({'family':'live','id':case['id'], **compare(live[offset:offset+count*4],candidate)})
    negatives=[]
    for mutation,case_id in [('nominal_attack',0),('snap_sustain',3),('reset_on_retrigger',5),('refresh_running_rate',1)]:
        case=live_meta['cases'][case_id]
        offset=case['byte_offset']; count=live_meta['frames_per_case']
        candidate=render(live_meta['settings_ADSR'],count,live_meta['gate_off_frame'],case['stimulus'],mutation)
        metric=compare(live[offset:offset+count*4],candidate)
        negatives.append({'mutation':mutation,'live_case':case_id,'rejected':metric['different_words']>0,**metric})
    report={'schema':'source-free-s1-envelope-hypothesis-v1',
        'comparison':'exact binary32 words; zero tolerance; no gain/time fitting',
        'source':str(Path(__file__).resolve()),'source_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        'input_hashes':{'static':hashlib.sha256(static).hexdigest(),'live':hashlib.sha256(live).hexdigest()},
        'status':'EXACT_COMPONENT_MATCH' if all(x['different_words']==0 and x['finite'] for x in results) and all(x['rejected'] for x in negatives) else 'COMPONENT_MISMATCH',
        'results':results,'negative_controls':negatives,
        'wall_seconds':time.monotonic()-started,
        'maximum_rss_kib':resource.getrusage(resource.RUSAGE_SELF).ru_maxrss,
        'limits':['Numeric output observations only; no original source used.',
        'No production activation, instrument conformance, new preset coverage, target math, native timing or memory-fit claim.',
        'Finite fixture agreement is not proof for all rates, controls or voice lifecycle events.']}
    args.output.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'status':report['status'],'samples':sum(x['samples'] for x in results),
        'different_words':sum(x['different_words'] for x in results),
        'negative_controls_rejected':sum(x['rejected'] for x in negatives),
        'wall_seconds':report['wall_seconds'],'maximum_rss_kib':report['maximum_rss_kib']}))
    return 0 if report['status']=='EXACT_COMPONENT_MATCH' else 1

if __name__=='__main__':
    raise SystemExit(main())
