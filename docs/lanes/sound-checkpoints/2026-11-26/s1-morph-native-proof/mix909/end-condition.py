import json,struct,subprocess,sys
from pathlib import Path
elf,ram=sys.argv[1:]; sy={}
for l in subprocess.check_output(['readelf','-sW',elf],text=True).splitlines():
 p=l.split()
 if len(p)>=8 and p[2].isdigit() and p[6]!='UND':sy[p[7]]=(int(p[1],16),int(p[2]))
b=Path(ram).read_bytes();a,n=sy['mix_glide']
assert n==5*6*8
cur_tgt=struct.unpack_from('<12f',b,a-0x01c00000)
g=list(zip(cur_tgt[::2],cur_tgt[1::2]))
print(json.dumps({'scope':'terminal evidence only, no activation count',
 'lanes':dict(zip(['L','R','REV','DLY','VREV','VDLY'],g)),
 'select_eligible_on_snapshot':all(a==b for a,b in g) and all(g[i][0]!=1 for i in [0,1,4,5])},indent=2))
