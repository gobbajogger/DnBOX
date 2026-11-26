#!/usr/bin/env python3
"""Reorder original reference commands to the tested compact pack order, not by name."""
import argparse,collections,json,struct
from hashlib import sha256
from pathlib import Path
a=argparse.ArgumentParser()
for x in ("source_commands","candidate_roundtrip_expect","out_commands","out_identity"):
    a.add_argument(x,type=Path)
p=a.parse_args()
s=p.source_commands.read_bytes(); e=p.candidate_roundtrip_expect.read_bytes()
ident,n=struct.unpack_from("<2I",s)
assert ident==0x53315043
def row(b,off):
    count,=struct.unpack_from("<I",b,off)
    assert count==144
    return b[off:off+4+count*8],off+4+count*8
orig=[]; off=8
while off<len(s):
    u,off=row(s,off); orig.append(u)
assert off==len(s) and len(orig)==n
by=collections.defaultdict(list)
for i,u in enumerate(orig): by[u].append(i)
off=0; output=[]; names=[]; source_index=[]
while off<len(e):
    num=e[off]; name=e[off+1:off+1+num].decode("utf8"); off+=1+num
    u,off=row(e,off)
    assert by[u],f"unmatched decoded parameter row: {name}"
    output.append(u); names.append(name); source_index.append(by[u].pop(0))
assert off==len(e) and len(output)==n and not any(by.values())
p.out_commands.write_bytes(struct.pack("<2I",ident,n)+b"".join(output))
p.out_identity.write_text(json.dumps({"count":n,"exact_command_occurrences":n,"permuted_positions":sum(i!=j for i,j in enumerate(source_index)),
    "source_sha256":sha256(s).hexdigest(),"candidate_roundtrip_expect_sha256":sha256(e).hexdigest(),
    "aligned_sha256":sha256(p.out_commands.read_bytes()).hexdigest(),
    "rows":[{"label":name,"original_order":i} for name,i in zip(names,source_index)]},indent=2)+"\n")
print(f"All {n} command rows matched byte-for-byte; {sum(i!=j for i,j in enumerate(source_index))} positions differ.")
