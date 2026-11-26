"""Use saved signed original Raw renders, no gain/time change or new PCM."""
from pathlib import Path
import hashlib
import json
import sys
import numpy as np
p=Path(sys.argv[1]); meta={}; pcm={}
for mode in ('dry','authored','authored-repeat'):
 d=p/('program-2-'+mode)
 m=json.loads((d/'capture.json').read_text())
 v=np.load(d/'output.npy',allow_pickle=False)
 assert v.shape==(2,148684) and m['rate']==44100
 assert hashlib.sha256(v.tobytes()).hexdigest()==m['pcm_sha256'] and m['active']==m['after']
 meta[mode]=m
 pcm[mode]=v.astype('f8')
assert np.array_equal(pcm['authored'],pcm['authored-repeat'])
assert all(q['schedule']==meta['dry']['schedule'] for q in meta.values())
a,b=({r['address']:r['value'] for r in meta[v]['active']} for v in ('authored','dry'))
assert {k for k in a if a[k]!=b[k]}=={'40'} and (a['40'],b['40'],a['208'])==(1,0,1)
part=pcm['authored']-pcm['dry']
rows={}
for tag,lo,hi in [('first',38434,49459),('final',82534,104584)]:
 rows[tag]={}
 for k,v in [('dry',pcm['dry']),('authored',pcm['authored']),('chorus_difference',part)]:
  a,b,c=(np.linalg.norm(v[:,lo:i])**2 for i in [lo+560,lo+1024,hi])
  rows[tag][k]={'energy_pct_in_initial_560':float(100*a/c),'energy_pct_in_initial_1024':float(100*b/c)}
print(json.dumps(rows,indent=2,allow_nan=False))
