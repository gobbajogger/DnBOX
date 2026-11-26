from emu import Emu
from pathlib import Path
from hashlib import sha256
import numpy as np
import ctypes as C
import paths,json
sha=lambda b:sha256(b).hexdigest()
h=sha(Path(paths.BIN).read_bytes());assert h=='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
e=Emu(); math=C.CDLL('libm.so.6');fm=math.fmaf
def static_name():
 dst,mode,limit,ptr=[e.x(i) for i in range(4)]
 value=e.cstr(ptr).encode();assert mode==0 and len(value)<limit and b"%" not in value
 e.wr(dst,value+b"\0");e.setx(0,len(value))
e.handlers["___sprintf_chk"]=static_name
fm.argtypes=(C.c_float,C.c_float,C.c_float);fm.restype=C.c_float
n=513
x=np.zeros((2,n),dtype='<f4')
x[0,3:171]=.73
x[1,9:141]=-.45
x[0,171:216]=np.linspace(-1.9,1.9,45,dtype=np.float32)
x[1,171:216]=np.linspace(1.9,-1.2,45,dtype=np.float32)
x[0,216:330]=.13
x[1,330:]=(-1.)**np.arange(n-330)*.7
def arm(fs,pole):
 p=e.call(0x10038f818,2,doubles=(float(fs),))
 if pole is not None:e.call(0x10038f880,p,floats=(pole,))
 allocs=[e.calloc(0x40),e.calloc(0x40)]
 ibuf=[e.calloc(4*n),e.calloc(4*n)];obuf=[e.calloc(4*n),e.calloc(4*n)]
 for head,buf in zip(allocs,(ibuf,obuf)):
  e.w32(head,2)
  for i,j in enumerate(buf):
   e.w32(head+8+i*16,1);e.w32(head+12+i*16,n*4);e.w64(head+16+i*16,j)
 for j in range(2):e.wr(ibuf[j],x[j].tobytes())
 e.w64(p+24,allocs[0]);e.w64(p+32,allocs[1])
 state=e.r64(p+64)
 selected=[e.rf(e.r64(state+i*8)+12) for i in range(2)]
 assert selected[0]==selected[1]
 e.wr(p+41,b'\1')
 for l,r,active in ((0,123,True),(123,147,True),(147,185,False),(185,513,True)):
  e.wr(p+41,bytes([active]));e.call(0x10038fad8,p,r-l,l)
 out=np.stack([np.frombuffer(e.rd(k,n*4),dtype='<f4') for k in obuf])
 expected=np.empty_like(x)
 for ch in range(2):
  lastX=lastY=0.
  for j,z in enumerate(x[ch]):
   if 147<=j<185:expected[ch,j]=z;continue
   lastY=fm(selected[ch],lastY,np.float32(z-lastX))
   expected[ch,j]=lastY
   lastX=z
 assert expected.tobytes()==out.tobytes(),(fs,pole,selected,np.argwhere(expected!=out)[:6].tolist(),np.max(np.abs(expected-out)))
 w={}
 db=lambda a,b:float(20*np.log10(np.linalg.norm(a.astype(float))/np.linalg.norm(b.astype(float))))
 for name,i,j in [('step',3,100),('bypass',147,185),('alternation',330,513)]:
  w[name]={'raw_relative_db':db(out[:,i:j],x[:,i:j]),'peak_out':float(np.max(np.abs(out[:,i:j])))}
 return dict(rate=fs,requested_pole=pole,real_state_pole=selected,actual_output_sha256=sha(out.tobytes()),float32_fma_expected_exact_samples=out.size,windows=w)
out=[arm(rate,pole) for rate in (44100,48000) for pole in (None,.999)]
print(json.dumps(dict(original_sha=h,known_input_sha=sha(x.tobytes()),scope='Original old ARM64 terminal DC DSP only; real DSP ctor/register/process; active flags imposed; original AU full graph is not offered',arms=out),indent=2))
