import sys,json,hashlib
import numpy as np
from pathlib import Path
t=Path(sys.argv[1]);w=(24576,37634)
def raw(k,p,st):return np.fromfile(t/f'{k}-{p}.{st}',dtype='<f4').reshape(148684,-1)
def x(k,p,st):return raw(k,p,st)[slice(*w)].astype(float)
def err(a,b):return float(20*np.log10(max(np.linalg.norm(a-b)/np.linalg.norm(b),1e-30)))
r={}
for k in [0,1]:
 for loc in ['pre','final']:
  y,z,l,m,b=(x(k,p,loc) for p in ['a-full','a-half','b-full','b-half','both'])
  q=64/127
  assert not np.any(raw(k,'a-zero',loc))
  r[f'{k}-{loc}']={'a_64_127_dBr':err(z,q*y),'b_64_127_dBr':err(m,q*l),
    'sum_difference_dBr':err(b,z+m),'peakA':float(abs(y).max()),'peakB':float(abs(l).max())}
for p in ['0-dry','1-dry','2-dry']:
 for q in ['pre','final','stems']:assert (t/f'0-{p}.{q}').read_bytes()==(t/f'1-{p}.{q}').read_bytes()
r['typed_final_sha256']={q.name:hashlib.sha256(q.read_bytes()).hexdigest() for q in sorted(t.glob('*.final'))}
print(json.dumps(r,indent=2))
