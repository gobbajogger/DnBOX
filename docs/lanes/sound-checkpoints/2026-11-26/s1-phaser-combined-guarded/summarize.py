from pathlib import Path
import json,statistics
p=Path(__file__).resolve().parent; a=list(map(json.loads,(p/'bank.jsonl').read_text().splitlines()));assert len(a)==len({q['row'] for q in a})==906
def pick(stage,key):
 q=max(a,key=lambda q:abs(q[stage][key]) if key=='level_db' else q[stage][key])
 return dict(row=q['row'],**q[stage])
b=[dict(row=q['row'],**n) for q in a for n in q.get('bands',[])]
o=dict(cases=len(a),exact=sum(q['exact'] for q in a),active=sum(bool(q['angle']) for q in a))
for s in ('whole','tail'):
 o[s]={k:pick(s,k) for k in ('null_dbr','err_dbfs','peak_dbfs','level_db')}
for k in ('err_dbfs','null_dbr','delta_db'):
 o['band_'+k]=max(b,key=lambda q:abs(q[k]) if k=='delta_db' else q[k])
for i,which in enumerate(('width','rate')):
 vv=[n for q in a for n in q['ctl'] if n[0]==i]
 o[which]=dict(changes=sum(n[1] for n in vv),minimum=min(n[2] for n in vv),maximum=max(n[3] for n in vv),fallback=sum(n[4] for n in vv))
(p/'bank-summary.json').write_text(json.dumps(o,indent=2)+'\n')
print(json.dumps(o))
