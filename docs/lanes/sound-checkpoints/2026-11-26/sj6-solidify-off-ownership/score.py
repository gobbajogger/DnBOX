import json,sys,hashlib,re
from pathlib import Path
import numpy as np
t,g=map(Path,sys.argv[1:3])
r=json.loads((t/'result.json').read_text())
states={}
for label in ('authored','swap','owner_first','only_owner','nonfixed'):
 a=np.fromfile(t/f'{label}.final','<f4').reshape(148684,2)
 q=[z.strip() for z in (t/f'{label}.log').read_text().splitlines() if z.startswith('post 82534')]
 v=[float.fromhex(x.split(' vel=')[1].split()[0]) for x in q]
 expected=float(np.float32((96 if label=='nonfixed' else 87)/127))
 assert v and all(z==expected for z in v)
 fa=a[82534:83558].astype(float)
 states[label]={'event_velocity':v,'row_count':len(q),'state':q,'rms':float(np.sqrt(np.mean(fa**2))),
   'whole_sha256':hashlib.sha256(a.tobytes()).hexdigest()}
 if label=='authored':
  assert a.tobytes()==(g/'suffix-1-1-dry.final').read_bytes()
  fixed=a
 else:
  assert np.array_equal(a,fixed)==(label=='swap')
for lane in ('0','1'):
 for label in ('authored','swap_discards','owner_first_negative','only_owner_negative','nonfixed_positive','hard_negative'):
  x=r['cases'][lane+'/'+label]
  assert x['same_output_as_authored']==(label in ('authored','swap_discards'))
  assert all(d[2][2]==float(np.float32((96 if label=='nonfixed_positive' else 87)/127)) for d in x['event_trace'])
  if label=='hard_negative': assert x['rms']==0
print(json.dumps(states,indent=2))
