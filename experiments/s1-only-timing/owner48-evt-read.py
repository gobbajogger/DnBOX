#!/usr/bin/env python3
"""Read known-frame sequencer/trigger diagnostic values, not uninstrumented timing."""
import argparse,json,struct,subprocess
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument("run",type=Path);d=p.parse_args().run
assert "skipped 0 cycles" in (d/"run.log").read_text()
for ln in subprocess.check_output(["readelf","-sW",d/"bench.elf"],text=True).splitlines():
 w=ln.split()
 if len(w)>7 and w[7]=="bench_evt_rows" and w[6]!="UND":
  assert int(w[2])==144
  base=int(w[1],16)-0x1c00000
  break
else: raise SystemExit("event probe not in ELF")
mem=(d/"ram.bin").read_bytes()
keys=("half","seq_advance","drum_hook","drum_metadata","trigger_909","trigger_808",
      "step_hook","midi_hook","midi_clock_count","midi_note_count","hit_count")
rows={}
for i in range(3):
 frame,*r=struct.unpack_from("<12I",mem,base+i*48)
 assert frame in (129024,131072,171520) and r[0]
 rows[frame]={k:(v if k.endswith("_count") else round(v/24,3)) for k,v in zip(keys,r)}
print(json.dumps(rows,indent=2))
