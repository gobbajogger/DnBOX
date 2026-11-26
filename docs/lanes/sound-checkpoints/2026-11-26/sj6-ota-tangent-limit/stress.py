import ctypes as C, sys, json, numpy as np
from pathlib import Path
ns={};p=Path('sj6-current-faders/probe.py');exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'),ns);e=ns['e']
co=np.asarray([e.rf(a) for a in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)],dtype='f4')
lib=[C.CDLL(a) for a in sys.argv[1:]]
for l in lib:l.physical_replay.argtypes=[C.c_void_p]*4+[C.c_uint]
rng=np.random.default_rng(631)
rows=[]
for fs in [32000,44100,48000,96000]:
 k=np.arange(8192); x=.45*np.sin(2*np.pi*129*k/fs)+.34*np.sin(2*np.pi*7391*k/fs)+.15*rng.standard_normal(len(k))
 x/=np.max(np.abs(x))
 for g in [.31277143955230713,1.8208369016647339]:
  p=np.asarray([g,.00035606283927336335,1,1],dtype='f4')
  for pk in [.001,.01,.1,.25,.5,1.,2.,4.,8.,12.,24.,48.]:
   src=(pk*x).astype('f4')
   ys=[np.empty_like(src),np.empty_like(src)]
   for l,y in zip(lib,ys):l.physical_replay(src.ctypes.data,y.ctypes.data,p.ctypes.data,co.ctypes.data,len(src))
   a,b=[y.astype('f8') for y in ys]; err=b-a
   rows.append(dict(rate=fs, g=g, peak=pk, error_dbr=20*float(np.log10(np.linalg.norm(err)/np.linalg.norm(a))), peakerr=float(np.max(np.abs(err))),bias=float(np.mean(err))))
print(json.dumps(rows,indent=1))
