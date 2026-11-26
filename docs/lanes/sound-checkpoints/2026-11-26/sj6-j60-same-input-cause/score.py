from pathlib import Path
import hashlib,tarfile,sys,io,json,numpy as np
from emu import Emu
from j60 import J60
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
def render(mode,enabled):
 e=Emu(str(src)); j=J60(e,mode,1.,1.,1.,sr=44100)
 assert [j.get(i) for i in range(5)]==[float(mode),1,1,1,0]
 if not enabled:e.wr(j.obj+0x29,b'\0')
 ret=np.empty_like(a); at=0
 for n in ns:
    ret[:,at:at+n]=j.process(a[:,at:at+n]); at+=n
 assert at==a.shape[1] and not e.unknown and np.isfinite(ret).all()
 return ret
oldoff=render(1,False); assert np.array_equal(a,oldoff)
oldon=render(1,True)
variants={'same_input_orig_j60':oldon*gain,'same_input_typed_j60':y}
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
print(json.dumps(dict(sha_oracle=sha(src.read_bytes()),pre_j60_sha=sha(a.tobytes()),route='actual compact pre-J60 frames and equal partition, authentic older app effect with its own 96k bridge, stopped equals input; newer installed-AU terminal context only because same native pre-J60 input not exposed',windows=rows),indent=2,allow_nan=False))
