"""Separate harmonic-phase shape measurement, no alignment of full PCM."""
from pathlib import Path
import sys,io,tarfile,json,hashlib,numpy as np
code=Path('sj6-sub-body/probe.py').read_text().split('def harmonics',1)[1].split('x,trace,commit,pre,post=render(False)',1)[0]
exec(compile('def harmonics'+code,'prior signed spectral witness','exec'))
archive,x,y=map(Path,sys.argv[1:])
with tarfile.open(archive) as t:
 d='native-warm-brainwave-fanin-authored-sub-only'
 r=np.load(io.BytesIO(t.extractfile(d+'/output.npy').read()),allow_pickle=False)
 m=json.load(t.extractfile(d+'/capture.json'))
assert hashlib.sha256(r.tobytes()).hexdigest()==m['pcm_sha256']
before,after=(np.fromfile(f,'<f4').reshape(-1,2).T for f in (x,y))
res={}
for nm,i,j,f in [('first_hold',24576,37634,32.7032),('second_hold',53971,81434,97.9989)]:
 res[nm]={k:spectrum(v[0,i:j],f) for k,v in [('base',before),('updated',after),('installed',r)]}
print(json.dumps(res,indent=2))
