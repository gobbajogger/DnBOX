from pathlib import Path
import json,re,struct,sys
from guarded_replay import symbols_and_sections,ELF_RAM
elf,folder,seed=map(Path,sys.argv[1:4])
_,s,sec=symbols_and_sections(elf)
r=(folder/'ram.bin').read_bytes()
log=(folder/'run.log').read_text()
a=seed.read_bytes(); b=(folder/'disposable.bin').read_bytes()
assert len(a)==len(b)==0x100000
assert all(x==y or 0xaf000<=i<0xaf010 for i,(x,y) in enumerate(zip(a,b)))
assert 'flash: 0 erases / programs' in log and 'panel.ppm at 8000 ms' in log
def get(k,n=1):
    i=s[k]-ELF_RAM
    assert 0<=i<=len(r)-n,(k,i)
    return int.from_bytes(r[i:i+n],'little')
assert get('safe_mode')==0 and get('cold_lab_bad')==0 and get('x0x_dbg',4)==0x44424731
for stem,label in [('ram_text','rt'),('c1_text','c1')]:
    addr,size,data=sec['.'+stem]
    assert addr==s[f'_{label}_start'] and data==r[addr-ELF_RAM:addr-ELF_RAM+size],stem
t=json.loads((folder/'telemetry.json').read_text());o=json.loads((folder/'owner.json').read_text())
skip=[int(n) for n in re.findall(r'skipped (\d+) cycles',log)]
assert t['halves']>0 and skip
stackbase=s['_c1_ustack']-ELF_RAM
stack=struct.unpack_from('<1088I',r,stackbase)
free=4*next((i for i,x in enumerate(stack) if x!=0x43314D4B),1088)
assert free>=256
valid_owner=o['count']>=4 and all(x['bank']==2 and x['status']==1 and x['cutoff']!=16383 and
       x['bankB']==0 and x['statusB']==1 and x['arp']==1 for x in o['owners'])
print(json.dumps({'boot_guard_and_overlays':True,'skipped_cycles':max(skip),**t,
     'point_owner_witnesses':o['count'],'point_owner_witnesses_valid':valid_owner,
     'core1_untouched_stack_bytes':free,'accepted':False,
     'meets_timing':valid_owner and t['worst_us']<=10809 and t['xruns']==t['raw_late']==max(skip)==0}))
