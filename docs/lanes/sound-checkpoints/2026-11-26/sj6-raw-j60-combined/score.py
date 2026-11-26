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
 middle_in=[]; middle_out=[]
 if kind=='tap':
    incoming=lambda *_:middle_in.append((e.s(0),e.s(1)))
    outgoing=lambda *_:middle_out.append((e.s(0),e.s(1)))
    e.uc.hook_add(UC_HOOK_CODE,incoming,begin=0x10000518c,end=0x10000518c)
    e.uc.hook_add(UC_HOOK_CODE,outgoing,begin=0x10000545c,end=0x10000545c)
 if kind=='wet':
    e.uc.hook_add(UC_HOOK_CODE,lambda *_:e.setx(9,0),begin=0x100005440,end=0x100005440)
 if kind=='direct':
    # Clamp only wet mix seen by middle, not d8 of wrapper which keeps
    # actual engaged dry/96k conversion running. BBD state still warms.
    e.uc.hook_add(UC_HOOK_CODE,lambda *_:e.setd(4,0.),begin=0x10000518c,end=0x10000518c)
 ret=np.empty_like(signal); at=0
 for n in parts:
    ret[:,at:at+n]=j.process(signal[:,at:at+n]); at+=n
 assert at==signal.shape[1] and not e.unknown and np.isfinite(ret).all()
 return (ret,middle_in,middle_out) if kind=="tap" else ret
from scipy.signal import lfilter
test_n=8192
pulse=np.zeros((2,test_n),dtype='f4')
where=[1024,1591,2147,2828,3411,4000,4483,5048,5759,6388]
pulse[:,where]=.25
sizes=[128]*(test_n//128)
g=render(1,True,'direct',pulse,sizes)
g2=render(1,True,'direct',pulse*.5,sizes)
assert np.array_equal(g2,g*.5)
k=g[0,where[0]:where[0]+256].astype('f8')*4
assert np.array_equal(g[0],g[1])
# Full impulse support (frozen 256), plus a distinct cheap 9-tap candidate
# whose placement only follows the dominating index in the source impulse.
peak=int(np.argmax(abs(k))); left=max(0,peak-4);right=peak+5
short=np.zeros_like(k);short[left:right]=k[left:right]
# Predeclared -55 dBr impulse-energy budget selects minimum support.
ordr=np.argsort(-abs(k),kind='stable')
remaining=np.cumsum((k[ordr][::-1]**2))[::-1]
cap=np.sum(k*k)*10**(-55/10)
nt=next(i for i,v in enumerate(remaining) if v<=cap)
assert nt>0 and remaining[nt-1]>cap
sparse=np.zeros_like(k); sparse[ordr[:nt]]=k[ordr[:nt]]
assert len(ordr[:nt])==43 and int(np.max(ordr[:nt]))==71
assert np.array_equal(render(1,False),a)
old_full=render(1,True)
old_direct=render(1,True,'direct')
# Clamp the original middle's dry register only. Remaining wet branch
# retains real original conversion, mix, feedback and partition cadence.
old_wet=render(1,True,"wet")
stock,post=(a.copy(),a.copy())
for name,val in (('stock',stock),('post',post)):
 lib=ct.CDLL(str(o/f'{name}.so'))
 fun=lib.witness_connected
 fun.argtypes=[ct.c_void_p,ct.c_void_p,ct.c_void_p,ct.c_uint]
 fun(val[0].ctypes.data,val[1].ctypes.data,ns.ctypes.data,len(ns))
assert np.array_equal(stock,c)
compact_direct=np.float32(1.3)*a
sparse_direct=lfilter(sparse,[1.],a.astype('f8'),axis=1)
sw,pw=stock-compact_direct,post-compact_direct
arms={'compact':stock,'scalar_only':post,
 'sparse_direct_only':np.float32(sw+sparse_direct),'combined':np.float32(pw+sparse_direct),
 'old_full_oracle':old_full}
def db(x,y):
 n,d=np.linalg.norm(x),np.linalg.norm(y)
 return float(20*np.log10(n/d)) if n and d else None
def mid(x):return x.mean(axis=0)
def sd(x):return (x[0]-x[1])*.5
wins={}
for name,l,r in [('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584),('full',16384,148684)]:
 z=lambda f:f[:,l:r].astype('f8')
 o0,od,ow=z(old_full),z(old_direct),z(old_wet)
 n,nd=z(refs['authored']),z(refs['dry'])
 row={'old_bridge_superposition_dbr':db(od+ow-o0,o0),
   'sparse_direct_error_dbr':db(z(sparse_direct)-od,od),
   'stock_wet_error_dbr':db(z(sw)-ow,ow),'post_wet_error_dbr':db(z(pw)-ow,ow),
   'native_context_delta_mid_db':db(mid(n-nd),mid(nd)),'arms':{}}
 for label,can in arms.items():
    v=z(can); t=z(can*gain)
    row['arms'][label]=dict(same_cue_stereo_dbr=db(v-o0,o0),same_cue_mid_dbr=db(mid(v-o0),mid(o0)),
      same_cue_side_dbr=db(sd(v-o0),sd(o0)),context_delta_mid_db=db(mid(t-z(coff)),mid(z(coff))),
      current_native_terminal_dbr=db(t-n,n))
 wins[name]=row
print(json.dumps(dict(cue_sha=sha(a.tobytes()),schedule_sha=sha(ns.tobytes()),stock_exact=True,scope="offline admitted active J60 I only; no shared-mode dispatch edits, other modes are untouched by construction (not tested)",sparse_count=nt,sparse_max_index=int(np.max(ordr[:nt])),coefficients_persisted=False,windows=wins),indent=2,allow_nan=False))
