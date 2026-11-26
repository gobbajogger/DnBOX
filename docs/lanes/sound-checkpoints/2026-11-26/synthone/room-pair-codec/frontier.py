from pathlib import Path
import json
D=Path(__file__).resolve().parent
s=json.loads((D/'screen.json').read_text())
d=json.loads((D/'donor.json').read_text())
out=[]
for k,rows in d.items():
 records=[]
 for r in rows:
  m=r['candidate_metrics']; old=r['baseline_metrics']
  records.append({'id':r['id'],'label':r['label'],
   'level_bound':max(abs(y) for w in ('attack','held','release','last_second','whole') for y in m[w]['level_db']),
   'shape':max(m['tail_spectral_rms_db']),'residual':max(m['whole']['residual_dbr']),
   'base_shape':max(old['tail_spectral_rms_db'])})
 assert s[k]['storage']['saturations']==[0,0]
 out.append({'format':k,'history_bytes':s[k]['storage']['history_bytes_including_pending'],
             'host_owner_state_bytes':s[k]['storage']['state_bytes'],'rows':len(rows),
             'level_shape_wave_pass':sum(x['level_bound']<=1. and x['shape']<=3. and x['residual']<=-20. for x in records),
             'worst_level':max(records,key=lambda x:x['level_bound']),
             'worst_shape':max(records,key=lambda x:x['shape']),
             'worst_residual':max(records,key=lambda x:x['residual']),
             'organ':next(x for x in records if x['id']==113)})
out.sort(key=lambda x:(x['history_bytes'],x['format']))
print(json.dumps(out,indent=2,allow_nan=False))
