import json
from pathlib import Path
p=Path(__file__).parent
origin=json.loads((p/'original-with-hash.json').read_text())
typed=json.loads((p/'typed.json').read_text())
new=json.loads((p/'amp.json').read_text())
for k,t in typed.items():
 r=origin[str(int(k)-12)]
 assert r['first']==t['first'] and r['gate_off']==t['gate_off']
 assert t['last']-r['last']==40 and t['last_nonzero_amp']-r['last_nonzero_amp']==39
 for data in new.values():
  cand=data['notes'][k]
  assert r['release_amp_hash']==cand['release_amp_hash']
  assert r['last_nonzero_amp']==cand['last_nonzero']
 for v in (r,t):v.pop('signed_amp_close',None);v['signed_derivatives']=v['signed_derivatives'][:2]
print(json.dumps(dict(current_original=origin,connected_incumbent=typed,model=new,exact_release_words_per_cue=4*560),indent=2))
