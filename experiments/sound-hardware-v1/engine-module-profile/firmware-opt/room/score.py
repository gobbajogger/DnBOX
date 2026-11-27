import numpy as np,hashlib,json,sys
import ctypes as C
from pathlib import Path
D,R=map(Path,sys.argv[1:])
sys.path.insert(0,str(R/'s1-controlled-ab'))
from measure import margin
p=np.ctypeslib.ndpointer(dtype=np.float32,flags='C_CONTIGUOUS')
def sha(x):return hashlib.sha256(np.ascontiguousarray(x).astype('<f4').tobytes()).hexdigest()
def metrics(a,b):
 f=lambda y:np.sqrt(np.mean(y.astype('f8')**2,axis=0))
 d=lambda y:(20*np.log10(np.maximum(y,1e-50))).tolist()
 return dict(whole=margin(a[128:],b[128:]),raw=d(f(a-b)/f(a)),late_raw=d(f(a[-44100:]-b[-44100:])/f(a[-44100:])),late_level=d(f(b[-44100:])/f(a[-44100:])))
root=R/'recovered-sound/s1-room-parity/check'
for i in ('062','113'):
 a=np.fromfile(root/f'{i}-room.tap','<f4').reshape(-1,10)
 x=np.column_stack((a[:,2:4],a[:,8])).astype('f4')
 y=a[:,4:6].copy();assert len(x)==264728
 arms=[]
 for k in range(3):
  lib=C.CDLL(str(D/f'path-{k}.so')); f=lib.run; f.argtypes=[p,p,C.c_uint];f.restype=C.c_uint
  z=np.empty_like(y);bytes=f(x,z,len(x));assert np.isfinite(z).all()
  if k==0:assert np.array_equal(y,z),(i,sha(y),sha(z))
  arms.append(dict(arm=k,history=bytes,output=sha(z),**metrics(y,z)))
 print(json.dumps(dict(program=i,input=sha(x),reference=sha(y),arms=arms)),flush=True)
