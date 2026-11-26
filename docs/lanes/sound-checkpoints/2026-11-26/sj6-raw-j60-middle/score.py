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
 if kind=='direct':
    # Clamp only wet mix seen by middle, not d8 of wrapper which keeps
    # actual engaged dry/96k conversion running. BBD state still warms.
    e.uc.hook_add(UC_HOOK_CODE,lambda *_:e.setd(4,0.),begin=0x10000518c,end=0x10000518c)
 ret=np.empty_like(signal); at=0
 for n in parts:
    ret[:,at:at+n]=j.process(signal[:,at:at+n]); at+=n
 assert at==signal.shape[1] and not e.unknown and np.isfinite(ret).all()
 return (ret,middle_in,middle_out) if kind=="tap" else ret
oldoff=render(1,False); assert np.array_equal(a,oldoff)
oldon,middle_in,middle_out=render(1,True,'tap')
direct=render(1,True,'direct')
assert len(middle_in)==len(middle_out) and abs(len(middle_in)-148684*96000/44100)<3
assert np.isfinite(direct).all() and not np.array_equal(direct,a)
compact_dry=np.float32(1.3)*a
dry_only_hybrid=np.float32(c-compact_dry+direct)
assert np.isfinite(dry_only_hybrid).all()
x=np.array(middle_in,dtype='f4').T.copy()
y96=x.copy()
s96=np.array(middle_out,dtype='f4').T.copy()
lib=ct.CDLL(str(o/'m.so'));lib.witness_middle.argtypes=[ct.c_void_p,ct.c_void_p,ct.c_uint]
lib.witness_middle(y96[0].ctypes.data,y96[1].ctypes.data,y96.shape[1])
assert np.isfinite(y96).all()
def middle_metric(label,lo,hi):
    i,j=round(lo*96000/44100),round(hi*96000/44100)
    # Same 96k F32 input to both middle blocks; no downconversion in score.
    nn=s96[:,i:j].astype('f8'); cc=y96[:,i:j].astype('f8')
    n=np.linalg.norm
    return dict(source=label,entered=[i,j],signed_dbr=float(20*np.log10(n(cc-nn)/n(nn))),
           delta_level_db=float(20*np.log10(n(cc)/n(nn))))

variants={'same_input_orig_j60':oldon*gain,'same_input_typed_j60':y,'original_direct_only_typed_middle_sensitivity':dry_only_hybrid*gain}
def calc(z,d):
 zz=z.astype('f8')-d
 return ((zz[0]+zz[1])*.5,(zz[0]-zz[1])*.5)
def rms(x):return float(np.sqrt(np.mean(x*x)))
def db(u):return float(20*np.log10(u)) if u else None
rows={}
for k,l,r in [('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584),('full',16384,148684)]:
 native=calc(refs['authored'][:,l:r],refs['dry'][:,l:r]); nd=rms(refs['dry'][:,l:r].astype('f8').mean(axis=0))
 inputs={p:calc(v[:,l:r],coff[:,l:r]) for p,v in variants.items()}
 rows[k]={'native_terminal_context_side_mid':db(rms(native[1])/rms(native[0])),
      'native_terminal_context_relative_to_own_dry_mid':[db(rms(v)/nd) for v in native],
      'same_input_compact_vs_orig_j60_signed_dbr':[db(rms(x-z)/rms(z)) for x,z in zip(inputs['same_input_typed_j60'],inputs['same_input_orig_j60'])],
      'same_input':{p:{'delta_side_to_mid_db':db(rms(x[1])/rms(x[0])),
      'delta_relative_to_own_dry_mid_db':[db(rms(v)/rms(coff[:,l:r].astype('f8').mean(axis=0))) for v in x]} for p,x in inputs.items()}}
# Isolated original direct-branch impulse. Use a single frozen operating
# phase; never optimize delay or gain on Raw. Other input phases are blind.
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
prior={}
for at in where[1:]:
    w=g[0,at:at+256].astype('f8')*4
    prior[str(at%147)]=db(rms(w-k)/rms(w))
# Full impulse support (frozen 256), plus a distinct cheap 9-tap candidate
# whose placement only follows the dominating index in the source impulse.
peak=int(np.argmax(abs(k))); left=max(0,peak-4);right=peak+5
short=np.zeros_like(k);short[left:right]=k[left:right]
resamp={}
for name,tap in [('source_one_phase_full',k),('source_one_phase_9tap',short)]:
 predicted=lfilter(tap,[1.],a.astype('f8'),axis=1)
 diagnostic=np.float32(c-compact_dry+predicted)*gain
 resamp[name]={}
 for label,l,r in [('held',20794,38434),('chord',53869,82534),('tail',82534,104584)]:
    mid,side=calc(diagnostic[:,l:r],coff[:,l:r])
    rmid=coff[:,l:r].astype('f8').mean(axis=0)
    resamp[name][label]={'direct_vs_actual_signed_dbr':db(rms(predicted[:,l:r]-direct[:,l:r])/rms(direct[:,l:r])),
      'hybrid_delta_mid_db':db(rms(mid)/rms(rmid)),'hybrid_delta_side_mid_db':db(rms(side)/rms(mid))}
print(json.dumps(dict(sha_oracle=sha(src.read_bytes()),pre_j60_sha=sha(a.tobytes()),route='actual compact pre-J60 frames and equal partition, authentic older app effect with its own 96k bridge, stopped equals input; newer installed-AU terminal context only because same native pre-J60 input not exposed',windows=rows,middle=[middle_metric(w,l,r) for w,l,r in [('held',20794,38434),('chord',53869,82534),('tail',82534,104584)]],inside_count=len(middle_in),source_impulse_model=dict(linear_half_input_word_exact=True,coefficients_not_exported=True, fixed_support_9tap=[left,right], blind_impulse_phase_dbr=prior, measured=resamp)),indent=2,allow_nan=False))
