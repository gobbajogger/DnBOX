#!/usr/bin/env python3
"""Signed fixed-frequency source discrimination across the sustained Raw chord.

Reports terminal complex partials with no gain or timing correction. It
cannot assign every bin to a voice: notes whose harmonics are nearly
coincident are excluded. Even bins can combine Main and Sub. Short release
spans are deliberately not treated as frequency-resolvable.
"""
import argparse,hashlib,json
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('native',type=Path)
p.add_argument('--compare',type=Path,help='same-clock generated interleaved Float32 stereo')
p.add_argument('--compare-sha')
a=p.parse_args()
m=json.loads((a.native/'capture.json').read_text())
x=np.load(a.native/'output.npy',allow_pickle=False)
sha=lambda b:hashlib.sha256(b).hexdigest()
assert (m['mode'],m['rate'],m['frames'],m['wanted']['program_uid'])==('dry',44100,148684,'9F6F6313-DD03-437F-B2EC-BCF7904DF871')
assert x.dtype==np.dtype('<f4') and x.shape==(2,148684) and sha(x.tobytes())==m['pcm_sha256']
assert np.isfinite(x).all() and m['active']==m['after']
assert [(e['note'],e['frame']) for e in m['schedule'] if e['action']=='on' and e['frame']>16384]==[(60,49459),(64,49459),(67,49459)]
if a.compare:
    b=a.compare.read_bytes()
    assert a.compare_sha and sha(b)==a.compare_sha and len(b)==148684*8
    cmp=np.frombuffer(b,'<f4').reshape(-1,2).T
    assert np.isfinite(cmp).all() and not cmp[:,:16384].any()
else:
    cmp=None
hz=lambda n:440*2**((n-69)/12)
design=[(n,h,h*hz(n)*.5) for n in (48,52,55) for h in range(1,17)]
def db(v):return float(20*np.log10(max(float(v),1e-20)))
def view(y,l,r):
    count=r-l
    mid=np.arange(l,r)
    t=mid/44100
    win=np.hanning(count)
    # Reject bins by independent frequency geometry, not by whether a candidate matches.
    spacing=3*44100/count
    probes=[(key,h,f) for key,h,f in design if all(k2==key or abs(f-f2)>=spacing for k2,h2,f2 in design)]
    angles=(-2j*np.pi*t[:,None])*np.array([d[2] for d in probes])[None,:]
    atom=np.exp(angles)
    vals=2*np.einsum("cn,nh->ch",y[:,l:r]*win,atom)/np.sum(win)
    assert np.isfinite(vals).all()
    # Numerically audit leakage in both directions to detect near-coincident masks.
    leak=[]
    for k,h,f in probes:
        others=[(k2,h2,f2) for k2,h2,f2 in design if k2!=k]
        leak.append(max(abs(np.dot(win,np.exp(2j*np.pi*(f2-f)*t))/np.sum(win)) for k2,h2,f2 in others))
    return (probes,vals,leak)
rows={}
for label,lr in [('sustain_early',(54000,69000)),('sustain_late',(67534,82534))]:
    d,v,leak=view(x,*lr)
    if cmp is not None:
        dd,z,ll=view(cmp,*lr);assert d==dd and leak==ll
    items=[]
    for i,(k,h,f) in enumerate(d):
        bas=v[:,i]
        row={'key':k,'harmonic':h,'hz':f,'native_dBFS':[db(abs(e)) for e in bas],
             'native_phase_deg':np.angle(bas,deg=True).tolist(),'max_mode_leakage_dB':db(leak[i])}
        if cmp is not None:
            row['candidate_level_delta_dB']=[db(abs(e)/abs(y)) for e,y in zip(z[:,i],bas)]
            row['candidate_phase_delta_deg']=np.angle(z[:,i]*bas.conj(),deg=True).tolist()
        items.append(row)
    for k in (48,52,55):
        assert any(z['key']==k and z['harmonic']==1 for z in items)
    rows[label]=items
print(json.dumps(dict(source=m['pcm_sha256'],comparison_sha=a.compare_sha,rows=rows),indent=2,allow_nan=False))
