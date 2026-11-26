import sys,subprocess,re,struct,hashlib,json
from pathlib import Path
p,r=map(Path,sys.argv[1:]);b=r.read_bytes(); assert len(b)==0x100000
m=re.findall(r'^\s*\d+: ([0-9a-f]+)\s+24\s+OBJECT\s+\w+\s+DEFAULT\s+\d+\s+s1_sub_meter$',subprocess.check_output(['readelf','-sW',str(p)],text=True),re.M)
assert len(m)==1,m
o=int(m[0],16)-0x01c00000;assert 0<=o<=len(b)-24
total,active_sub,non_normal,eligible,first_live,magic=struct.unpack_from('<6I',b,o)
assert magic==0x53315355 and 0<total and all(x<=total for x in (active_sub,non_normal,eligible,first_live))
assert (first_live!=0)==(active_sub!=0) and eligible<=total-max(active_sub,non_normal)
print(json.dumps(dict(elf_sha256=hashlib.sha256(p.read_bytes()).hexdigest(),ram_sha256=hashlib.sha256(b).hexdigest(),active_voice_calls=total,active_sub_nonzero=active_sub,main_not_finite_normal=non_normal,eligible=eligible,first_nonzero_source_voice_call=first_live,scope='instrumented non-timing identity replay, owner A only'),indent=2))
