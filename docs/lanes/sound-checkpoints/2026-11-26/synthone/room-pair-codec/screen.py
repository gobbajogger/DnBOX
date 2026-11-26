from pathlib import Path
import sys,json,hashlib,numpy as np
p,t=map(Path,sys.argv[1:])
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'s1-controlled-ab'))
from measure import margin
r=np.fromfile(p,'<f4').reshape(-1,10)[128:,4:6]
def resid(r,y):
 a,b=r.astype('f8'),y.astype('f8');return (10*np.log10(np.mean((a-b)**2,axis=0)/np.mean(a*a,axis=0))).tolist()
o={}
for a in sorted(t.glob('*.json')):
 b=a.with_suffix('.raw'); y=np.fromfile(b,'<f4').reshape(-1,2)[128:]
 assert r.shape==y.shape and np.isfinite(y).all()
 o[a.stem]={'storage':json.loads(a.read_text()),'formal':margin(r,y),'whole_per_channel_dbr':resid(r,y),'tail_per_channel_dbr':resid(r[-44100:],y[-44100:]),'pcm':hashlib.sha256(b.read_bytes()).hexdigest()}
print(json.dumps(o,indent=2,allow_nan=False))
