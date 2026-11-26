import sys,json,numpy as np,ctypes as C,hashlib
from pathlib import Path
gdir,loc=map(Path,sys.argv[1:3]);s=(gdir/'score.py').read_text();exec(s[:s.rindex('print(json.dumps')])
original=json.loads((loc/'tap.json').read_text())
F=C.CDLL('libm.so.6').fma; F.restype=C.c_double;F.argtypes=[C.c_double]*3
d={'source_sha':original['binary'],'source_cases':[],'host':{}}
for case in original['cases']:
 assert all(event[2] for event in case['events'])
 for k,x,b,y,c,z in case['receipts']:
  assert x==y and b==c
  b0,b1,b2,a1,a2,x1,x2,y1,y2=b
  p2=np.float32(F(-a2,y2,F(-a1,y1,F(b2,x2,F(b0,x,b1*x1)))))
  assert p2==z,(k,p2,z)
 d['source_cases'].append(dict(layer=case['layer'],exact=len(case['receipts']),
 events_unchanged_history=[v[:3] for v in case['events']],
 coefficients=case['receipts'][0][2][:5],last_prepostmarker=[case['receipts'][-1][k] for k in [0,1,5]]))
assert (loc/'0-1-dry.final').read_bytes()==(gdir/'suffix-1-1-dry.final').read_bytes()
hold=[(82534,82662),(82662,82918),(82918,83094),(83094,83534)]
for case in range(3):
 x=np.fromfile(loc/f'{case}-1-dry.final','<f4').reshape(-1,2).T; ref=clips['dry']
 for keep in ('0-dry','2-dry'):
  for e in ('pre','final'):assert (loc/f'{case}-{keep}.{e}').read_bytes()==(loc/f'0-{keep}.{e}').read_bytes()
 r={}
 for i,j in hold:
  a,b=(z[0,i:j].astype(float) for z in (ref,x))
  r[str(i)]={'level_db':float(db(np.linalg.norm(b)/np.linalg.norm(a))),
              'signed_dBr':float(db(np.linalg.norm(b-a)/np.linalg.norm(a)))}
 d['host'][str(case)]={'windows':score(ref,x),'fine_release':r,'typed_sha':hashlib.sha256((loc/f'{case}-1-dry.final').read_bytes()).hexdigest()}
print(json.dumps(d,indent=2,allow_nan=False))
