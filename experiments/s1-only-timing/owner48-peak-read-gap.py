#!/usr/bin/env python3
"""Decode the clock ledger for the three sampled 512-frame audio halves."""
from pathlib import Path
import argparse, json, struct, subprocess

p = argparse.ArgumentParser(description=__doc__)
p.add_argument("replay_dir", type=Path)
p.add_argument("--trigger-split", action="store_true")
a = p.parse_args()
d = a.replay_dir
if "skipped 0 cycles" not in (d/"run.log").read_text():
    raise SystemExit("Not a valid strict replay")
for line in subprocess.check_output(["readelf","-sW",d/"bench.elf"],text=True).splitlines():
    w=line.split()
    if len(w)>=8 and w[7]=="bench_gap_rows" and w[6]!="UND":
        off=int(w[1],16)-0x1c00000
        if int(w[2])!=3*13*4:
            raise SystemExit("Gap row size mismatch")
        break
else:
    raise SystemExit("Gap probe symbol missing")
ram=(d/"ram.bin").read_bytes()
labels=("half","engine","usb_volume","engine_prepare","sequencer","render_sub",
        "deferred_master","pcm","scope_meters","sub_setup","planner","sub_launch")
if a.trigger_split:
    labels=labels[:-3]+("trigger_count","trigger909","trigger808")
rows={}
for i in range(3):
    fr,*ticks=struct.unpack_from("<13I",ram,off+52*i)
    if fr not in (129024,131072,171520) or not ticks[0]:
        raise SystemExit("missing sampled half")
    rows[fr]={k:(v if k=="trigger_count" else round(v/24,3)) for k,v in zip(labels,ticks)}
print(json.dumps(rows,indent=2))
