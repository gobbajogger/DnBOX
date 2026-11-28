#!/usr/bin/env python3
"""Measure completed halves and stale 512-frame-or-longer DMA repeats."""
from pathlib import Path
import hashlib
import json
import struct
import subprocess
import sys

elf, d=map(Path,sys.argv[1:])
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
print(json.dumps(dict(halves=st[1], peak_us=st[2], in_render=st[4],
       parity_late=st[5], last_completed_us=st[8], raw_frames=len(p),
       midi_notes=sum('usb out: 91 ' in l for l in log.splitlines()),
       stale_spans=len(runs), stale_frames=sum(b-a for a,b in runs),
       raw_sha256=hashlib.sha256(raw).hexdigest()),sort_keys=True))
