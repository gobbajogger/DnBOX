import json,struct,sys
from pathlib import Path
d=Path(sys.argv[1])
def grab(name):
    assert 'skipped 0 cycles' in (d/(name+'.log')).read_text()
    b=(d/(name+'.ram')).read_bytes()
    assert len(b)==0x100000
    assert struct.unpack_from('<III',b,0x10000)==(0x4d4f5250,16384,3)
    return [struct.unpack_from('<IIIIff',b,0x1000c+24*k) for k in range(3)]
a,b=grab('uncached'),grab('cached')
out=[]
for f,u,v in zip((440,880,12000),a,b):
    assert u[2:]==v[2:] and u[2]==u[3]
    assert abs(u[4]-u[5])<.02
    row=dict(frequency_hz=f,direct_ticks=u[0],compact_ticks=u[1],direct_paired_image_ticks=v[0],paired_ticks=v[1],
             compact_over_direct=u[1]/u[0],paired_over_direct=v[1]/v[0],paired_over_simple=v[1]/u[1],
             phase=u[2],direct_sample_sum=u[4],compact_sample_sum=u[5])
    out.append(row)
print(json.dumps({'scope':'actual PI32v2 object and strict raw emulator, one core, no audio/scheduler/USB, relative timings only','lookups_per_arm':16384,'rows':out},indent=2))
