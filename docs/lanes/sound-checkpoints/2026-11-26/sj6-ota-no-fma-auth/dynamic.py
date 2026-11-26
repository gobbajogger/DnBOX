import ctypes as C, sys, json, numpy as np
from pathlib import Path
p=Path('sj6-current-faders/probe.py'); ns={};exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'),ns);e=ns['e']
co=(C.c_float*5)(*[e.rf(a) for a in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)])
require_equal='--require-equal' in sys.argv[1:]
libs=[C.CDLL(p) for p in sys.argv[1:] if p!='--require-equal'];f=C.POINTER(C.c_float)
for l in libs:l.one.argtypes=[f,C.c_float,f,f];l.one.restype=C.c_float
rng=np.random.default_rng(5133)
rows=[]
for fs in (32000,44100,48000,96000):
 for peak in (.001, .5, 2., 12.):
  k=np.arange(8192);x=(.5*np.sin(k*2*np.pi*224/fs)+.125*rng.standard_normal(8192))*peak
  x[12:20]=[0.,-0.,1e-40,-1e-40,0.,0.,peak,-peak]
  st=[[(C.c_float*13)() for _ in range(2)] for _ in libs];ys=[[] for _ in libs]
  for i,z in enumerate(x):
   if i==3072:
    untouched=[bytes(row[1]) for row in st]
    for row in st:C.memset(row[0],0,52)
    assert untouched==[bytes(row[1]) for row in st]
   for voice in (0,1):
    if (i+voice*11)%17<voice*2+3:continue
    # One g snap/ramp-like changed value per base frame, same through its two substeps.
    p=(C.c_float*4)(.004+1.82*((i+238*voice)%2048)/2048., .00035606283927336335, (1.,0.,.35)[(i//128)%3],1.)
    for l,out,hist in zip(libs,ys,st):out.append(l.one(hist[voice],float(z)*(1 if voice==0 else -1),p,co))
  a,b=[np.asarray(y,dtype='f8') for y in ys];v=b-a
  assert np.isfinite(a).all() and np.isfinite(b).all()
  if require_equal:
    assert not np.any(v) and all(bytes(a)==bytes(b) for a,b in zip(*st))
    rows.append(dict(rate=fs,peak=peak,frames=len(v),exact=True))
  else:
    rows.append(dict(rate=fs,peak=peak,frames=len(v),dbr=20*np.log10(np.linalg.norm(v)/np.linalg.norm(a)),max_error=float(np.max(np.abs(v))),worst_end_state=max(abs(a-b) for v,w in zip(*st) for a,b in zip(v,w))))
print(json.dumps(dict(scope='F32 changing g/blend, immutable exact allpass bank privately resolved, two interleaved owners, freeze/reset',rows=rows),indent=2))
