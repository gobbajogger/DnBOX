"""Read only sealed lab replay telemetry. It cannot grant release qualification."""
import json,re,struct,sys
from pathlib import Path
here=Path(__file__).resolve().parent
sys.path.insert(0,str(here.parent/'restored-harness-inspect/helpers'))
from guarded_replay import ELF_RAM,symbols_and_sections,ensure
elf,out=map(Path,sys.argv[1:3])
_,sym,sec=symbols_and_sections(elf)
r=(out/'ram.bin').read_bytes()
pcm=(out/'audio.s16le').read_bytes()
def words(name,n=1):
    i=sym[name]-ELF_RAM
    ensure(0<=i<=len(r)-4*n,name)
    return struct.unpack_from(f'<{n}I',r,i)
def byte(name):
    i=sym[name]-ELF_RAM
    ensure(0<=i<len(r),name)
    return r[i]
for stem,label in (('ram_text','rt'),('c1_text','c1')):
    addr,n,expected=sec['.'+stem]
    i=addr-ELF_RAM
    ensure(addr==sym[f'_{label}_start'] and r[i:i+n]==expected,stem)
ensure(not byte('safe_mode') and not byte('cold_lab_bad'),'guard or cold mode failed')
dbg=words('x0x_dbg',13)
ensure(dbg[0]==0x44424731 and dbg[1]>0 and len(pcm)%4==0 and pcm,'no audio')
skipped=re.findall(r'skipped (\d+) cycles',(out/'run.log').read_text())
ensure(skipped,'no skip witness')
xrun,=words('audio_xruns')
owner=json.loads((out/'owner.json').read_text())
base=sym['fm1_c1_mb']-ELF_RAM
dispatched,=struct.unpack_from('<I',r,base)
alive,finished,bad=struct.unpack_from('<III',r,base+64)
peak=max(abs(n) for n, in struct.iter_unpack('<h',pcm))
skip=max(map(int,skipped))
print(json.dumps(dict(worst_us=dbg[2],halves=dbg[1],raw_late=dbg[5],xruns=xrun,skipped_cycles=skip,
    peak=peak, frames=len(pcm)//4, sealed_boot=True, settled_owner_readbacks=owner['count'],
    meets_timing=dbg[2]<=10809 and dbg[5]==xrun==skip==0 and peak>0,
    core1=dict(enabled=words('fm1_c1_on')[0],gave_up=byte('fm1_c1_gave_up'),alive=alive,
      dispatched=dispatched,finished=finished,bad_mailbox=bad),
    accepted=False)))
