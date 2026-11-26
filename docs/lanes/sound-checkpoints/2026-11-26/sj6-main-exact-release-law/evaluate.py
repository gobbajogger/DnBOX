import ctypes as C, sys,json,hashlib,tarfile,io,numpy as np
from pathlib import Path
o=Path(sys.argv[1]); x=Path('sj6-current-faders/probe.py'); ns={};exec(compile(x.read_text().split('cases={}')[0],str(x),'exec'),ns);e=ns['e']
co=(C.c_float*5)(*[e.rf(t) for t in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)])
p=(C.c_float*4)(*json.load(open('sj6-raw-main-ota/physical.json'))['physical_controls'][0])
dtype=np.dtype([(k,'<u4') for k in ('frame','key','slot','gate','stage')]+[(k,'<f4') for k in ('amp','sub','mix')])
h=lambda x:hashlib.sha256(x).hexdigest()
def render(k,arm):
 d=Path(f'sj6-cross-preset/sub-range.private-output/2-{k}.private.txt')
 sp=o/f'{k}-{arm}.so';sp.write_bytes((o/'combined.so').read_bytes())
 lib=C.CDLL(str(sp));lib.select_ota.argtypes=[C.c_uint,C.c_void_p,C.c_void_p];lib.select_ota(arm,p,co)
 lib.run_render.argtypes=[C.c_int,C.POINTER(C.c_char_p)]
 f=o/f'{k}-{arm}.f32';a=(C.c_char_p*3)(b'probe',str(d).encode(),str(f).encode())
 assert lib.run_render(3,a)==0
 pcm=np.fromfile(f,'<f4').reshape(-1,2).T.copy();f.unlink();sp.unlink()
 lib.release_trace.restype=C.c_void_p;n=lib.release_trace_count();assert n==123675
 z=np.frombuffer(C.string_at(lib.release_trace(),n*dtype.itemsize),dtype=dtype)
 return pcm,z
rows={}
with tarfile.open('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz') as a:
 for k in ('dry','authored'):
  q=json.load(open('sj6-ota-history/lease-connected/verify.json'))['lease_vs_prior'][k]
  baseline,t0=render(k,3); b,t1=render(k,4)
  assert h(baseline.tobytes())==q['candidate_pcm_sha']
  assert np.array_equal(t0['sub'],t1['sub']) and np.array_equal(t0['frame'],t1['frame'])
  g=t0['gate']>0; assert np.array_equal(t0['amp'][g],t1['amp'][g])
  ref=np.load(io.BytesIO(a.extractfile(f'./program-2-{k}/output.npy').read()),allow_pickle=False)
  res={}
  def db(n,den):
   if not np.any(n):return None
   return 20*float(np.log10(np.linalg.norm(n)/np.linalg.norm(den)))
  for name,l,r in [('attack',16384,20794),('hold',20794,38434),('owner1',38434,38994),('after1',38994,49459),('second hold',53869,82534),('owner2',82534,83094),('after2',83094,104584),('full',16384,148684)]:
   x,y,z=[v[:,l:r].astype('f8') for v in (baseline,b,ref)];res[name]={'stock_connected':db(x-z,z),'modeled_Main_release':db(y-z,z),'relative':db(y-x,x)}
  notes={}
  for note in np.unique(t1['key']):
   v=t1[(t1['key']==note)&(t1['gate']==0)]
   r=v['amp']!=0
   z=v['amp'][:560].copy()
   notes[int(note)]={'first':int(v['frame'][0]),'last_nonzero':int(v['frame'][r][-1]),'last':int(v['frame'][-1]),'release_amp_hash':h(z.tobytes()),'first_amp':z[:6].tolist(),'last_amp':z[-4:].tolist()}
  rows[k]=dict(notes=notes,stock_Sub_words_exact=len(t1),scores=res)
print(json.dumps(rows,indent=2))
