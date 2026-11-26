import hashlib,json,math,sys,os
from pathlib import Path
from functools import lru_cache
import numpy as np
from codec import encode,decode
SR=44100; BOUNDS=[4534,25792]; NAMES={62:'Irresistible Forces Pad',151:'BB Kubrick Hall Drone',366:'Deeper Space Pad',459:'Infinite Tension FX'}
for row in [int(n) for n in sys.argv[1:]] or NAMES:
 p=Path(f'{row}.state'); a=np.fromfile(p,'<f4').reshape(-1,8)
 w=np.fromfile(f'{row}.write','<f4').reshape(-1,8)
 assert len(a)==len(w)==264728 and np.array_equal(a[:,4],w[:,4]) and np.array_equal(a[:,6]+a[:,7],w[:,7])
 outputs=[]
 for scheme in ('ieee32-exact','binary16-input-lossy'):
  # The smaller depth comparison reconstructs every *rounded* word only.
  t=np.ascontiguousarray(a[:,:4],dtype=('<f4' if scheme.startswith('ieee') else '<f2'))
  y=t.view('<u4' if scheme.startswith('ieee') else '<u2')
  assert np.isfinite(t).all()
  for block in (64,128):
   for coupling in ('independent','dated-cross-lanes'):
    @lru_cache(None)
    def enc(absolute,lanes):
     v=np.ascontiguousarray(y[absolute:absolute+block,lanes])
     coded=encode(v)
     got=decode(coded,v.shape,v.dtype)
     assert got.shape==v.shape and np.array_equal(v,got)
     return len(coded),len(coded)-4,v.nbytes,int(coded[0]&3)
    for at in (2.,4.,5.,5.5):
     end=int(at*SR)+128
     dt=float(w[end,0])
     spans=[math.ceil(dt*SR*(2 if i<2 else 1))+4 for i in range(4)]
     # Causal window: store the partially written tail separately in raw
     # words, no past-window optimizations based on future content.
     newest=end//block
     if min(end-s for s in spans)<0: continue
     groups=[]
     if coupling=='independent':
      for lane,n in enumerate(spans):
       groups += [(k,(lane,)) for k in range((end-n)//block,newest)]
     else:
      bg=min((end-n)//block for n in spans)
      for k in range(bg,newest):
       lanes=tuple(j for j,s in enumerate(spans) if (end-s)//block<=k)
       groups.append((k,lanes))
     z=[enc(k*block,lanes) for k,lanes in groups]
     live_seed=(end%block)*4*y.dtype.itemsize
     ring_meta=128+4*len(z)+live_seed
     payload=sum(a[1] for a in z)
     total=sum(a[0] for a in z)+ring_meta
     worst=max(a[0] for a in z)
     outputs.append(dict(block=block,source=scheme,coupling=coupling,seconds=at,T=dt,feedback=float(w[end,1]),records=len(z),header_bytes=len(z)*4,index_bytes=len(z)*4,seed_and_state=live_seed+128,source_predictor_seeds_inside_payload=True,payload=payload,total=total,worst_record_bytes=worst,max_worst_including_index=worst+4,uncompressed_group_bytes=sum(a[2] for a in z),exact_depth_word_equality=True,coders={str(i):sum(a[3]==i for a in z) for i in range(3)},fits=[n for n in BOUNDS if total<=n]))
 print(json.dumps(dict(row=row,name=NAMES[row],source_state_sha256=hashlib.sha256(p.read_bytes()).hexdigest(),metrics=outputs)),flush=True)
