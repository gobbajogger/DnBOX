"""Decode the single worst observed half from a distinct instrumented native ELF."""
import hashlib,json,re,struct,subprocess,sys
from pathlib import Path
elf,ram,keys=map(Path,sys.argv[1:])
sym={}
for l in subprocess.check_output(['readelf','-sW',elf],text=True).splitlines():
 c=l.split()
 if len(c)>=8 and c[2].isdigit() and c[6]!='UND':sym[c[7]]=(int(c[1],16),int(c[2]))
r=ram.read_bytes()
n,sz=sym['lab_morph_trace']; assert sz==68
row=struct.unpack_from('<17I',r,n-0x01c00000)
half,us,ms,maskset,spans,*metrics=row
dbg,sz=sym['x0x_dbg']; d=struct.unpack_from('<13I',r,dbg-0x01c00000)
assert 0<half<=d[1] and us==d[2] and 0<ms<=8000 and spans>0
ev=[k.partition(':') for k in keys.read_text().strip().split(',')]; known=[{"ms":int(m),"key":k} for m,_,k in ev if ms-(us+999)//1000<=int(m)<=ms]
ticks=metrics[:10]; waits=metrics[10:]
tot=[sum(ticks[i:i+5]) for i in (0,5)]
print(json.dumps({'scope':'Instrumentation build, record worst half only. Timer includes IRQs. Recorded job span excludes 808 body split outside run_job, other orchestration, bus FX/master, PCM conversion and interrupt activity. Both core sums overlap; do not add.',
 'elf_sha256':hashlib.sha256(elf.read_bytes()).hexdigest(),'half':half,'us':us,'latched_ms':ms,'raw_late_total':d[5],
 'plan_masks':[hex(i) for i in range(32) if maskset&(1<<i)],'stretches':spans,
 'job_labels':['909','808','A-Synth-One','B-Chicago','BREAK'],
 'execution_us_by_core':[[t/24 for t in ticks[i:i+5]] for i in (0,5)],
 'execution_us_total_per_core':[t/24 for t in tot],
 'ordered_mix_wait_us_core0_1':[w/24 for w in waits],
 'approx_events_in_same_wall_window':known},indent=2))
