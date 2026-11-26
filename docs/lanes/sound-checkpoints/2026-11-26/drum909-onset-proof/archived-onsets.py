from pathlib import Path
import json,sys
roots=list(map(Path,sys.argv[1:]))
out={}
for m in ('elite','pro'):
 rows=[]
 for root in roots:
  p,=root.glob(f'dnbox-loaded-replay-{m}-*/audio.s16le')
  b=p.read_bytes()
  start=next(i//4 for i in range(0,len(b),4) if b[i:i+4]!=bytes(4))
  rows.append({'onset':start,'nonzero_first_512_frames':sum(b[i*4:i*4+4]!=bytes(4) for i in range(65600,66112)),'nonzero_second_512_frames':sum(b[i*4:i*4+4]!=bytes(4) for i in range(66112,66624))})
 out[m]=rows
assert out['elite'][0]['onset']==out['elite'][1]['onset']==out['pro'][0]['onset']==65600
assert out['pro'][1]['onset']==66624
print(json.dumps(out,indent=2))
