from pathlib import Path
import tarfile,json,hashlib,numpy as np
from source_main import render
p=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
assert hashlib.sha256(p.read_bytes()).hexdigest()=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(p) as t: m=json.load(t.extractfile('./program-2-dry/capture.json'))
assert m['active']==m['after']
r={int(k['address']):k['value'] for k in m['active']}
v,g,z,h,keys,c,b=render(m,r)
out={}
for i,k in enumerate(keys):
 f=np.flatnonzero(h[i])
 out[str(k)]={'reached':[int(f[0]),int(f[-1]),int(len(f))],'held_factor':float(g[i,int(f[0])+1024]),'last_factor':float(g[i,f[-1]]),'active_original_level_rms':float(np.sqrt(np.mean(z[i,f].astype('f8')**2)))}
print(json.dumps(dict(binary=b,readbacks=r[39],cue=m['cue_sha256'],rate=m['rate'],words=c,source=out,exact_amp_multiplication_states=int(h.sum())),indent=2))
