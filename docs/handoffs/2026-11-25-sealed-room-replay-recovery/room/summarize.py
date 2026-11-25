"""Produce source-only settled results; never include PCM, NOR, or addresses."""
from pathlib import Path
import json,re,sys
text=Path(sys.argv[1]).read_text()
runs=[]
for m in re.finditer(r'^(base|trial) (elite|pro) (active|normal) ([0-5]) (/tmp/dnbox-emulator-only-[^\s]+)$',text,re.M):
    arm,model,profile,offset,run=m.groups()
    p=Path(run)
    def read(tag):
        f=p/f'{tag}.json'
        return json.loads(f.read_text()) if f.is_file() else None
    a=read('strict-budget'); r=read('room-witness')
    if a is None or r is None or not a['point_owner_witnesses_valid']:
        print(f'REJECTED {arm} {model} {profile} offset={offset}',file=sys.stderr);continue
    if profile=='active' and not (r['delay_on']==0 and r['reverb_on']==1 and r['reverb_mix']>0 and ((r['room_fb']==-2.)==(arm=='trial'))):
        print(f'REJECTED ACTIVE ROOM WITNESS {arm} {model}',file=sys.stderr);continue
    runs.append(dict(arm=arm,model=model,profile=profile,offset=int(offset),**a,
                     eight_line_active=r['room_fb']==-2.,reverb_on=r['reverb_on'],
                     delay_on=r['delay_on'],reverb_mix=r['reverb_mix']))
print(json.dumps(runs,indent=2))
