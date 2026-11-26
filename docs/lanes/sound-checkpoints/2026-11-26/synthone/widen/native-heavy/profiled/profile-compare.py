#!/usr/bin/env python3
"""Scope the sampling perturbation before using a separately profiled replay."""
import argparse
from bisect import bisect_right
from collections import Counter, defaultdict
from hashlib import sha256
from pathlib import Path
import json
import struct
import subprocess

ap=argparse.ArgumentParser()
ap.add_argument("elf",type=Path)
ap.add_argument("baseline",type=Path)
ap.add_argument("profiled",type=Path)
a=ap.parse_args()
def digest(p):
    return sha256(p.read_bytes()).hexdigest()
def read(p,name):
    return json.loads((p/name).read_text())
bins=defaultdict(list)
for line in subprocess.check_output(["readelf","-sW",str(a.elf)],text=True).splitlines():
    f=line.split()
    if len(f)>=8 and f[3]=="FUNC" and f[6]!="UND" and int(f[2]):
        start=int(f[1],16)
        bins[start].append((int(f[2]),f[7]))
starts=sorted(bins)
counts=defaultdict(Counter)
for line in (a.profiled/"profile.csv").read_text().splitlines():
    core,pc,weight=line.split()
    pc=int(pc,16)
    index=bisect_right(starts,pc)-1
    candidates=[]
    for start in reversed(starts[:index+1]):
        if candidates and start<candidates[0][2]-8:
            break
        candidates.extend((size,name,start) for size,name in bins[start] if pc-start<size)
        if pc-start>65536:
            break
    _,name,_=min(candidates,default=(0,"unresolved",0))
    counts[core][name]+=int(weight)
def read_samples(p):
    return [sample[0] for sample in struct.iter_unpack("<h",p.read_bytes())]
raw=[p/"audio.s16le" for p in (a.baseline,a.profiled)]
old,new=map(read_samples,raw)
assert len(old)==len(new) and len(old)%2==0
neq=[i for i,(l,r) in enumerate(zip(old,new)) if l!=r]
results=[read(p,"result.json") for p in (a.baseline,a.profiled)]
owners=[read(p,"owner.json")["owners"] for p in (a.baseline,a.profiled)]
nor=[digest(p/"disposable.bin") for p in (a.baseline,a.profiled)]
scene=[digest(p/"scene.keys") for p in (a.baseline,a.profiled)]
assert nor[0]==nor[1] and scene[0]==scene[1]
def no_times(seq):
    return [{k:v for k,v in row.items() if k!="ms"} for row in seq]
print(json.dumps({
    "scope":"Sampled replay scaled PC weights only; not per-block cycles or proof of an unchanged replay timeline.",
    "elf_sha256":digest(a.elf),"seed_sha256":nor[0],"event_tape_sha256":scene[0],
    "unprofiled":results[0],"profiled":results[1],
    "owner_values_equal":no_times(owners[0])==no_times(owners[1]),
    "owner_times_equal":owners[0]==owners[1],
    "readback_ms":[[o["ms"] for o in seq] for seq in owners],
    "pcm_sha256":[digest(p) for p in raw],"pcm_equal":not neq,
    "different_pcm_words":len(neq),
    "first_different_stereo_frame":neq[0]//2 if neq else None,
    "can_attribute_unprofiled_deadline":False,
    "pc_report_sha256":digest(a.profiled/"profile.csv"),
    "sampled_top_functions":{k:{"total_weight":sum(c.values()),"top":[[key,w,100*w/sum(c.values())] for key,w in c.most_common(25)]} for k,c in counts.items()}
},indent=2))
