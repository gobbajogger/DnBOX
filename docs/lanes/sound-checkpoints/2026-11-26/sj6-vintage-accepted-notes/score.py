from pathlib import Path
import hashlib,tarfile,sys,io,json,numpy as np
o,b=map(Path,sys.argv[1:])
q=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
sha=lambda z:hashlib.sha256(z).hexdigest()
assert sha(q.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
orig=json.load(open('sj6-solidify-sine-phase/result.json'))
r={}
def err(x,y):
 x,y=(z.astype('f8') for z in (x,y))
 return float(20*np.log10(np.linalg.norm(x-y)/np.linalg.norm(y))) if np.any(x-y) else None
for k in ('0-dry','1-dry','1-authored','2-dry'):
    a,d=(np.fromfile(o/f'{j}-{k}.f32','<f4').reshape(-1,2) for j in (0,1))
    assert sha(a.tobytes())==orig[k]['variants']['1:2']['candidate_pcm_sha']
    delta=np.flatnonzero(np.any(a!=d,axis=1))
    if not k.startswith('1'):
       assert not len(delta)
       r[k]={'unchanged':True}; continue
    assert len(delta) and delta[0]>=49459
    with tarfile.open(q) as f:
       k0=f'./program-{k}/'
       ref=np.load(io.BytesIO(f.extractfile(k0+'output.npy').read()),allow_pickle=False)
       m=json.load(f.extractfile(k0+'capture.json'))
    assert sha(ref.tobytes())==m['pcm_sha256'] and m['after']==m['active']
    n=ref.T
    res={}
    for name,lo,hi in [('first_hold',20794,38434),('first_release',38434,49459),('chord',53869,82534),('final_early',82534,83239)]:
      x,y,z=a[lo:hi],d[lo:hi],n[lo:hi]
      res[name]={'before_vs_saved':err(x,z),'after_vs_saved':err(y,z),'after_vs_before':err(y,x)}
    r[k]={'first_changed_frame':int(delta[0]),'scope':'fixed compact PRNG seed only; original entropy not matched, diagnostic only','rows':res}
print(json.dumps(r,indent=2,allow_nan=False))
