from pathlib import Path
import ctypes as c, sys, json,hashlib,tarfile,io
import numpy as np
o,b=map(Path,sys.argv[1:]); sha=lambda q:hashlib.sha256(q).hexdigest()
p=Path('sj6-current-faders/probe.py')
ns={};exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'),ns)
e=ns['e']; coeff=np.array([e.rf(a) for a in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)],dtype='f4')
ph=json.load(open('sj6-raw-main-ota/physical.json')); par=np.array(ph['physical_controls'][0],dtype='f4')
base=json.load(open('sj6-j60-warm-center/result.json'))
arc=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert sha(arc.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
ref={}
with tarfile.open(arc) as f:
 for k in ('dry','authored','authored-repeat'):
  p=f'./program-2-{k}/'; rec=json.load(f.extractfile(p+'capture.json'))
  x=np.load(io.BytesIO(f.extractfile(p+'output.npy').read()),allow_pickle=False)
  assert sha(x.tobytes())==rec['pcm_sha256']==base['original_sha'][k] and rec['active']==rec['after']
  ref[k]=x
assert np.array_equal(ref['authored'],ref['authored-repeat'])
from current import instantiate, NEW
callback=c.CFUNCTYPE(c.c_int,c.POINTER(c.c_float),c.POINTER(c.c_float),c.c_uint,c.c_uint)
def render(arm,k,replace=False):
 mute=False
 cfg=b/f'sub-range.private-output/2-{k}.private.txt'
 assert sha(cfg.read_bytes()) in base['controls_compact_fixture_sha']
 if mute:
  q=cfg.read_text().splitlines()
  i=next(i for i in range(2,2+int(q[1])) if q[i].split()[0]=='1')
  q[i]='1 0';cfg=o/'sub-only.cfg';cfg.write_text('\n'.join(q)+'\n')
 # Unique handles isolate synth and effect owners. Unlink only our temp files.
 so=o/f'{arm}-{k}-{replace}.so';so.write_bytes((o/'combined.so').read_bytes())
 lib=c.CDLL(str(so)); lib.select_ota.argtypes=[c.c_uint,c.c_void_p,c.c_void_p]
 lib.select_ota(arm,par.ctypes.data,coeff.ctypes.data)
 leaf=instantiate(k=='authored') if replace else None
 ins=hashlib.sha256(); count=[]; errs=[]
 @callback
 def probe(l,r,n,mode):
  try:
    assert mode==(2 if k=='authored' else 1)
    ll,rr=(np.ctypeslib.as_array(i,shape=(n,)) for i in (l,r))
    v=np.stack((ll,rr)); ins.update(v.tobytes()); count.append(n)
    if leaf:
     q=leaf.process(v); assert np.isfinite(q).all()
     if k=='dry': assert np.array_equal(v,q)
     ll[:],rr[:]=q
    return int(replace)
  except Exception as err:
   errs.append(repr(err)); return 0
 lib.bind_j60.argtypes=[callback];lib.bind_j60(probe)
 lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
 f=o/'one.f32';argv=(c.c_char_p*3)(b'probe',str(cfg).encode(),str(f).encode())
 assert lib.run_render(3,argv)==0 and not errs,errs
 assert len(count)==1164 and sum(count)==148684
 if leaf: assert not leaf.e.unknown,leaf.e.unknown
 q=np.fromfile(f,'<f4').reshape(-1,2).T.copy();f.unlink();so.unlink()
 assert q.shape==(2,148684) and np.isfinite(q).all()
 ct=lib.ota_frames()
 if arm and not mute:assert ct==123675,ct
 else:assert ct==0,ct
 return q,ins.hexdigest()
arms={(m,k):[render(m,k,on) for on in (False,True)] for m in (0,2) for k in ('dry','authored')}
for key,v in arms.items():
 assert v[0][1]==v[1][1]
 m,k=key
 if k=='dry':assert np.array_equal(v[0][0],v[1][0])
 if m==0:assert sha(v[0][0].tobytes())==base['compact_sha']['center-'+k]
prior=json.load(open('sj6-raw-main-ota/whole.json'))
def metrics(a,b):
 a,b=a.astype('f8'),b.astype('f8');n=np.linalg.norm
 return {'signed_dbr':20*float(np.log10(n(a-b)/n(b))), 'level_db':20*float(np.log10(n(a)/n(b)))}
rows={}
for w,l,r in [('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584),('full',16384,148684)]:
 rows[w]={}
 for (m,k),v in arms.items():
  team=[metrics(q[:,l:r],ref[k][:,l:r]) for q,_ in v]
  a='stock' if not m else 'separate_actual_sum'
  assert max(abs(team[0][t]-x) for t,x in prior['windows'][w][a][k].items()) < 1e-9
  rows[w][f'{a}-{k}']=team
print(json.dumps(dict(original_leaf=NEW,input_sha={f'{m}-{k}':v[0][1] for (m,k),v in arms.items()},each_comparison_same_input=True,inactive_identity_exact=True,scope='Variant only swaps current original J60 leaf, with candidate typed controls and all other typed stages untouched. Paired 0=compact J60,1=current original. Model 0=stock Main OTA,2=physical Main with independently clocked compact Sub.',windows=rows),indent=2))
