#!/usr/bin/env python3
"""Compare two existing scored DnBOX emulator captures. Not a release gate."""
import argparse
import hashlib
import json
from pathlib import Path
import re

LIMIT_US, XIP_MIN, POOL_MIN = 11609, 1024, 512

def digest(value, field):
    if not isinstance(value,str) or not re.fullmatch('[0-9a-f]{64}',value):
        raise ValueError(f'invalid {field} digest')
    return value

def count(value, field, minimum=0):
    if type(value) is not int or value < minimum:
        raise ValueError(f'invalid {field}')
    return value

def parse_budget(log, model):
    pattern=rf'^{re.escape(model)}: XIP free: (\d+) B \(min 1024\); pool margin: (\d+) B \(min 512\)$'
    hits=re.findall(pattern,log,re.M)
    if len(hits)!=1:
        raise ValueError(f'expected one stock budget line for {model}')
    return tuple(map(int,hits[0]))

def compare(a,b,mem_a,mem_b,model,*,state_a=None,state_b=None,report_audio_change=False):
    if model not in ('elite','pro'):
        raise ValueError('invalid stock model')
    for r in (a,b):
        if r.get('model',model)!=model:
            raise ValueError('model differs from requested stock model')
        for field in ('emulator_sha256','seed_sha256','fwsc_sha256','elf_sha256'):
            digest(r[field],field)
        for field in ('scene_sha256','pcm_sha256'):
            digest(r['shot'][field],field)
    if a['emulator_sha256']!=b['emulator_sha256']:
        raise ValueError('emulator digest differs')
    for k in ('duration_ms','shift_ms','events','strict','mhz','scene_sha256','pcm_bytes','halves'):
        if a['shot'][k]!=b['shot'][k]:
            raise ValueError(f'workload/capture {k} differs')
    if a['shot']['strict']!=1 or a['shot']['mhz']!=360:
        raise ValueError('requires strict 360 MHz receipts')
    for k in ('duration_ms','events','pcm_bytes','halves'):
        count(a['shot'][k],k,1)
    if state_a is not None or state_b is not None:
        if digest(state_a,'baseline user state')!=digest(state_b,'candidate user state'):
            raise ValueError('user state digests differ')
    elif a['seed_sha256']!=b['seed_sha256']:
        raise ValueError('seed differs: provide both user-state digests for repacked seeds')

    def row(r,mem):
        s=r['shot']
        peak=count(s['max_us'],'max_us',1)
        result=dict(peak_us=peak,deadline_margin_us=LIMIT_US-peak,
                    raw_late=count(s['raw_late'],'raw_late'),
                    audio_xruns=count(s['audio_xruns'],'audio_xruns'),
                    xip_free_bytes=count(mem[0],'XIP free'),
                    pool_margin_bytes=count(mem[1],'pool margin'))
        healthy=r.get('passed') is True and r.get('capture_health',{}).get('passed') is True
        activity=r.get('activity_ok',r.get('all_active')) is True
        return result | {'capture_healthy':healthy and activity}

    ra,rb=row(a,mem_a),row(b,mem_b)
    same_pcm=a['shot']['pcm_sha256']==b['shot']['pcm_sha256']
    guarded=lambda v: (v['capture_healthy'] and v['deadline_margin_us']>0 and
        v['raw_late']==v['audio_xruns']==0 and
        v['xip_free_bytes']>=XIP_MIN and v['pool_margin_bytes']>=POOL_MIN)
    return dict(model=model,base=ra,candidate=rb,
        delta={k:rb[k]-ra[k] for k in ra if k!='capture_healthy'},
        audio_bit_exact=same_pcm,audio_change_reporting_only=report_audio_change,
        guards_passed=guarded(ra) and guarded(rb) and (same_pcm or report_audio_change),
        workload={k:a['shot'][k] for k in ('scene_sha256','duration_ms','shift_ms','events','strict','mhz')},
        baseline_fwsc=a['fwsc_sha256'],candidate_fwsc=b['fwsc_sha256'],
        scope='scored software emulator receipts only; not a release gate or fidelity certification')

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--model',choices=('elite','pro'),required=True)
    for k in ('baseline','candidate','baseline-budget','candidate-budget'):
        p.add_argument('--'+k,type=Path,required=True)
    for k in ('baseline','candidate'):
        p.add_argument('--'+k+'-user-state-sha256')
    p.add_argument('--report-audio-change',action='store_true',
                   help='report timing/space even if audio changed; does not score fidelity')
    a=p.parse_args()
    try:
        x,y=a.baseline.read_bytes(),a.candidate.read_bytes()
        data=compare(json.loads(x),json.loads(y),parse_budget(a.baseline_budget.read_text(),a.model),
                     parse_budget(a.candidate_budget.read_text(),a.model),a.model,
                     state_a=a.baseline_user_state_sha256,state_b=a.candidate_user_state_sha256,
                     report_audio_change=a.report_audio_change)
    except (ValueError,TypeError,KeyError,OSError) as exc:
        p.exit(2,f'incomparable receipts: {exc}\n')
    data['receipt_sha256']=[hashlib.sha256(v).hexdigest() for v in (x,y)]
    print(json.dumps(data,indent=2))
    return 0 if data['guards_passed'] else 1

if __name__=='__main__':
    raise SystemExit(main())
