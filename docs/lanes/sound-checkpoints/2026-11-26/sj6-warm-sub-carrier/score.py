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
def cfgpin(a):
 cfg=b/f'sub-range.private-output/2-{a}.private.txt'
 lines=cfg.read_text().splitlines();nw=int(lines[1]); words=dict((int(x),float(y)) for x,y in map(str.split,lines[2:2+nw]))
 assert all(words[k]==0 for k in (32,33,35,201)) and words[202]>0
 return cfg,lines,nw,words
def run(a,enabled,mute=False):
 cfg,lines,nw,words=cfgpin(a)
 if mute:
  words[202]=0.
  cfg=o/f'{a}-muted.cfg'
  cfg.write_text('\n'.join(lines[:2]+[f'{k} {v:.9g}' for k,v in words.items()]+lines[2+nw:])+'\n')
 name=f'{a}-{int(mute)}-{enabled}'
 p=o/f'{name}.so';p.write_bytes((o/'center.so').read_bytes())
 lib=c.CDLL(str(p))
 lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
 lib.set_zone.argtypes=[c.c_void_p]+[c.c_uint]*5+[c.c_double]
 for g in regions:
    sr,med=pack[g['sample'].replace('\\','/').split('/')[-1]]
    K,L,H,I,J=(int(g[x]) for x in ('pitch_keycenter','lokey','hikey','loop_start','loop_end'))
    lib.set_zone(med.ctypes.data,len(med),I,J,L,H,sr/(220.*2**((K-69)/12.)))
 lib.set_arm(enabled)
 f=o/f'{name}.f32'
 argv=(c.c_char_p*3)(b'probe',str(cfg).encode(),str(f).encode())
 assert lib.run_render(3,argv)==0
 counts={k:c.c_uint.in_dll(lib,k).value for k in ('reads','loops','resets')}
 assert counts['loops']==0
 return np.fromfile(f,'<f4').reshape(-1,2).T.copy(),counts
base=json.load(open('sj6-j60-warm-center/result.json'))
arms={a:[run(a,j) for j in (0,1)] for a in ('authored','dry')}
for a in ('dry','authored'):
 assert sha(arms[a][0][0].tobytes())==base['compact_sha']['center-'+a]
 assert cfgpin(a)[0] and sha(cfgpin(a)[0].read_bytes()) in base['controls_compact_fixture_sha']
 assert arms[a][0][1]['resets']==arms[a][1][1]['resets']==4
 assert arms[a][1][1]['reads']>0
x,cx=run('authored',0,True);y,cy=run('authored',1,True)
assert np.array_equal(x,y) and cx['reads']==cy['reads']==0
def rms(a):return float(np.sqrt(np.mean(a*a)))
def db(a,b):return float(20*np.log10(a/b))
rows={}
for w,l,r in [('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584),('full',16384,148684)]:
 row={}
 for a in ('authored','dry'):
  ref=refs[a][0][:,l:r].astype('f8')
  row[a]=[{'signed_dbr':db(rms(pc[:,l:r]-ref),rms(ref)),'level_db':db(rms(pc[:,l:r]),rms(ref))}
          for pc,count in arms[a]]
 rows[w]=row
print(json.dumps(dict(controls_compact_fixture_sha=base['controls_compact_fixture_sha'],real_app_ref_sha={a:refs[a][1]['pcm_sha256'] for a in refs},
 original_sub_map_sha=sha(sfz),original_sub_data_sha=sha(data),authentic_sub_samples_only_in_memory=True,
 baseline_warm_center_off_exact=True, muted_control_guard_exact_sha=sha(x.tobytes()),
 count_by_arm={a:[t[1] for t in values] for a,values in arms.items()},windows=rows),allow_nan=False,indent=2))
