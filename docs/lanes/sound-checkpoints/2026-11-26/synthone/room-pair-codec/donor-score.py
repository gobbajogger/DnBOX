import json,sys
from pathlib import Path
t,donor=map(Path,sys.argv[1:])
def load(p):
 a=[json.loads(x) for x in p.read_text().splitlines()];return {r['id']:r for r in a[1:]}
base=load(t/'base.jsonl'); old=load(donor)
sc={}
for p in sorted(t.glob('[0-9]*.jsonl')):
 d=load(p);o=[]
 for k,a in base.items():
  b=d[k];c=old[k]
  assert a['ref_sha256']==b['ref_sha256']==c['ref_sha256'],k
  assert a['trial_sha256']==c['trial_sha256'],(k,a['trial_sha256'],c['trial_sha256'])
  assert a['same_note_events']==b['same_note_events']==c['same_note_events'],k
  o.append({'id':k,'label':a['label'],'baseline_pcm':a['trial_sha256'],'candidate_pcm':b['trial_sha256'],'original_pcm':a['ref_sha256'],'events':a['same_note_events'],'baseline_metrics':a['metrics'],'candidate_metrics':b['metrics']})
 sc[p.stem]=o
print(json.dumps(sc,indent=2,allow_nan=False))
