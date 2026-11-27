import ctypes as C, numpy as np, hashlib, sys, json
from pathlib import Path
import emu,macho
from unicorn.arm64_const import UC_CPU_ARM64_MAX
macho.EXPECTED_SHA='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
old=emu.Uc
def mk(*a,**k):u=old(*a,**k);u.ctl_set_cpu_model(UC_CPU_ARM64_MAX);return u
emu.Uc=mk
P=Path(sys.argv[1]); n=24576
F=C.CDLL(str(P/'c.so')).render_room; ptr=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
F.argtypes=[C.c_uint,C.c_uint,C.c_uint,ptr,ptr,C.c_uint];F.restype=C.c_float
def digest(x): return hashlib.sha256(x.astype('<f4').tobytes()).hexdigest()
def stats(y,x):
 t=np.flatnonzero(np.any(y!=0,axis=0)); assert len(t)>0
 peak=np.max(np.abs(y));E=np.abs(np.fft.rfft(y,axis=1))**2
 freq=np.fft.rfftfreq(n,1/44100)
 return dict(first_nonzero=int(t[0]),peak=float(peak),signed_sha=digest(y),
  rms_by_window=[[float(np.sqrt(np.mean(row[a:b].astype('f8')**2))) for a,b in [(0,4096),(4096,12288),(12288,n)]] for row in y],
  energy_8k_fraction=float(E[:,freq>=8000].sum()/E.sum()),energy_above_half_rate_nyquist=float(E[:,freq>=11025].sum()/E.sum()))
A={'_sp_revsc_create':0x1003b3398,'_sp_revsc_init':0x1003b33c4,'_sp_revsc_compute':0x1003b3640,'_sp_revsc_destroy':0x1003b360c}
def original(x, cut, fb):
 e=emu.Emu(str(P/'mac-arm64')); assert all(e.im.sym_by_name[k]==v for k,v in A.items())
 sp,ind,f=e.calloc(256),e.calloc(8),e.calloc(16)
 e.w32(sp+8,44100);e.w32(sp+12,2)
 assert e.call(A['_sp_revsc_create'],ind)==1
 s=e.r64(ind)
 assert e.call(A['_sp_revsc_init'],sp,s)==1
 e.wf(s,fb);e.wf(s+4,cut)
 assert e.r64(s+0x1a8)==98760
 y=np.empty_like(x)
 for i in range(n):
  e.wf(f,float(x[0,i]));e.wf(f+4,float(x[1,i]))
  assert e.call(A['_sp_revsc_compute'],sp,s,f,f+4,f+8,f+12)==1
  y[:,i]=e.rf(f+8),e.rf(f+12)
 e.call(A['_sp_revsc_destroy'],ind)
 assert not e.unknown,e.unknown
 return y
x=np.zeros((2,n),dtype='f4');x[0,0]=.25;x[1,37]=-.125
l,r=x.copy(); fb=F(1,80,0,l,r,n)
assert digest(np.stack((l,r)))==digest(x)
rows={'input_signed_sha':digest(x),'feedback_matched_physical':fb,'frames':n}
results={}
for k,cut,t in (('i',4000.,0),('ii',20000.,1)):
 l,r=x.copy(); f=F(t,80,1,l,r,n); assert f==fb
 c=np.stack((l,r));o=original(x,cut,fb)
 rows[k]={'stock_raw':stats(c,x),'original_raw':stats(o,x),
  'reconstruction_dBr':float(20*np.log10(np.linalg.norm(c.astype('f8')-o)/np.linalg.norm(o.astype('f8'))))}
 results[k]=o
rows['original_damping_alone_residual_dBr']=float(20*np.log10(np.linalg.norm(results['i'].astype('f8')-results['ii'])/np.linalg.norm(results['ii'].astype('f8'))))
close=6144
G=C.CDLL(str(P/"c.so")).render_room_close
G.argtypes=[C.c_uint,ptr,ptr,C.c_uint];G.restype=C.c_float
l,r=x.copy();assert G(close,l,r,n)==fb
y=np.stack((l,r)); assert np.array_equal(y[:,close:],x[:,close:])
l,r=x.copy();assert F(1,80,1,l,r,n)==fb
c=np.stack((l,r)); assert np.array_equal(y[:,:close],c[:,:close])
def rms(z):return float(np.linalg.norm(z.astype("f8"))/np.sqrt(z.size))
rows["gate_close"]={"frame":close,"actual_compact_before_exact":bool(np.array_equal(y[:,:close],c[:,:close])),"actual_stock_wet_tail_stays_open_rms":rms(c[:,close:]-x[:,close:]),"actual_stock_wet_tail_closed_rms":rms(y[:,close:]-x[:,close:]),"original_continued_Costello_with_zero_send_rms":rms(results["ii"][:,close:]),"scope":"Original send gate is static verified as upstream; the exact detached active Costello here never stops and input is already zero."}
print(json.dumps(rows,indent=2))
