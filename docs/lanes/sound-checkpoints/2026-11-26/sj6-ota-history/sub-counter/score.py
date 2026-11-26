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
import os
p=Path('sj6-current-faders/probe.py')
s={};exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'),s);e=s['e']
co=(c.c_float*5)(*[e.rf(t) for t in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)])
par=(c.c_float*4)(*json.load(open('sj6-raw-main-ota/physical.json'))['physical_controls'][0])
m=refs['dry'][1];raw={int(v['address']):v['value'] for v in m['active']}
stems,pin=collect(m,raw,regions,pack,sub_only=True)
sub=stems['Sub'][0];assert sub.shape==(2,148684)
assert sha(sub.tobytes())==json.load(open('sj6-warm-raw-postvoice/result-final.json'))['source_roles']['Sub']['output_sha']
assert not sub[:,:16384].any()
def render(k,source):
 fd=os.memfd_create(k);os.write(fd,(o/'combined.so').read_bytes());lib=c.CDLL(f'/proc/self/fd/{fd}')
 lib.select_ota.argtypes=[c.c_uint,c.c_void_p,c.c_void_p];lib.select_ota(4,par,co)
 stock=np.zeros(148684,'f4')
 lib.bind_original_sub.argtypes=[c.c_void_p,c.c_uint,c.c_void_p]
 lib.bind_original_sub(source.ctypes.data if source is not None else None,len(stock),stock.ctypes.data)
 cfg=b/f'sub-range.private-output/2-{k}.private.txt'
 assert sha(cfg.read_bytes()) in json.load(open('sj6-j60-warm-center/result.json'))['controls_compact_fixture_sha']
 lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
 of=os.memfd_create(k+'-pcm');f=f'/proc/self/fd/{of}';argv=(c.c_char_p*3)(b'probe',str(cfg).encode(),f.encode())
 assert lib.run_render(3,argv)==0
 assert lib.audited_sub_frames()==(0 if source is None else 148684)
 pcm=np.fromfile(f,'<f4').reshape(-1,2).T.copy();os.close(of)
 assert pcm.shape==(2,148684) and not pcm[:,:16384].any()
 lib.release_trace.restype=c.c_void_p;n=lib.release_trace_count()
 assert n==123675
 trace=c.string_at(lib.release_trace(),n*32)
 return pcm,stock,sha(trace)
prior=json.load(open('sj6-ota-history/lease-release/amp.json'))
def stats(a,b):
 x,y=(q.astype('f8') for q in (a,b));n=np.linalg.norm
 a,b,c=n(x),n(y),n(x-y)
 assert b>0
 return {
  'signed_dbr':float(20*np.log10(c/b)) if c else None,
  'level_db':float(20*np.log10(a/b)) if a else None,
  'signed_cosine':float(np.vdot(x,y)/a/b) if a else None}
rows={}
for k in ('dry','authored'):
 base,stock,reported=render(k,None)
 cand,guard,tr=render(k,sub)
 source_muted,guard2,tr2=render(k,np.zeros_like(sub))
 assert tr==tr2==reported and np.array_equal(stock,guard) and np.array_equal(stock,guard2)
 ref=refs[k][0]
 wins={}
 for name,l,r in [('attack',16384,20794),('hold',20794,38434),('owner1',38434,38994),('after1',38994,49459),('second hold',53869,82534),('owner2',82534,83094),('after2',83094,104584),('full',16384,148684)]:
  x,y,z=[q[:,l:r] for q in (base,cand,ref)]
  a,b1=stats(x,z),stats(y,z)
  assert abs(a['signed_dbr']-prior[k]['scores'][name]['modeled_Main_release'])<1e-9
  u=stock[l:r];v=sub[0,l:r]
  stage=stats(u,v) if np.any(u) and np.any(v) else dict(stock_active=bool(np.any(u)),genuine_active=bool(np.any(v)))
  wins[name]=dict(mixed_terminal={'stock':a,'independent_actual_sub':b1,'sub_muted':stats(source_muted[:,l:r],z)},
                  true_vs_stock_sub_at_mix=stage)
 rows[k]=dict(windows=wins,stock_sub_hash=sha(stock.tobytes()),main_and_stock_owner_trace=tr,mix_sha=[sha(q.tobytes()) for q in (base,cand,source_muted)])
assert rows['dry']['stock_sub_hash']==rows['authored']['stock_sub_hash']
assert rows['dry']['main_and_stock_owner_trace']==rows['authored']['main_and_stock_owner_trace']
print(json.dumps(dict(source=pin, actual_sub_sha=sha(sub.tobytes()),ref_stereo_controls_same=True,
 scope='same authentic release-corrected compact Main + intact stock Sub owner, replace only stock audible Sub with actual current source stereo already post OTA/AMP at proven centered fan-in, no FX/gain/control/clock changes; no installed private stage assumption',
 rows=rows),indent=2,allow_nan=False))
