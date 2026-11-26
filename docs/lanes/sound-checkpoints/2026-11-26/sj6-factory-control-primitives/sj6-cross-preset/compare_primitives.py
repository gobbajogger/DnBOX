"""Summarize distinct scoped control variants without aggregating tolerances."""
from pathlib import Path
import hashlib,json,math,numpy as np
W=Path(__file__).parent
out={}
B=W/'primitive-0.private-output'
def dig(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for a in B.glob('*.f32'):
 assert dig(a)==dig(W/'projected-v2.private'/a.name)
for m in (1,2,4):
 d=W/f'primitive-{m}.private-output'
 ref=json.loads((W/'primitive-0.json').read_text())
 actual=json.loads((W/f'primitive-{m}.json').read_text())
 rows={}
 for b in sorted(B.glob('*.f32')):
  i,arm,*_=b.name.split('.')[0].split('-')
  x=np.fromfile(b,'<f4').astype('f8')
  y=np.fromfile(d/b.name,'<f4').astype('f8')
  n=np.flatnonzero(x!=y)
  dn=np.linalg.norm(y-x)
  bl=np.array([r['level_delta_db'] for r in ref[int(i)][arm]['whole']])
  cl=np.array([r['level_delta_db'] for r in actual[int(i)][arm]['whole']])
  rows[b.name]=dict(first_different_frame=int(n[0]//2) if n.size else None,
   signed_vs_baseline_dbr=20*math.log10(dn/np.linalg.norm(x)) if dn else None,
   all_sample_words_equal=not n.size, native_whole_level_change_db=(cl-bl).tolist())
 out[m]=rows
print(json.dumps(out,indent=2))
