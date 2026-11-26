from pathlib import Path
import numpy as np,ctypes as ct
import hashlib,sys,json
from emu import Emu
from j60 import J60
from unicorn import UC_HOOK_CODE
import paths
a=Path(sys.argv[1])
digest=lambda b:hashlib.sha256(b).hexdigest()
assert digest(Path(paths.BIN).read_bytes())=='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
t=np.arange(38400,dtype='f8')/96000.
x=np.float32([.19*np.sin(2*np.pi*180*t)+.09*np.cos(2*np.pi*1283*t),.12*np.cos(2*np.pi*243*t)-.055*np.sin(2*np.pi*1859*t)])
x[:,:2048]=0;x[:,29000:]=0;x[0,4096]+=.27;x[1,9273]-=.23
assert x.shape==(2,38400)
report={}
for mode in (0,2,3):
 e=Emu(str(paths.BIN));j=J60(e,mode,1.,1.,1.,sr=96000.)
 entered=[];original=[]
 if mode:
  e.uc.hook_add(UC_HOOK_CODE,lambda *_:entered.append((e.s(0),e.s(1))),begin=0x10000518c,end=0x10000518c)
  e.uc.hook_add(UC_HOOK_CODE,lambda *_:e.setx(9,0),begin=0x100005440,end=0x100005440)
  e.uc.hook_add(UC_HOOK_CODE,lambda *_:original.append((e.s(0),e.s(1))),begin=0x10000545c,end=0x10000545c)
 else:e.wr(j.obj+0x29,b'\0')
 out=j.process(x)
 assert not e.unknown and np.isfinite(out).all()
 if mode:
  assert len(entered)==len(original)==x.shape[1]
  p=np.asarray(entered,'f4').T.copy();ref=np.asarray(original,'f4').T.copy()
 else:
  assert np.array_equal(x,out)
  p=x;ref=x
 comp=[]
 for v in (0,1):
  dll=ct.CDLL(str(a/f'fixture-{v}.so')); fn=dll.witness
  fn.argtypes=[ct.c_void_p,ct.c_void_p,ct.c_uint,ct.c_uint]
  y=p.copy();fn(y[0].ctypes.data,y[1].ctypes.data,y.shape[1],mode)
  comp.append(y)
 if not mode:
  assert all(np.array_equal(x,y) for y in comp)
  report['bypass']={'frames_exact':[x.shape[1]]*3,'same_ref_input_sha':digest(x.tobytes())}
  continue
 windows={'attack':[2048,8000],'held':[12000,28000],'tail':[29000,30500]}
 measure=lambda y,r:20*np.log10(np.linalg.norm(y-r)/np.linalg.norm(r))
 report[str(mode)]={'entered_sha':digest(p.tobytes()),'source_wet_sha':digest(ref.tobytes()),'frames':p.shape[1],
  'signed_dbr':{w:[measure(y[:,i:j].astype('f8'),ref[:,i:j].astype('f8')) for y in comp] for w,(i,j) in windows.items()}}
print(json.dumps(report,allow_nan=False,indent=2))
