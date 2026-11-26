import ctypes as C, numpy as np, sys
from pathlib import Path
env={}; p=Path('sj6-current-faders/probe.py');exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'),env)
em=env['e']; co=(C.c_float*5)(*[em.rf(a) for a in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)])
p=(C.c_float*4)(.31277143955230713,.00035606283927336335,1,1)
fp=C.POINTER(C.c_float); c=C.CDLL(sys.argv[1])
c.admit.argtypes=[fp,fp,C.c_float,fp,fp,C.c_uint,C.c_uint,C.c_float]; c.admit.restype=C.c_float
c.physical_replay.argtypes=[fp]*4+[C.c_uint]
a=(C.c_float*55)(); pole=(C.c_float*24)()
rng=np.random.default_rng(631); out=0; misses=0
for k in range(6):
 x=np.ascontiguousarray(rng.uniform(-1,1,size=4096),dtype='f4')
 ptr=C.cast(C.byref(pole,k*16),fp); out_nom=np.empty_like(x)
 c.physical_replay(x.ctypes.data_as(fp),out_nom.ctypes.data_as(fp),p,co,4096)
 a0=[bytes(a)[j*36:(j+1)*36] for j in range(6)]
 p0=[bytes(pole)[j*16:(j+1)*16] for j in range(6)]
 for j,z in enumerate(x):
  if j%27==0:
   before=(bytes(a),bytes(pole))
   o=c.admit(a,ptr,z,p,co,0,k,-0.);assert np.signbit(o) and o==0 and before==(bytes(a),bytes(pole));misses+=1
  o=c.admit(a,ptr,z,p,co,1<<k,k,-0.); assert np.float32(o).tobytes()==out_nom[j].tobytes();out+=1
 for j in range(6):
  if j!=k: assert bytes(a)[j*36:(j+1)*36]==a0[j] and bytes(pole)[j*16:(j+1)*16]==p0[j]
 # Only fresh (cold) reset. No unobserved switch/retime of fixed coefficients.
 C.memset(C.byref(a,36*k),0,36); C.memset(ptr,0,16)
 assert bytes(a)[k*36:(k+1)*36]==a0[k] and bytes(pole)[k*16:(k+1)*16]==p0[k]
print(f'{out} split/contiguous exact output words; {misses} rejected admissions did not mutate state; six distinct cold resets preserve unrelated slots')
