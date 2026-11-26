import json
from pathlib import Path
p=Path(__file__).with_name('result.json')
d=json.loads(p.read_text()); zz=d['zones']
def best(budget):
 for bound in sorted({a['worst'] for v in zz.values() for a in v.values()}):
    arms={}
    for root, options in zz.items():
        options=[(m['B'],k,m['worst']) for k,m in options.items() if m['worst']<=bound]
        if options:arms[root]=min(options)
    B=288+sum(v[0] for v in arms.values())
    if len(arms)==len(zz) and B<=budget:
        return dict(budget=budget,payload_bytes=B,max_signed_dbr=bound,options={k:q[1] for k,q in arms.items()})
 return dict(budget=budget,options=None)
print(json.dumps([best(x) for x in (8192,16384,24576,112*1024)],indent=2))
