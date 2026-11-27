"""Read preserved post-Normal Main1/Main2/convergence taps, no audio output."""
import hashlib,json
from pathlib import Path
import numpy as np
r=Path('recovered-sound/main-route/exact-feature')
windows={'attack':(16384,18432),'held':(24576,37634),'release':(38434,38946),'handoff':(49459,49875),'chord':(53971,81434),'final':(82534,83046)}
def score(x):
 a,b,bus=x
 s=np.add(a,b,dtype=np.float32)
 d={}
 for n,(i,j) in windows.items():
  aa,bb,ss,tt=(z[i:j].astype('f8') for z in (a,b,s,bus))
  # Ratios diagnose recorded source math, never applied to the audio.
  strong=np.abs(ss)>1e-6; coeff=tt[strong]/ss[strong]
  d[n]=dict(peak_in=[float(np.max(abs(z))) for z in (aa,bb,ss)],peak_out=float(np.max(abs(tt))),
   distinct_roles=bool(np.any(aa!=bb)),nonzero_pair=int(np.count_nonzero(strong)),
   receiver_ratio_min_max=[float(np.min(coeff)),float(np.max(coeff))] if len(coeff) else None,
   polarity_loss=int(np.count_nonzero(ss*tt<0)))
 return d
res={}
for p in r.glob('*.private.f32'):
 b=p.read_bytes();x=np.frombuffer(b,'<f4').reshape(3,-1)
 assert x.shape==(3,148684) and np.isfinite(x).all()
 res[p.name]=dict(sha=hashlib.sha256(b).hexdigest(),windows=score(x))
print(json.dumps(res,indent=2))
