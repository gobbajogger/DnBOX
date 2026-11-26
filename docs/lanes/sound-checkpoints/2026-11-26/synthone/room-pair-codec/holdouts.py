"""Deterministic roster: diverse wet programs, no trial audio used for selection."""
import json
from pathlib import Path
D=Path(__file__).resolve().parents[2]/'recovered-sound'
cases=json.loads((D/'s1-bank-replay/original-private/ORIGINAL-PRESET-COMMANDS.json').read_text())['cases']
rows=json.loads((D/'s1-room-parity/aligned-bank.json').read_text())['rows']
base={0,62,113,154,369,527,788,848}
wet=[]
from hashlib import sha256
for i, row in enumerate(rows):
 c=cases[row['original_order']]
 v=dict(c['commands'])
 if i not in base and v[37]>.5 and v[40]>0:
  assert c['name']==row['label'],(i,c['name'],row)
  wet.append(dict(id=i,bank=c['bank'],uid=c['uid'],label=c['name'],mix=v[40],feedback=v[38],hp=v[39],order=sha256(c['uid'].encode()).hexdigest()))
h=[]
for lo,hi in [(0,.10),(.10,.25),(.25,.5),(.5,1.01)]:
 for flo,fhi in [(0,.6),(.6,1.01)]:
  p=sorted((r for r in wet if lo<r['mix']<=hi and flo<=r['feedback']<fhi),key=lambda r:r['order'])
  h+=p[:3]
print(' '.join(str(i) for i in sorted(base|{r['id'] for r in h})))
