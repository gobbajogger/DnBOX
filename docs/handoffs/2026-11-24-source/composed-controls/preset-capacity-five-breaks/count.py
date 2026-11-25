"""Offline exact whole-bank resident count with all five BREAK loops held fixed."""
from collections import defaultdict
from functools import lru_cache
import argparse
import json
from pathlib import Path
import sys
import zipfile

p=argparse.ArgumentParser(description=__doc__)
p.add_argument("helpers",type=Path)
p.add_argument("superj6",type=Path)
p.add_argument("synthone",type=Path)
a=p.parse_args()
sys.path.insert(0,str(a.helpers))
from install_bank import encode, parse_j6, parse_s1

banks=[]
for path,engine in ((a.superj6,"SuperJ6"),(a.synthone,"Synth One")):
    with zipfile.ZipFile(path) as z:
        for member in sorted(z.namelist()):
            if member.startswith("__MACOSX/"): continue
            if engine=="SuperJ6" and not member.endswith(".superj6.bank.json"): continue
            if engine=="Synth One" and not (member.startswith("Synth one/") and member.endswith(".json")): continue
            raw=z.read(member)
            rows=parse_j6(raw) if engine=="SuperJ6" else parse_s1(raw,Path(member).stem)
            if not rows: continue
            payload=encode(rows)
            sectors=(len(payload)+32+4095)//4096
            banks.append((engine,rows[0].bank,len(rows),len(payload),sectors))
assert len(banks)==25 and sum(r[2] for r in banks)==1546

def best(extents,reserve=0,need_mask=0):
    @lru_cache(None)
    def solve(i,remainders,masks):
        if i==len(banks): return (-10**9,()) if masks else (0,())
        no=solve(i+1,remainders,masks)
        engine,_,count,_,need=banks[i]
        flag=1 if engine=="Synth One" else 2
        if reserve and need>reserve: return no
        for j,remain in enumerate(remainders):
            if remain<need: continue
            r=list(remainders);r[j]-=need
            n,sel=solve(i+1,tuple(sorted(r)),masks & ~flag)
            if n+count>no[0]:
                no=(n+count,(i,)+sel)
        return no
    alloc=list(extents)
    if reserve:
        j=alloc.index(max(alloc))
        if alloc[j]<reserve: return 0,()
        alloc[j]-=reserve
    n,ix=solve(0,tuple(sorted(alloc)),need_mask)
    return (n,ix) if n>=0 else (0,())
def result(r):
    count,ix=r
    return {"count":count,"banks":[dict(zip(("engine","bank","presets","bytes","sectors"),banks[i])) for i in ix]}
out=[]
for n in (1,2,3):
    ext=[2]*n
    out.append({"conditional_payload_KiB":8*n,
                "static_whole_banks":result(best(ext)),
                "same_size_single_bank_replace_reserve":result(max((best(ext,k) for k in (1,2)),key=lambda r:r[0])),
                "both_engines_same_size_replace_reserve":result(max((best(ext,k,3) for k in (1,2)),key=lambda r:r[0]))})
print(json.dumps({"total_banks":len(banks),"total_presets":sum(r[2] for r in banks),"shared_layout_with_five_BREAK_loops":out},indent=2))
