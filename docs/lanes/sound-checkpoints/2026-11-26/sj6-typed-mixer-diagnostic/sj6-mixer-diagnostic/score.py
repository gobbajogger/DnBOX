from pathlib import Path
import io,sys,tarfile,json,hashlib
import numpy as np
o,b,s=map(Path,sys.argv[1:])
sys.path.insert(0,str(s))
from compare import channel_score
def dbr(r,a):return (20*np.log10(np.linalg.norm(a,axis=0)/np.linalg.norm(r,axis=0))).tolist()
sha=lambda x:hashlib.sha256(x).hexdigest()
results={}
with tarfile.open(b/'SJ6-Three-Factory-Presets-original-AUv3.tgz') as t:
 for i in range(3):
  n=f'./program-{i}-dry'
  cap=json.load(t.extractfile(n+'/capture.json'))
  source=np.load(io.BytesIO(t.extractfile(n+'/output.npy').read()))
  assert sha(source.tobytes())==cap['pcm_sha256'] and source.shape==(2,148684)
  r=source[:,16384:].T.astype('f8')
  ares={}
  for arm in ('base','divisor','center'):
   raw=(o/f'{arm}-{i}.raw').read_bytes()
   a=np.frombuffer(raw,'<f4').reshape(-1,2)
   assert a.shape==(148684,2) and np.isfinite(a).all() and not np.any(a[:16384])
   a=a[16384:].astype('f8')
   ares[arm]={'level_dB':dbr(r,a),'residual_dBr':dbr(r,a-r),'spectral_rms_dB':[float(channel_score(r[:,j],a[:,j],44100,(20,16000))[1]) for j in (0,1)],'meters':(o/f'{arm}-{i}.txt').read_text().strip(),
      'peak':abs(a).max(axis=0).tolist(),'clip_count':(abs(a)>1).sum(axis=0).tolist(),
      'matches_previous_projection':raw==(b/f'projected-v2.private/{i}-dry.private.f32').read_bytes(),'sha':sha(raw)}
  results[cap['wanted']['program']]=ares
print(json.dumps(results,indent=2,allow_nan=False))
