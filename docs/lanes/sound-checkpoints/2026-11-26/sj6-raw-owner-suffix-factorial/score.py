from pathlib import Path
import ctypes as c, hashlib, io, json, math, re, sys, tarfile, zipfile,struct
import numpy as np
o,b=map(Path,sys.argv[1:])
# Reuse authorized matching sample capsule decoder, no compiled app import.
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('ipa=')[1].split('nat=Path(')[0]
exec(compile('ipa='+s,'existing private SFZ container decoder','exec'))
sha=lambda z:hashlib.sha256(z).hexdigest()
assert sha(data)=='ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740'
assert sha(sfz)=='3668ef83c102b4952a19bc6fd5adc585eb69d0eff4dd4743be9488b0771fc75d'
arc=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert sha(arc.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(arc) as t:
 def take(a):
  p=f'./program-2-{a}/'
  m=json.load(t.extractfile(p+'capture.json'))
  r=np.load(io.BytesIO(t.extractfile(p+'output.npy').read()),allow_pickle=False).copy()
  assert sha(r.tobytes())==m['pcm_sha256'] and m['active']==m['after'] and m['frames']==r.shape[1]==148684
  return r,m
 refs=dict((a,take(a)) for a in ('authored','dry','authored-repeat'))
assert np.array_equal(refs['authored'][0],refs['authored-repeat'][0])
from dose import collect
m=refs['dry'][1]
raw={int(v['address']):v['value'] for v in m['active']}
stems,binary=collect(m,raw,regions,pack)
true_sum=np.float32(stems['Main'][0]+stems['Sub'][0])
assert true_sum.shape==(2,148684) and not true_sum[:,:16384].any()
base=json.load(open('sj6-j60-warm-center/result.json'))
def admit(m):
 v={int(x['address']):x['value'] for x in m['active']}
 required={0:raw[0],11:0,16:0,17:0,22:0,23:0,24:0,28:0,41:0,59:0,63:0,89:0,95:0,99:0,103:0,107:0,108:0,109:0,142:0,201:0,208:1,215:.5,216:.5,217:1}
 return m['rate']==44100 and all(v.get(k)==q for k,q in required.items()) and v.get(40) in (0,1)
for a in ('authored','dry','authored-repeat'):assert admit(refs[a][1])
for k in (28,103,107,108,109):
 import copy
 bad=copy.deepcopy(refs['dry'][1]);next(z for z in bad['active'] if int(z['address'])==k)['value']=.5
 assert not admit(bad)
src_bin=Path('sj6-current-faders/probe.py'); ns={};exec(compile(src_bin.read_text().split('cases={}')[0],str(src_bin),'exec'),ns);e=ns['e']
co=(c.c_float*5)(*[e.rf(t) for t in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)])
p0=(c.c_float*4)(*json.load(open('sj6-raw-main-ota/physical.json'))['physical_controls'][0])
def run(kind,opt):
 lab=int(kind=='connected')*3; take=int(kind=='source')
 cfg=b/'sub-range.private-output/2-dry.private.txt'
 assert sha(cfg.read_bytes()) in base['controls_compact_fixture_sha']
 folder=o/f'{kind}-{opt}.so';folder.write_bytes((o/'combined.so').read_bytes())
 lib=c.CDLL(str(folder));lib.select_ota.argtypes=[c.c_uint,c.c_void_p,c.c_void_p];lib.select_ota(lab,p0,co)
 lib.upstream.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_int];lib.upstream(true_sum.ctypes.data,148684,take,opt)
 lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
 f=o/f'{kind}-{opt}.f32'; argv=(c.c_char_p*3)(b'probe',str(cfg).encode(),str(f).encode())
 assert lib.run_render(3,argv)==0 and lib.reached_frames()==148684
 cs=(c.c_uint*3)();lib.physical_counts(cs); assert list(cs)==[148684*opt]*3
 q=np.fromfile(f,'<f4').reshape(-1,2).T.copy();f.unlink();folder.unlink()
 lib.stage_ptr.restype=c.POINTER(c.c_float)
 st=[np.ctypeslib.as_array(lib.stage_ptr(i),shape=(2*148684,)).reshape(2,148684).copy() for i in range(4)]
 if take:assert st[0].tobytes()==true_sum.tobytes()
 return st+[q]
allr={(k,o):run(k,o) for k in ('stock','connected','source') for o in (0,1)}
for k in ('stock','connected','source'):assert np.array_equal(allr[k,0][0],allr[k,1][0])
assert sha(allr['stock',0][-1].tobytes())==base['compact_sha']['center-dry']
assert sha(allr['connected',0][-1].tobytes())=='5bb4c2aa9bd9ea8a87b2ba505ce93c7e6f756505cd213c2b100515e805713d9c'
strong=json.load(open('sj6-warm-raw-postvoice/result-final.json'))
assert all(sha(stems[x][0].tobytes())==strong['source_roles'][x]['output_sha'] for x in ('Main','Sub'))
def metrics(a,b):
 ar,br=[np.linalg.norm(x) for x in (a,b)];d=np.linalg.norm(a-b)
 db=lambda u: 20*float(np.log10(u/br)) if u and br else None
 return dict(error_dbr=db(d),level_delta=db(ar),residual=float(d),reference_norm=float(br))
names=['pre_HP','after_HP','prechorus_after_Crusher','pre_Dc','terminal']
rows={}
for label,l,r in [('hold',20794,38434),('first_16',38434,38450),('first_560',38434,38994),('after_first',38994,49459),('final_16',82534,82550),('final_560',82534,83094),('after_final',83094,104584)]:
 slice=lambda a:a[:,l:r].astype('f8')
 rows[label]={}
 for o in (0,1):
  exact=allr['source',o]; current=allr['connected',o];stock=allr['stock',o]
  cols={z:dict(connected_vs_source=metrics(slice(y),slice(x)),stock_vs_source=metrics(slice(q),slice(x))) for z,x,y,q in zip(names,exact,current,stock)}
  ref=slice(refs['dry'][0])
  terms={k:metrics(slice(allr[k,o][-1]),ref) for k in ('source','connected','stock')}
  rows[label][o]=dict(conditional_same_stage=cols,terminal_vs_current_official=terms)
print(json.dumps(dict(witnessed_independent_source_sha=sha(true_sum.tobytes()),installed_reference_sha=refs['dry'][1]['pcm_sha256'],stages=names,
 scope='Identical warmed HP/Crusher/terminal state initialization and ordered input chronology; real original compiled voices without unavailable installed AVAudioMixerNode sum tap; different pre-HP input leads to different chronological internal states.',windows=rows),indent=2,allow_nan=False))
