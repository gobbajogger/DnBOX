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
def render(arm,k,mute=False):
 cfg=b/f'sub-range.private-output/2-{k}.private.txt'
 assert sha(cfg.read_bytes()) in base['controls_compact_fixture_sha']
 if mute:
  q=cfg.read_text().splitlines()
  i=next(i for i in range(2,2+int(q[1])) if q[i].split()[0]=='1')
  q[i]='1 0';cfg=o/'sub-only.cfg';cfg.write_text('\n'.join(q)+'\n')
 # Unique handles isolate synth and effect owners. Unlink only our temp files.
 so=o/f'{arm}-{k}-{mute}.so';so.write_bytes((o/'combined.so').read_bytes())
 lib=c.CDLL(str(so)); lib.select_ota.argtypes=[c.c_uint,c.c_void_p,c.c_void_p]
 lib.select_ota(arm,par.ctypes.data,coeff.ctypes.data)
 lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
 f=o/'one.f32';argv=(c.c_char_p*3)(b'probe',str(cfg).encode(),str(f).encode())
 assert lib.run_render(3,argv)==0
 q=np.fromfile(f,'<f4').reshape(-1,2).T.copy();f.unlink();so.unlink()
 assert q.shape==(2,148684) and np.isfinite(q).all()
 ct=lib.ota_frames()
 if arm and not mute:assert ct==123675,ct
 else:assert ct==0,ct
 return q
x={k:[render(a,k) for a in range(3)] for k in ('dry','authored')}
for k in x:assert sha(x[k][0].tobytes())==base['compact_sha']['center-'+k]
m=[render(a,'dry',True) for a in range(3)]
assert np.array_equal(m[0],m[1]) and np.array_equal(m[0],m[2])
def db(a,b):return float(20*np.log10(np.linalg.norm(a)/np.linalg.norm(b)))
rows={}
for w,l,r in [('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584),('full',16384,148684)]:
 rows[w]={}
 z=lambda q:q[:,l:r].astype('f8')
 for i,label in enumerate(('stock','marginal','separate_actual_sum')):
  rows[w][label]={k:{'signed_dbr':db(z(v[i])-z(ref[k]),z(ref[k])),'level_db':db(z(v[i]),z(ref[k]))} for k,v in x.items()}
print(json.dumps(dict(baseline_bytes=True,Sub_only_bytes=True,rate=44100,admitted_samples=123675,unchanged_J60="actual typed post+right all arms; not original J60 unit replay",slots=['stock','changes only marginal Main with stock nonlinear cross terms','independent Main+Sub OTA before existing owner amp/part mixer; Main physical recurrence, Sub unchanged compact carrier and legacy OTA law, independently clocked'],windows=rows),indent=2))
