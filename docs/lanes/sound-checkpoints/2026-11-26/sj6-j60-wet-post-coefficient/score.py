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
    def incoming(*_):
      i=len(middle_in)
      middle_in.append((e.s(0),e.s(1)))
      if i in (0,1,2,128,16384,24000,50000,200000):
       import struct
       at=e.x(0)
       sample=dict(i=i,phase_center=struct.unpack('<2f',e.rd(at+0x18,8)),d234=[e.d(k) for k in (2,3,4)],mode=e.x(1),
         post_hp=struct.unpack('<3d',e.rd(at+0x25370,24)),coeffs=struct.unpack('<28d',e.rd(at+0x4028,224)))
       print(json.dumps(sample),file=sys.stderr)
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
 return (ret,middle_in,middle_out) if kind=="tap" else ret
old_wet_bridge,xi,yo=render(1,True,'tap')
assert len(xi)==len(yo)==323664
x=np.asarray(xi,'f4').T.copy()
y=np.asarray(yo,'f4').T.copy()
del xi,yo
def db(n,d):return float(20*np.log10(np.linalg.norm(n)/np.linalg.norm(d)))
def window(y,l,r):
 i,j=round(l*96000/44100),round(r*96000/44100)
 return y[:,i:j].astype('f8')
res={}
for mode in ('stock','phase','delay','phase-delay','post','all'):
 cand=x.copy()
 lib=ct.CDLL(str(o/f'{mode}.so'));lib.witness_middle.argtypes=[ct.c_void_p,ct.c_void_p,ct.c_uint]
 lib.witness_middle(cand[0].ctypes.data,cand[1].ctypes.data,cand.shape[1])
 rows={}
 for name,l,r in [('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584)]:
  nn=window(y,l,r);cc=window(cand,l,r)
  rows[name]={'signed_stereo_dbr':db(cc-nn,nn),'per_channel_dbr':[db(c-n,n) for c,n in zip(cc,nn)],'delta_level_db':db(cc,nn)}
 res[mode]=rows
connected={}
def terminal(x):
    return x*gain
def rms(x): return np.sqrt(np.mean(x*x))
rs=[('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584)]
for mode in ('stock','post'):
 lib=ct.CDLL(str(o/f'{mode}.so'))
 run=lib.witness_connected
 run.argtypes=[ct.c_void_p,ct.c_void_p,ct.c_void_p,ct.c_uint]
 can=a.copy()
 run(can[0].ctypes.data,can[1].ctypes.data,ns.ctypes.data,len(ns))
 if mode=='stock':
    assert np.array_equal(can,c), 'actual connected stock compact exact tap guard'
 wet=(can-np.float32(1.3)*a)
 rows={}
 for name,l,r in rs:
    w=wet[:,l:r].astype('f8');v=old_wet_bridge[:,l:r].astype('f8')
    cap=terminal(can)[:,l:r].astype('f8')
    n=refs['authored'][:,l:r].astype('f8')
    rows[name]={'wet_signed_to_old_same_input_dbr':db(w-v,v),
                'wet_level_delta_db':db(w,v),
                'whole_terminal_current_native_context_dbr':db(cap-n,n)}
 connected[mode]=rows
print(json.dumps({'connected_44100_stock_matches_real_pinned_route_exactly':True,'connected':connected,'same_96k_samples' :x.shape[1],'pre_input_sha':sha(x.tobytes()),'original_wet_sha':sha(y.tobytes()),'original_source_sha':sha(src.read_bytes()),'windows':res},indent=2,allow_nan=False))
