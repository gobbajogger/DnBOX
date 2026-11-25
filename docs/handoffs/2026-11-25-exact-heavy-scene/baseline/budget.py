"""Fail-closed validation of guest startup and raw per-run timing, no fixture mutation."""
from pathlib import Path
import json,re,struct,sys
from guarded_replay import symbols_and_sections,ELF_RAM
elf,folder=map(Path,sys.argv[1:3])
_,s,sec=symbols_and_sections(elf)
r=(folder/'ram.bin').read_bytes()
log=(folder/'run.log').read_text()
def get(k,n=1):
    i=s[k]-ELF_RAM
    assert 0<=i<=len(r)-n,(k,i)
    data=r[i:i+n]
    return data[0] if n==1 else int.from_bytes(data,'little')
assert get('safe_mode')==0 and get('cold_lab_bad')==0
for stem,label in [('ram_text','rt'),('c1_text','c1')]:
    addr,size,data=sec['.'+stem]
    assert addr==s[f'_{label}_start'] and data==r[addr-ELF_RAM:addr-ELF_RAM+size],stem
t=json.loads((folder/'telemetry.json').read_text())
o=json.loads((folder/'owner.json').read_text())
dbg,=struct.unpack_from('<I',r,s['x0x_dbg']-ELF_RAM)
skipped=[int(n) for n in re.findall(r'skipped (\d+) cycles',log)]
assert dbg==0x44424731 and t['halves']>0 and skipped
result={'boot_guard_and_overlays':True,'skipped_cycles':max(skipped),'accepted':False,**t}
result['meets_timing']=t['worst_us']<=10809 and t['xruns']==t['raw_late']==max(skipped)==0
result['settled_owner_witnesses']=o['count']
base=s['fm1_c1_mb']-ELF_RAM
job,=struct.unpack_from('<I',r,base)
alive,done,bad=struct.unpack_from('<III',r,base+64)
result['core1_final']={'enabled':get('fm1_c1_on',4),'gave_up':get('fm1_c1_gave_up'),
                       'alive':alive,'dispatched':job,'finished':done,'bad_mailbox':bad}
stackbase=s['_c1_ustack']-ELF_RAM
stack=struct.unpack_from('<1088I',r,stackbase)
result['core1_untouched_stack_bytes']=4*next((i for i,x in enumerate(stack) if x!=0x43314D4B),1088)
# This does not confer qualification: scope also requires full gate, exact owner
# throughout, independent A/B, and all requested offsets on both models.
print(json.dumps(result))
