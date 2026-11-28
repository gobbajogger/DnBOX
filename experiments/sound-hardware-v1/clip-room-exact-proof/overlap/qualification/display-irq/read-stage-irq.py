#!/usr/bin/env python3
from pathlib import Path
import json, struct, subprocess, sys
elf,run=map(Path,sys.argv[1:3]); nm=sys.argv[3] if len(sys.argv)>3 else 'nm'
addr={ln.split()[-1]:int(ln.split()[0],16)-0x1c00000 for ln in subprocess.check_output([nm,'-S',str(elf)],text=True).splitlines() if len(ln.split()) in (3,4)}
raw=(run/'ram.bin').read_bytes()
assert len(raw)==0x100000
log=(run/'run.log').read_text()
assert 'skipped 0 cycles' in log and 'work/lcd.ppm at 5300 ms' in log
assert log.count('usb out: 92 ') == 6 and 'flash: 0 erases / programs' in log
a=addr['stage_irq_trace']
magic,last_t,last_span,active_phase=struct.unpack_from('<4I',raw,a)
assert magic==0x53544952
period=512*1_000_000/44100
parts=('other','pack','unpack'); out={}
for i,name in enumerate(parts):
    starts,wall,wall_max,active,active_max,source_masked,n,t,mx,late,gap_n,gap_max,ln,ll,lt,lm=struct.unpack_from('<16I',raw,a+16+64*i)
    out[name]=dict(starts=starts,source_masked=source_masked,wall_mean_us=wall/max(starts,1)/24,wall_max_us=wall_max/24,ex_alnk_mean_us=active/max(starts,1)/24,ex_alnk_max_us=active_max/24,alnk_preemptions=n,alnk_all_us=t/24,alnk_max_us=mx/24,late=late,arrivals_after_fast_half=gap_n,largest_fast_arrival_gap_us=gap_max/24,largest_fast_arrival_excess_us=gap_max/24-period if gap_n else None,loaded_entries=ln,loaded_late=ll,loaded_mean_us=lt/max(ln,1)/24,loaded_max_us=lm/24)
dbg=struct.unpack_from('<13I',raw,addr['x0x_dbg'])
assert dbg[0]==0x44424731
print(json.dumps(dict(stages=out,completed_halves=dbg[1],sampled_peak_us=dbg[2],late_halves=dbg[5],tail_phase=active_phase,expected_interrupt_us=period),indent=2))
