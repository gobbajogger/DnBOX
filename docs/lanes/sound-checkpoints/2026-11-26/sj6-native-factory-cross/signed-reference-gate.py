#!/usr/bin/env python3
"""Verify the nine frozen AU arms and measure repeated-source / ablation floors."""
import hashlib,json,sys
from pathlib import Path
import numpy as np

cue='17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943'
events=(('on',16384,48,100),('off',38434,48,0),
        ('on',49459,60,96),('on',49459,64,80),('on',49459,67,88),
        ('off',82534,60,0),('off',82534,64,0),('off',82534,67,0))
root=Path(sys.argv[1])
sha=lambda data:hashlib.sha256(data).hexdigest()
manifest={}
for line in (root/'SHA256SUMS').read_text().splitlines():
    h, filename=line.split(maxsplit=1)
    manifest[filename.removeprefix('./')]=h
def pinned(arm,name):
    rel=f'{arm}/{name}'
    f=root/rel
    assert sha(f.read_bytes())==manifest[rel],rel
    return f
def run(uid,arm):
    name=f'program-{uid}-{arm}'
    x=json.loads(pinned(name,'capture.json').read_text())
    a=np.load(pinned(name,'output.npy'),allow_pickle=False)
    assert a.dtype==np.dtype('<f4') and a.shape==(2,148684) and np.isfinite(a).all()
    assert sha(a.tobytes())==x['pcm_sha256']
    assert (x['component'],x['component_version'],x['rate'],x['cue_sha256'],x['warm_frames'],x['frames'],x['mode'])==('aumu/j6ct/AuPr',67072,44100,cue,16384,148684,arm)
    assert len(x['blocks'])==1164 and sum(x['blocks'])==x['frames']
    assert tuple(tuple(d[q] for q in ('action','frame','note','velocity')) for d in x['schedule'])==events
    sig={k['address']:(k['identifier'],k['value']) for k in x['active']}
    assert len(sig)==len(x['active'])==170 and x['active']==x['after']
    return x,a[:,16384:].astype(np.float64),sig
def db_delta(base,variant):
    a=float(np.sum(base*base))
    b=float(np.sum((variant-base)**2))
    return 10*np.log10(b/a) if a and b else ('exact' if b==0 else 'empty_reference')
results=[]
for i,toggle in enumerate(('11','107','40')):
    x,a,b=run(i,'authored')
    y,d,e=run(i,'dry')
    z,r,f=run(i,'authored-repeat')
    assert x['wanted']==y['wanted']==z['wanted'] and b==f
    assert x['blocks']==y['blocks']==z['blocks']
    assert {k for k in b if b[k]!=e[k]}=={toggle}
    windows={'whole': slice(None),'first_release_560':slice(22050,22610),'last_release_560':slice(66150,66710)}
    q={'program':x['wanted']['program'],'uid':x['wanted']['program_uid'],'changed_control':toggle,'repeat_residual_db':{},'effect_residual_db':{},'whole_repeat_per_channel_db':[]}
    for k,s in windows.items():
        q['repeat_residual_db'][k]=db_delta(a[:,s],r[:,s])
        q['effect_residual_db'][k]=db_delta(d[:,s],a[:,s])
    q['whole_repeat_per_channel_db']=[db_delta(a[k],r[k]) for k in range(2)]
    results.append(q)
assert results[2]['repeat_residual_db']['whole']=='exact'
print(json.dumps({'arms_verified':9,'notes':'Signed Float32 original renders only. No gain, offset or channel fitting. No compact acceptance verdict.','results':results},indent=2))
