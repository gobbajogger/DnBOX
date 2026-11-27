import ctypes as C,numpy as np,hashlib,sys,json
from pathlib import Path
import emu,macho
from unicorn.arm64_const import UC_CPU_ARM64_MAX
macho.EXPECTED_SHA='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
old=emu.Uc
def mk(*a,**k):u=old(*a,**k);u.ctl_set_cpu_model(UC_CPU_ARM64_MAX);return u
emu.Uc=mk
D=Path(sys.argv[1]);n=32768
A={'_sp_revsc_create':0x1003b3398,'_sp_revsc_init':0x1003b33c4,'_sp_revsc_compute':0x1003b3640,'_sp_revsc_destroy':0x1003b360c}
def original(x,cut,fb):
 e=emu.Emu(str(D/'mac-arm64')); assert all(e.im.sym_by_name[k]==v for k,v in A.items())
 sp,ind,f=e.calloc(256),e.calloc(8),e.calloc(16)
 e.w32(sp+8,44100);e.w32(sp+12,2)
 assert e.call(A['_sp_revsc_create'],ind)==1
 s=e.r64(ind);assert e.call(A['_sp_revsc_init'],sp,s)==1
 e.wf(s,float(fb));e.wf(s+4,float(cut));assert e.r64(s+0x1a8)==98760
 y=np.empty_like(x)
 for i in range(n):
  e.wf(f,float(x[0,i]));e.wf(f+4,float(x[1,i]))
  assert e.call(A['_sp_revsc_compute'],sp,s,f,f+4,f+8,f+12)==1
  y[:,i]=e.rf(f+8),e.rf(f+12)
 damp=e.rf(s+0x18)
 e.call(A['_sp_revsc_destroy'],ind);assert not e.unknown,e.unknown
 return y,damp
ptr=np.ctypeslib.ndpointer(dtype=np.float32,flags='C_CONTIGUOUS')
def model(x,div,damp,fb):
 f=C.CDLL(str(D/f'model{div}.so')).run;f.argtypes=[ptr,ptr,C.c_uint,C.c_float,C.c_float];f.restype=C.c_uint
 y=np.empty_like(x);size=f(x,y,n,fb,damp);assert size and (size<=700 or div=='float28') and np.isfinite(y).all()
 return y,size
def dbr(x,y):
 a=float(np.linalg.norm(x));b=float(np.linalg.norm(y))
 return 20*np.log10(a/b) if a and b else None
def digest(x):return hashlib.sha256(x.astype('<f4').tobytes()).hexdigest()
def describe(y):
 t=np.flatnonzero(np.any(y!=0,axis=0))
 z=np.abs(np.fft.rfft(y,axis=1))**2;hz=np.fft.rfftfreq(n,1/44100)
 return dict(first_nonzero=int(t[0]) if len(t) else None,nonzero=int(len(t)),signed_sha=digest(y),
        above_8k=float(z[:,hz>8000].sum()/z.sum()) if z.sum() else None)
i=np.zeros((2,n),'f4');i[0,0]=.25;i[1,37]=-.125
t=np.arange(n);a=(t<7424)
d=np.stack([((t*3023%65536)-32768)/196608.*a,((t*1597%65536)-32768)/196608.*a]).astype('f4')
sec=t/44100.;en=np.minimum(1.,sec/.010)*np.minimum(1.,np.maximum(0.,(.257-sec)/.040))
h=np.stack([(.135*np.sin(261.625*2*np.pi*sec)+.072*np.sin(880.*2*np.pi*sec)+.021*np.sin(12853.*2*np.pi*sec))*en,
 (.12*np.sin(329.627*2*np.pi*sec+.2)+.063*np.sin(1302.2*2*np.pi*sec)+.027*np.sin(9565.*2*np.pi*sec))*en]).astype('f4')
rows=[]
for name,x,cut,fb in [('impulse',i,4000,.7779527902603149),('impulse',i,20000,.7779527902603149),('dense',d,20000,.7779527902603149),('heldout chords + release',h,20000,.92)]:
 ref,damp=original(x,cut,fb);w=[]
 for div in (28,32,'float28'):
  y,s=model(x,div,damp,fb);r=ref.astype('f8');c=y.astype('f8')
  w.append(dict(div=div,bytes=s,**describe(y),raw_residual_dBr=dbr(c-r,r),early_raw_dBr=dbr((c-r)[:,:4096],r[:,:4096]),
          late_raw_dBr=dbr((c-r)[:,16000:],r[:,16000:]),late_level_delta_db=dbr(c[:,16000:],r[:,16000:])))
 rows.append(dict(cue=name,cut=cut,fb=fb,read_original_damping=damp,input_signed_sha=digest(x),original=describe(ref),arms=w))
 print(json.dumps(rows[-1]))
