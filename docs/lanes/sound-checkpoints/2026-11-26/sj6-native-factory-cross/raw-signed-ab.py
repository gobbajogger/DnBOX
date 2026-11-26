#!/usr/bin/env python3
"""Compare two actual compact Raw renders with the fixed native dry or authored tape."""
import argparse,hashlib,json
from pathlib import Path
import numpy as np

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('reference',type=Path)
p.add_argument('baseline',type=Path)
p.add_argument('candidate',type=Path)
p.add_argument('--baseline-sha256',required=True)
p.add_argument('--candidate-sha256',required=True)
p.add_argument('--spectral',action='store_true')
a=p.parse_args()
def sha(v):return hashlib.sha256(v).hexdigest()
g=np.load(a.reference/'output.npy',allow_pickle=False)
j=json.loads((a.reference/'capture.json').read_text())
assert g.dtype==np.dtype('<f4') and g.shape==(2,148684) and np.isfinite(g).all()
assert sha(g.tobytes())==j['pcm_sha256'] and j['active']==j['after'] and j['component']=='aumu/j6ct/AuPr'
assert j['wanted']['program_uid']=='9F6F6313-DD03-437F-B2EC-BCF7904DF871'
assert j['cue_sha256']=='17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943'
assert j['rate']==44100 and j['warm_frames']==16384 and j['frames']==148684 and j['mode'] in ('dry','authored')
def load(p,h):
    b=p.read_bytes()
    assert sha(b)==h and len(b)==148684*8
    y=np.frombuffer(b,'<f4').reshape(-1,2).T
    assert np.isfinite(y).all() and not y[:,:16384].any()
    return y.astype('f8')
v=load(a.baseline,a.baseline_sha256)
z=load(a.candidate,a.candidate_sha256)
g=g.astype('f8')
def score(x,y):
    den=np.linalg.norm(y);err=np.linalg.norm(x-y)
    signed=float(20*np.log10(err/den)) if den and err else None
    mag=np.linalg.norm(x)
    level=float(20*np.log10(mag/den)) if den and mag else None
    return dict(signed_dbr=signed,level_db=level,error_rms=float(err/np.sqrt(len(x))))
def modes(y):
    sl=slice(20794,38434);n=np.arange(sl.start,sl.stop,dtype='f8');w=np.hanning(len(n));y=y[sl]
    out={}
    for k in (1,2,3,4,5,7,8,10):
        f=k*32.70319566257483
        c=np.dot(y*w,np.exp((-2j*np.pi*f/44100)*n))*2./sum(w)
        out[k]={'level_dBFS':float(20*np.log10(abs(c))) if c else None,'phase_deg':float(np.angle(c,deg=True))}
    return out
wins={'attack_head':(16384,16448),'attack':(16384,20794),'held':(20794,38434),
      'first_release_560':(38434,38994),'first_release':(38434,49459),
      'chord':(53869,82534),'last_release_560':(82534,83094),'last_release':(82534,104584),
      'late_tail':(104584,148684),'whole':(16384,148684)}
ans={}
for title,(l,h) in wins.items():
    ans[title]=[{'baseline':score(v0[l:h],g0[l:h]), 'candidate':score(z0[l:h],g0[l:h])}
               for v0,z0,g0 in zip(v,z,g)]
out={'reference_pcm_sha256':j['pcm_sha256'],'ref_arm':j['mode'],'baseline_sha':a.baseline_sha256,
     'candidate_sha':a.candidate_sha256,'uniform_test_caveat':'Outer tape pin verified here; renderers must separately show the exact controls and note schedule.',
     'regressions':[(w,i) for w,r in ans.items() for i,ch in enumerate(r)
                    if ch['candidate']['error_rms']>ch['baseline']['error_rms']],
     'windows':ans}
if a.spectral:out['held_complex_bins']={name:[modes(x) for x in pcm] for name,pcm in (('original',g),('baseline',v),('candidate',z))}
print(json.dumps(out,indent=2,allow_nan=False))
