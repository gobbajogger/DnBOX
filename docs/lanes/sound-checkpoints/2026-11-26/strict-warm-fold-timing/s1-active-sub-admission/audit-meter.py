"""An empty meter is inconclusive, never evidence of zero gain/positive eligibility."""
from pathlib import Path
import sys,struct,json
p=Path(__file__).resolve().parent
sys.path.insert(0,str(p.parent/'restored-harness-inspect/helpers'))
import guarded_replay as g
f,ram,pin=sys.argv[1:]
b,sym,sec=g.symbols_and_sections(Path(f));assert g.sha(b)==pin
r=g.regular(Path(ram))
for name in ('ram_text','c1_text'):
 a,n,data=sec['.'+name];at=a-g.ELF_RAM
 assert len(data)==n and data==r[at:at+n]
at=sym['s1_sub_meter']-g.ELF_RAM
assert 0<=at<=len(r)-24
a,b,c,d,e,k=struct.unpack_from('<6I',r,at)
if (a,b,c,d,e,k)==(0,0,0,0,0,0):
 o=dict(meter_populated=False,eligibility_verified=False,reason='no matched owner-A live KR-voice samples witnessed; DO NOT treat as active muted Sub')
else:
 assert a and k==0x53315355 and all(i<=a for i in (b,c,d,e))
 assert bool(e)==bool(b) and d<=a-max(b,c)
 o=dict(meter_populated=True, eligibility_verified=True,active_voice_calls=a,active_sub_nonzero=b,main_not_finite_normal=c,eligible=d,first_nonzero_source_voice_call=e)
o.update(elf_sha256=pin,ram_sha256=g.sha(r),scope='owner A only instrumented non-timing')
print(json.dumps(o,indent=2))
