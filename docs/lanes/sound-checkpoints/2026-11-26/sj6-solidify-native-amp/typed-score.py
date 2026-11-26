import sys,numpy as np,json,hashlib
from pathlib import Path
gdir,loc=map(Path,sys.argv[1:3]);s=(gdir/'score.py').read_text();exec(s[:s.rindex('print(json.dumps')])
for g in ['1-authored','0-dry','2-dry']:
 for st in ['stems','pre','final']:assert (loc/f'0-{g}.{st}').read_bytes()==(loc/f'1-{g}.{st}').read_bytes()
assert (loc/'0-1-dry.final').read_bytes()==(gdir/'suffix-1-1-dry.final').read_bytes()
xs=[np.fromfile(loc/f'{k}-1-dry.final','<f4').reshape(-1,2).T for k in [0,1]]; ref=clips['dry']
r={}
def env(a,b):
 n=len(a)//128*128
 q=np.linalg.norm(a[:n].astype(float).reshape(-1,128),axis=1)
 y=np.linalg.norm(b[:n].astype(float).reshape(-1,128),axis=1)
 m=q>max(q.max()/50,1e-5)
 return float(np.sqrt(np.mean((20*np.log10(np.maximum(y[m]/q[m],1e-24)))**2))) if m.any() else None
for title,(i,j) in W.items():
 r[title]=[list({'reference':calc(x,y),'envelope_rms_error_128frame_dB':env(x,y)} for x,y in zip(ref[:,i:j],q[:,i:j])) for q in xs]
r['typed_pcm_hashes']={q.name:hashlib.sha256(q.read_bytes()).hexdigest() for q in sorted(loc.glob('*.final'))}
print(json.dumps(r,indent=2,allow_nan=False))
