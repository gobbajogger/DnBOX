"""One Init stage at the same input: execute the original Normal recurrence."""
import ctypes as C,hashlib,json,math,os,struct,subprocess,tempfile,sys
from pathlib import Path
import numpy as np
import emu
from emu import Emu
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_S1,UC_CPU_ARM64_MAX

d=Path(__file__).resolve().parent
b=Path(os.environ['SJ6_BIN'])
assert hashlib.sha256(b.read_bytes()).hexdigest()=='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
ctor=emu.Uc
def new_uc(*a,**k):
 u=ctor(*a,**k);u.ctl_set_cpu_model(UC_CPU_ARM64_MAX);return u
emu.Uc=new_uc
e=Emu(str(b))
ptr=e.calloc(0x600); p=ptr+0x220
e.call(0x1003754fc,p)
e.call(0x10037556c,p,doubles=(44100.,))
end=0x1003a8340
e.uc.hook_add(UC_HOOK_CODE,lambda *_:e.uc.emu_stop(),begin=end,end=end)
def words(a):
 return np.ascontiguousarray(a,dtype='<f4').view('<u4')
def score(ref,out):
 return float(20*np.log10(max(np.linalg.norm(out.astype('f8')-ref),1.e-70)/np.linalg.norm(ref)))
def wave(length,hz):
 t=np.arange(length,dtype='f8')/44100.
 x=np.zeros(length,dtype='f8')
 # Band-limited generated saw, dry, with held note and release.
 for n in range(1,17):
  x+=np.sin(2*np.pi*n*hz*t)/n
 env=np.minimum(t/.008,1)*np.exp(-np.maximum(t-(length/2)/44100,0)/.024)
 return (.28*env*x).astype('f4')
with tempfile.TemporaryDirectory() as td:
 so=Path(td)/'normal.so'
 subprocess.run(['cc','-std=c11','-O2','-fno-fast-math','-ffp-contract=off','-shared','-fPIC',str(d/'normal.c'),'-lm','-o',str(so)],check=True)
 lib=C.CDLL(str(so))
 lib.run_stage.argtypes=[C.POINTER(C.c_double),C.c_float,C.c_float,C.c_uint,C.POINTER(C.c_float),C.POINTER(C.c_float),C.POINTER(C.c_float)]
 def ap(a):return a.ctypes.data_as(C.POINTER(C.c_float))
 rows=[]
 for cutoff,damping,hz in ((4000.,1.,130.81278265),(1200.,.7,261.6255653),(300.,.4043673574924469,65.406391325)):
  e.call(0x10037556c,p,doubles=(44100.,))
  e.call(0x10037558c,p,doubles=(cutoff,damping))
  c=[e.rd_(p+8*i) for i in range(5)]
  x=wave(2048,hz); ref=np.empty_like(x)
  for i,z in enumerate(x):
   e.call(0x1003a82ec,floats=(float(z),),regs={24:ptr})
   ref.view('u4')[i]=e.uc.reg_read(UC_ARM64_REG_S1)
  new=np.empty_like(x); old=np.empty_like(x)
  lib.run_stage((C.c_double*5)(*c),cutoff,damping,len(x),ap(x),ap(new),ap(old))
  assert np.array_equal(words(new),words(ref)),(cutoff,np.count_nonzero(words(new)!=words(ref)))
  assert score(ref,old)>-30.
  rows.append(dict(coeff_hex=[z.hex() for z in c],cutoff_hz=cutoff,original_damping=damping,saw_hz=hz,compared_words=len(x),
                   compact_rms_residual_dbr=score(ref,old),corrected_bit_equal=True,
                   original_signed_sha=hashlib.sha256(ref.tobytes()).hexdigest()))
 assert not e.unknown,e.unknown
if len(sys.argv)>1:
 tap=Path(sys.argv[1])
 a=np.fromfile(tap,dtype='<f4').reshape(-1,2)
 assert a.shape==(1024,2) and .001<np.abs(a[:,0]).max()<1.
 e.call(0x10037556c,p,doubles=(44100.,))
 e.call(0x10037558c,p,doubles=(4000.,1.))
 heard=np.empty(len(a),dtype='f4')
 for i,x in enumerate(a[:,0]):
  e.call(0x1003a82ec,floats=(float(x),),regs={24:ptr})
  heard.view('u4')[i]=e.uc.reg_read(UC_ARM64_REG_S1)
 assert not e.unknown,e.unknown
 assert np.array_equal(words(a[:,1]),words(heard))
 rows.append(dict(runtime_first_note=True,sample_words=len(a),sample_prefix_sha=hashlib.sha256(a.tobytes()).hexdigest(),corrected_bit_equal=True))
print(json.dumps(dict(scope='Actual old ARM64 IIR instructions on matching signed dry Init-like saw input. Portable reference with original coefficient words; compact same-input two-pole, no gain fitting. This compares IIR stage only; original oscillator and AU presets not rendered.',cases=rows),indent=2))
