"""Replay a frozen heldout without ratio-driven changes to the anchor.
Compare h2..18 using H1-referenced magnitude/signed phase, plus H1 level if
an *actual prefilter stage tape* is supplied. Full installed AU must use the
explicitly proven matching forward Normal and suffix; never time-fit audio.
"""
import argparse, ctypes, json, math
from pathlib import Path
import numpy as np
def coeff(x,f,t,H):
    return np.array([2*np.mean(x*np.exp(-2j*np.pi*f*n*t)) for n in range(1,H+1)])
def harmonic_rel(h):
    return h / abs(h[0]) * np.exp(-1j*np.angle(h[0])*np.arange(1,len(h)+1))
def score_h(a,b):
    r,s=harmonic_rel(a),harmonic_rel(b)
    return float(20*np.log10(np.linalg.norm(r[1:]-s[1:])/np.linalg.norm(s)))
if __name__=='__main__':
    p=argparse.ArgumentParser()
    p.add_argument('model_so');p.add_argument('projection_json')
    a=p.parse_args()
    j=json.loads(Path(a.projection_json).read_text())
    f32=ctypes.c_float
    run=ctypes.CDLL(a.model_so).sj6_anchor_run
    run.argtypes=[ctypes.POINTER(f32),ctypes.c_uint,f32,ctypes.POINTER(f32),ctypes.c_uint]
    rec={}
    for case in j['cases']:
        s=(f32*17)()
        # A sequence of [length,base_phase_increment]. Both note reset and
        # held bend are represented, not inferred from audio.
        y=[]
        for n,step in case['runs']:
            v=(f32*n)()
            run(s,case['shape'],step,v,n)
            y.append(np.frombuffer(v,dtype='f4').astype('f8'))
        y=np.concatenate(y)
        scored=[]
        for w in case['windows']:
            lo,hi=w['frames'];f=w['step'];H=min(18,int(.49/f))
            x=coeff(y[lo:hi],f,np.arange(lo,hi),H)
            if 'transfer' in w:
                x*=np.array([complex(*q) for q in w['transfer']][:H])
            true=np.array([complex(*q) for q in w['harmonics']][:H])
            scored.append({'frames':[lo,hi],'harmonic_rel_residual_db':score_h(x,true),
                           'fundamental_model_peak':float(abs(x[0])),
                           'fundamental_reference_peak':float(abs(true[0])),
                           'fundamental_phase_delta_deg':float(np.angle(x[0]/true[0],deg=True))})
        rec[case['name']]=scored
    print(json.dumps(rec,indent=2))
