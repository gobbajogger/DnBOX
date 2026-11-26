from pathlib import Path
import hashlib,tarfile,sys,io,json,numpy as np
from emu import Emu
from j60 import J60
from unicorn import UC_HOOK_CODE
import ctypes as ct
import paths
o,b=map(Path,sys.argv[1:])
src=Path(paths.BIN)
sha=lambda x:hashlib.sha256(x).hexdigest()
assert sha(src.read_bytes())=='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
arc=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert sha(arc.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
gain=np.float32(float.fromhex((o/'route.log').read_text().split()[1]))
q=lambda p:np.fromfile(p,'<f4').reshape(-1,2).T.copy()
a,c,y=[q(o/v) for v in ('tap-0.f32','tap-1.f32','typed-out.f32')]
ns=np.fromfile(o/'blocks.i32',dtype='<u4')
assert np.sum(ns)==148684 and all(ns>0) and all(ns<=128)
assert np.array_equal(c*gain,y)
assert (o/'typed-out.f32').read_bytes()==(b/'sub-range.private-output/2-authored.private.f32').read_bytes()
coff=q(b/'sub-range.private-output/2-dry.private.f32')
assert np.array_equal(a*gain,coff)
refs={}
meta={}
with tarfile.open(arc) as f:
 for r in ('dry','authored','authored-repeat'):
    p=f'./program-2-{r}/'
    x=np.load(io.BytesIO(f.extractfile(p+'output.npy').read()),allow_pickle=False)
    m=json.load(f.extractfile(p+'capture.json'))
    assert m['active']==m['after'] and sha(x.tobytes())==m['pcm_sha256']
    refs[r]=x; meta[r]=m
assert np.array_equal(refs['authored'],refs['authored-repeat'])
read=lambda arm:{i['address']:i['value'] for i in meta[arm]['active']}
wp=read('authored');wd=read('dry')
assert {k for k in wp if wp[k]!=wd[k]}=={'40'} and [wp[k] for k in ('40','41','208','215','216','217')]==[1,0,1,.5,.5,1]
def render(mode,enabled,kind="ordinary",signal=a,parts=ns):
 e=Emu(str(src)); j=J60(e,mode,1.,1.,1.,sr=44100)
 assert [j.get(i) for i in range(5)]==[float(mode),1,1,1,0]
 if not enabled:e.wr(j.obj+0x29,b'\0')
 middle_in=[]; middle_out=[]; pre_bbd=[]
 tr=np.empty((323664,2,7),dtype="f8")
 if kind=='tap':
    incoming=lambda *_:middle_in.append((e.s(0),e.s(1)))
    e.uc.hook_add(UC_HOOK_CODE,lambda *_:pre_bbd.append(e.d(6)),begin=0x100005240,end=0x100005240)
    def read_pos(*_):
      i=len(middle_in)-1;ch=e.x(11);assert ch in (0,1)
      tr[i,ch,0]=e.d(19);tr[i,ch,1]=e.x(9)
    def read_words(*_):
      i=len(middle_in)-1;ch=e.x(11);assert ch in (0,1)
      tr[i,ch,2]=e.d(19)   # fractional coefficient prior to FMA
      tr[i,ch,3]=e.d(20)   # ascending ring base
      tr[i,ch,4]=e.d(21)   # next minus base
      tr[i,ch,5]=e.x(16)  # ascending next word modulo 1024
    def read_value(*_):
      i=len(middle_in)-1;ch=e.x(11);assert ch in (0,1)
      tr[i,ch,6]=e.d(19)   # after interpolation; before feedback
    for pc,fn in ((0x1000053ac,read_pos),(0x1000053d4,read_words),(0x1000053d8,read_value)):
      e.uc.hook_add(UC_HOOK_CODE,fn,begin=pc,end=pc)
    outgoing=lambda *_:middle_out.append((e.s(0),e.s(1)))
    e.uc.hook_add(UC_HOOK_CODE,incoming,begin=0x10000518c,end=0x10000518c)
    e.uc.hook_add(UC_HOOK_CODE,outgoing,begin=0x10000545c,end=0x10000545c)
    # .5440 copies x9 into the two dry multiplier lanes. The actual wet
    # output, internal BBD state and wrapper parameters remain untouched.
    e.uc.hook_add(UC_HOOK_CODE,lambda *_:e.setx(9,0),begin=0x100005440,end=0x100005440)
 if kind=='direct':
    # Clamp only wet mix seen by middle, not d8 of wrapper which keeps
    # actual engaged dry/96k conversion running. BBD state still warms.
    e.uc.hook_add(UC_HOOK_CODE,lambda *_:e.setd(4,0.),begin=0x10000518c,end=0x10000518c)
 ret=np.empty_like(signal); at=0
 for n in parts:
    ret[:,at:at+n]=j.process(signal[:,at:at+n]); at+=n
 assert at==signal.shape[1] and not e.unknown and np.isfinite(ret).all()
 return (ret,middle_in,middle_out,pre_bbd,tr) if kind=="tap" else ret
_,xi,yo,hp_ref,tr=render(1,True,'tap')
assert len(xi)==len(yo)==323664 and len(hp_ref)==2*len(xi)
x=np.asarray(xi,'f4').T.copy()
y=np.asarray(yo,'f4').T.copy()
hp_ref=np.asarray(hp_ref,'f8').reshape(-1,2).T.copy()
del xi,yo
t=np.ascontiguousarray(tr.transpose(1,0,2))
w=t[:,:,1]
assert np.array_equal(w,np.broadcast_to(np.arange(323664)%1024,w.shape))
pos=t[:,:,0]
frac=t[:,:,2]
assert np.array_equal(frac,pos-np.floor(pos))
assert np.array_equal(t[:,:,5],(np.floor(pos)+1)%1024)
hist=np.zeros((2,323664+1024),dtype='f8');hist[:,1024:]=hp_ref
abs_age=(w-np.floor(pos))%1024
base=np.take_along_axis(hist,1024+np.arange(w.shape[1])[None,:]-abs_age.astype('i8'),axis=1)
nxt=np.take_along_axis(hist,1024+np.arange(w.shape[1])[None,:]-(abs_age-1).astype('i8'),axis=1)
assert np.array_equal(base,t[:,:,3]) and np.array_equal(nxt-base,t[:,:,4])
lib=ct.CDLL(str(o/'m.so')); lib.witness.argtypes=[ct.c_void_p]*6+[ct.c_uint,ct.c_int]
def db(n,d):return float(20*np.log10(np.linalg.norm(n)/np.linalg.norm(d))) if np.any(n) else None
def window(y,l,r):
 i,j=round(l*96000/44100),round(r*96000/44100)
 return y[:,i:j].astype('f8')
results={}
for m,label in ((0,'compact_trajectory_exact_history'),(1,'original_pos_exact_history'),(2,'original_read_exact_value'),(3,'source_right_multiplier_only'),(4,'stock_Q12_corrected_post'),(5,'source_right_multiplier_only_Q12')):
 cand=x.copy(); cp=np.empty((2,x.shape[1],5),dtype='f8')
 lib.witness(cand[0].ctypes.data,cand[1].ctypes.data,hp_ref.ctypes.data,t.ctypes.data,cp[0].ctypes.data,cp[1].ctypes.data,cand.shape[1],m)
 p=cp[:,:,0]; cfrac=cp[:,:,1]; v=cp[:,:,4]
 rows={}
 for name,l,r in [('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584)]:
  nn=window(y,l,r);cc=window(cand,l,r)
  vr=window(t[:,:,6],l,r);vv=window(v,l,r)
  d=window(((p-pos+512)%1024)-512,l,r)
  rows[name]={'wet_signed_dbr':db(cc-nn,nn),'read_signed_dbr':db(vv-vr,vr),'position_offset_samples_mean':np.mean(d,axis=1).tolist(),'position_offset_samples_maxabs':np.max(abs(d),axis=1).tolist()}
 if m==1:
  assert np.array_equal(v,t[:,:,6])
  assert np.array_equal(cp[:,:,2],base) and np.array_equal(cp[:,:,3]-base,t[:,:,4])
 if m==0:
  diff=((p-pos+512)%1024)-512
  where=next(i for i in range(x.shape[1]-256) if np.all(np.abs(diff[:,i:i+256])>.01))
  offset={'first_both_channels_256_steps_abs_position_gt_0p01':where,'signed_deltas_at_first':diff[:,where].tolist(),
   'source_position':pos[:,where].tolist(),'compact_position':p[:,where].tolist()}
 results[label]=rows
print(json.dumps({'same_96k_frames':x.shape[1],'same_input_sha':sha(x.tobytes()),'real_source_write_history_sha':sha(hp_ref.tobytes()),'original_taps_sha':sha(t.tobytes()),'ring_auth_reconstruct_both_words_exact':True,'persistent':offset,'windows':results},allow_nan=False,indent=2))
