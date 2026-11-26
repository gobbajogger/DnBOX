from pathlib import Path
import hashlib, tarfile, io, sys, json, numpy as np
o,b=map(Path,sys.argv[1:])
a=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
def sha(x):return hashlib.sha256(x).hexdigest()
assert sha(a.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
def score(x,y):
 x,y=(k.astype('f8') for k in (x,y))
 l=lambda a,b:float(20*np.log10(a/b)) if a and b else None
 n=np.linalg.norm(y)
 return dict(level_db=l(np.linalg.norm(x),n),signed_dbr=l(np.linalg.norm(x-y),n))
res={}
with tarfile.open(a) as t:
 for i in range(3):
  for arm in ('authored','dry'):
   k=f'{i}-{arm}';d=f'./program-{k}'
   m=json.load(t.extractfile(d+'/capture.json')); ref=np.load(io.BytesIO(t.extractfile(d+'/output.npy').read()),allow_pickle=False)
   assert sha(ref.tobytes())==m['pcm_sha256'] and m['active']==m['after'] and ref.shape==(2,148684)
   w={int(x['address']):x['value'] for x in m['active']}
   r={}
   for center in (0,1):
    rr={}
    for variant in (0,2,3):
     stem=o/f'{center}-{variant}-{k}'
     raw=stem.with_suffix('.f32').read_bytes()
     pro,meters=stem.with_suffix('.txt').read_text().splitlines()
     assert pro.startswith('projection')
     pcm=np.frombuffer(raw,'<f4').reshape((-1,2))
     assert len(pcm)==len(ref.T) and not pcm[:16384].any()
     if center==variant==0:assert raw==(b/f'sub-range.private-output/{k}.private.f32').read_bytes()
     rr[variant]=(pcm,pro,meters,sha(raw))
    x,p0,mt0,h0=rr[0]
    for v in (2,3):
     y,p1,mt1,h1=rr[v]
     assert p0==p1
     assert (h0!=h1)==(i==1)
     assert mt0.split()[1]==mt1.split()[1]
     windows={}
     for name,l,u in [('whole',16384,148684),('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584)]:
         windows[name]=dict(before=score(x[l:u],ref.T[l:u]),after=score(y[l:u],ref.T[l:u]),change=score(y[l:u],x[l:u]))
     r[f'{center}:{v}']={'projection':p0,'base_meters':mt0,'candidate_meters':mt1,'untouched_exact':h0==h1,
                     'base_pcm_sha':h0,'candidate_pcm_sha':h1,'windows':windows}
   res[k]=dict(name=m['wanted']['program'],live_ctor_shape=[w[82],w[83]],version=m['component_version'],variants=r)
print(json.dumps(res,indent=2,allow_nan=False))
