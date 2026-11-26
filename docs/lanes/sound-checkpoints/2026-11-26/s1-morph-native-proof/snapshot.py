"""Endpoint state already captured by uninstrumented strict runs; not a timing profile."""
import json,struct,subprocess,sys
from pathlib import Path
elf,ram=sys.argv[1:]; symbols={}
for row in subprocess.check_output(['readelf','-sW',elf],text=True).splitlines():
    c=row.split()
    if len(c)>=8 and c[2].isdigit() and c[6]!='UND':
        symbols[c[7]]=(int(c[1],16),int(c[2]))
r=Path(ram).read_bytes()
def data(name,n=None):
    a,l=symbols[name];a-=0x01c00000
    assert a>=0 and a+(n or l)<=len(r)
    return r[a:a+(n or l)]
def number(name):
    b=data(name)
    if len(b) not in (1,2,4):return None
    return int.from_bytes(b,'little')
p={name:number(name) for name in ('eng_plan','eng_c1_mask','eng_stage','eng_c1_next','eng_job_arg','eng_808_split','safe_mode','audio_xruns') if name in symbols}
p['job_cost_24M_per_256_frames']=struct.unpack('<5I',data('eng_job_cost',20))
i,l=symbols['_c1_ustack'];e,_=symbols['_c1_ustack_top'];n=e-i
s=data('_c1_ustack',n); mark=struct.pack('<I',0x43314d4b)
untouched=next((i for i in range(0,n,4) if s[i:i+4]!=mark),n)
p.update(stack_region_bytes=n,stack_untouched_at_bottom=untouched,stack_high_water_bytes=n-untouched)
print(json.dumps(p,indent=2))
