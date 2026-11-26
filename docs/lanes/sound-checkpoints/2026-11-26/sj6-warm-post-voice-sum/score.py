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
def run(a,arm,which):
 src=stems[which][0] if which else true_sum
 cfg=b/f'sub-range.private-output/2-{a}.private.txt'
 assert sha(cfg.read_bytes()) in base['controls_compact_fixture_sha']
 name=f'{a}-{int(arm)}-{which}'
 p=o/f'{name}.so';p.write_bytes((o/'center.so').read_bytes())
 lib=c.CDLL(str(p))
 lib.upstream.argtypes=[c.c_void_p,c.c_uint,c.c_int];lib.upstream(src.ctypes.data,src.shape[1],arm)
 lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
 f=o/f'{name}.f32'
 argv=(c.c_char_p*3)(b'probe',str(cfg).encode(),str(f).encode())
 assert lib.run_render(3,argv)==0 and lib.reached_frames()==src.shape[1]
 x=np.fromfile(f,'<f4').reshape(-1,2).T.copy()
 assert x.shape==true_sum.shape
 return x
results={x:[run(x,0,None),run(x,1,None)] for x in ('authored','dry')}
for a in ('authored','dry'):
 assert sha(results[a][0].tobytes())==base['compact_sha']['center-'+a]
 assert refs[a][1]['pcm_sha256']==base['original_sha'][a]
isolated={}
for x in ('Main','Sub'):
 isolated[x]=run('dry',1,x)
assert np.array_equal(refs['authored'][0],refs['authored-repeat'][0])
def rms(x):return float(np.sqrt(np.mean(x*x)))
def db(a,b):return float(20*np.log10(a/b)) if a and b else None
wins={}
for w,l,r in [('attack',16384,20794),('held',20794,38434),('first-release',38434,49459),('chord',53869,82534),('final-release',82534,104584),('full',16384,148684)]:
 row={}
 for a in ('authored','dry'):
  ref=refs[a][0][:,l:r].astype('f8')
  row[a]=[{'signed_dbr':db(rms(pc[:,l:r]-ref),rms(ref)),'level_db':db(rms(pc[:,l:r]),rms(ref))}
          for pc in results[a]]
 for v in (0,1):
  x=results['authored'][v][:,l:r]-results['dry'][v][:,l:r]
  y=refs['authored'][0][:,l:r]-refs['dry'][0][:,l:r]
  row[f'authored_minus_dry_{v}_signed_dbr']=db(rms(x-y),rms(y))
 dt=results['dry'][1][:,l:r].astype('f8')
 mix=isolated['Main'][:,l:r].astype('f8')+isolated['Sub'][:,l:r].astype('f8')
 row['separate_process_sum_vs_joined_compact_dry_dbr']=db(rms(dt-mix),rms(dt))
 wins[w]=row
print(json.dumps(dict(source_binary=binary,source_asset_hashes={'data':sha(data),'map':sha(sfz)},source_roles={
 name:dict(output_sha=sha(x.tobytes()),receiver_controls=c,events=by,
   events_columns=['first frame','last owner frame','last nonzero AMP','first release','active counted']) for name,(x,c,by) in stems.items()},
 input_control_words=base['shared_normalized_binary32_exact_for_both_arms'],source_predry_HP_scope='before actual compact HP; Raw compact cached HP 0 does not assert original Apple floor/state',
 source_reference_tape_hashes=base['original_sha'],unchanged_prior_compact_dry_authored=True,original_assets_and_tape_in_ram_only=True,
 tested_modes=['exact Compact original runtime same source with independent per-sampler post-OTA stereo child sum','no original full app suffix assertion'],windows=wins),allow_nan=False,indent=2))
