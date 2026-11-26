import numpy as np, ctypes as C, sys, json, hashlib
from pathlib import Path
ns={};p=Path('sj6-current-faders/probe.py');exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'),ns); e=ns['e']
def arr(a):return (C.c_float*len(a))(*a)
co=arr([e.rf(a) for a in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)])
l=[C.CDLL(x) for x in sys.argv[1:]]
for z in l:
 z.tick_owner.argtypes=[C.c_uint,C.c_float,C.POINTER(C.c_float),C.POINTER(C.c_float)]
 z.tick_owner.restype=C.c_float
assert [z.state_bytes() for z in l]==[84,64]
rows={}
rng=np.random.default_rng(76193)
for g in (.31277143955230713,1.8208369016647339):
 for hz in (32000,44100,48000,96000):
  for pk in (1e-5,.5,12.):
   # Distinct source observations supply g. Rate here changes only input density
   # (ordained source model uses g as physical input, not an inferred rate law).
   k=np.arange(4096)
   w=pk*(.45*np.sin(2*np.pi*129*k/hz)+.34*np.sin(2*np.pi*7391*k/hz)+.15*rng.standard_normal(len(k)))
   w[19:27]=[0.,-0.,1e-40,-1e-40,0.,0.,pk,-pk]
   a=arr([g,.00035606283927336335,1,1]); b=arr([g,.00035606283927336335,0,1])
   alt=arr([.31277143955230713 if g>1 else 1.8208369016647339,.00035606283927336335,.35,1])
   y=[[],[]]; n=0
   for j,z in enumerate(l):
    for i in (0,1):z.reset_owner(i)
    # 0→1→0 interleave, skip inactive states exactly; zero-reset owner 0
    # at frame 1536 and leave owner's other branch untouched throughout.
    for k,x in enumerate(w):
     if k==1536:z.reset_owner(0)
     if k%97<41:y[j].append(z.tick_owner(0,float(x),a if k<3072 else alt,co))
     if k%211>73:y[j].append(z.tick_owner(1,float(-x),b,co))
   pair=[np.asarray(x,dtype='<f4') for x in y]
   assert np.isfinite(pair[0]).all() and pair[0].tobytes()==pair[1].tobytes()
   rows[f'{g}:{hz}:{pk}']={'words':len(pair[0]),'sha':hashlib.sha256(pair[0].tobytes()).hexdigest()}
print(json.dumps(dict(original_bytes=84,new_bytes=64,all_raw_words_equal=True,rows=rows),indent=2))
