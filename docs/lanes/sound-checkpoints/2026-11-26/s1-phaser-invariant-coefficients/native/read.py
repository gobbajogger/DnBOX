from pathlib import Path
import json,struct
def a(name):
 x=Path(name+'.ram').read_bytes(); assert len(x)==1048576
 assert struct.unpack_from('<2I',x,32768)==(0x43554231,0x600d600d)
 assert 'flash: 0 erases / programs' in Path(name+'.log').read_text()
 return list(struct.iter_unpack('<2I',x[32784:32784+12*8]))
a,b=a('base'),a('new');assert all(x[1]==y[1] for x,y in zip(a,b)),(a,b)
rows=[dict(profile=i//4,try_=i%4,stock_timer_ticks=x[0],candidate_timer_ticks=y[0],signed_pcm_FNV=x[1],stock_us=x[0]/24,candidate_us=y[0]/24) for i,(x,y) in enumerate(zip(a,b))]
Path('native.json').write_text(json.dumps(rows,indent=2))
print(rows)
