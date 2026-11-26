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
def run(a,enabled,mute=False,shape=1,split=0):
 cfg,lines,nw,words=cfgpin(a)
 if mute:
  words[202]=0.
  cfg=o/f'{a}-muted.cfg'
  cfg.write_text('\n'.join(lines[:2]+[f'{k} {v:.9g}' for k,v in words.items()]+lines[2+nw:])+'\n')
 name=f'{a}-{int(mute)}-{enabled}-{shape}-{split}'
 p=o/f'{name}-{len(list(o.glob("*.so")))}.so';p.write_bytes((o/'center.so').read_bytes())
 lib=c.CDLL(str(p))
 lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
 lib.set_zone.argtypes=[c.c_void_p]+[c.c_uint]*5+[c.c_double]
 for g in regions:
    sr,med=pack[g['sample'].replace('\\','/').split('/')[-1]]
    K,L,H,I,J=(int(g[x]) for x in ('pitch_keycenter','lokey','hikey','loop_start','loop_end'))
    lib.set_zone(med.ctypes.data,len(med),I,J,L,H,sr/(220.*2**((K-69)/12.)))
 lib.set_arm(enabled)
 lib.set_shape(shape)
 lib.ota_split_enable(split)
 f=o/f'{name}.f32'
 argv=(c.c_char_p*3)(b'probe',str(cfg).encode(),str(f).encode())
 assert lib.run_render(3,argv)==0
 counts={k:c.c_uint.in_dll(lib,k).value for k in ('reads','loops','resets')}
 assert counts['loops']==0
 q=np.fromfile(f,'<f4').reshape(-1,2).T.copy(); f.unlink(); p.unlink()
 counts['split']=lib.ota_split_count()
 return q,counts

import os
def render(a,split,mute=False):
 with open(o/f'{a}-{split}-{mute}.log','w') as log:
   old=os.dup(2);os.dup2(log.fileno(),2)
   try:return run(a,1,mute,1,split)
   finally:os.dup2(old,2);os.close(old)
arms={a:[render(a,s) for s in (0,1)] for a in ('authored','dry')}
def rms(x):return np.sqrt(np.mean(x*x))
def db(x,y):return float(20*np.log10(rms(x)/rms(y)))
base=json.load(open('sj6-warm-sub-release-lab/result.json'))['windows']
rows={}
for label,l,r in [('attack',16384,20794),('held',20794,38434),('first-release',38434,49459),('chord',53869,82534),('final-release',82534,104584),('full',16384,148684)]:
 rows[label]={}
 for a in arms:
   y=refs[a][0][:,l:r].astype('f8')
   q=[(v[:,l:r].astype('f8'),ct) for v,ct in arms[a]]
   assert q[0][1]==dict(reads=123675,loops=0,resets=4,split=0)
   assert q[1][1]==dict(reads=123675,loops=0,resets=4,split=123675),q[1][1]
   scores=[dict(signed_dbr=db(t-y,y),level_db=db(t,y)) for t,_ in q]
   if label in base:assert abs(scores[0]['signed_dbr']-base[label][a][1]['signed_dbr'])<1e-9
   rows[label][a]=scores
m0,c0=render('authored',0,True);m1,c1=render('authored',1,True)
assert c0==c1 and not c0['split'] and np.array_equal(m0,m1)
print(json.dumps(dict(mode='one actual host owner; raw carrier in private RAM only; source-shaped release after Sub OTA instead of pre-shaping carrier; known native terminal only',source_off_baseline_verified=True,mute_exact=True,entry_counts={a:[c for _,c in v] for a,v in arms.items()},windows=rows),indent=2,allow_nan=False))
