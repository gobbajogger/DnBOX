#!/usr/bin/env python3
"""Measure completed halves and stale 512-frame-or-longer DMA repeats."""
from pathlib import Path
import hashlib
import json
import struct
import subprocess
import sys

elf, d=map(Path,sys.argv[1:3])
owner=int(sys.argv[3]) if len(sys.argv)>3 else 0
assert owner in (0,1)
syms={line.split()[-1]:int(line.split()[0],16)-0x01c00000
      for line in subprocess.check_output(['nm','-S',str(elf)],text=True).splitlines()
      if len(line.split()) in (3,4)}
r=(d/'ram.bin').read_bytes()
st=struct.unpack_from('<13I',r,syms['x0x_dbg'])
assert st[0]==0x44424731
raw=(d/'work/audio.raw').read_bytes()
p=[x[0] for x in struct.iter_unpack('<I',raw)]
log=(d/'run.log').read_text()
assert 'skipped 0 cycles' in log and 'work/lcd.ppm at 5300 ms' in log
assert 'flash: 0 erases / programs' in log, 'Unexpected cold initialization invalidated the saved project'
notes=sum(f'usb out: {0x91+owner:x} ' in l for l in log.splitlines())
state=syms['original_bass']
kind_status=[list(r[state+i*5344+5340:state+i*5344+5342]) for i in range(2)]
v=state+owner*5344+840
gates=[list(r[v+168*j+100:v+168*j+102]) for j in range(6)]
assert kind_status[owner]==[2,1] and kind_status[1-owner]==[0,1],kind_status
if notes:
    assert notes==6 and gates==[[n,1] for n in (48,52,55,59,62,67)],(notes,gates)
active_start=next((i for i,x in enumerate(p) if x),len(p))
runs=[]
start=None
for i in range(max(active_start+1024,1024),len(p)+1):
    same=i<len(p) and p[i]==p[i-1024]
    if same:
        if start is None: start=i
    elif start is not None:
        if i-start>=512 and any(p[start:i]):
            runs.append((start,i))
        start=None
split={}
if 'split_area' in syms:
    area=syms['original_workspace_b']
    words=struct.unpack_from('<1280I',r,area)
    o=2712//4
    split=dict(worker_gave_up=r[syms['fm1_c1_gave_up']],
               nonzero_worker_words=sum(i!=0 for i in words[o:]),
               last_engine_worker_mask=r[syms['eng_c1_mask']])
print(json.dumps(dict(**split,halves=st[1], peak_us=st[2], in_render=st[4],
       parity_late=st[5], last_completed_us=st[8], raw_frames=len(p),
       midi_notes=notes, kind_status=kind_status, synth_owner=owner,synth_gates=gates,
       stale_spans=len(runs), stale_frames=sum(b-a for a,b in runs),
       raw_sha256=hashlib.sha256(raw).hexdigest()),sort_keys=True))
