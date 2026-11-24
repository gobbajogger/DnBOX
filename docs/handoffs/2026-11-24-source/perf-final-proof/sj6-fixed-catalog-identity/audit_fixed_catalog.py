#!/usr/bin/env python3
"""Pin exact Build9 identity and metadata; print aggregate digest, not the payload."""
import argparse, hashlib, json, sys
from pathlib import Path
from uuid import UUID
p=argparse.ArgumentParser()
p.add_argument("factory_source",type=Path)
p.add_argument("archive",type=Path)
a=p.parse_args()
assert hashlib.sha256(a.archive.read_bytes()).hexdigest()=="ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8"
sys.path.insert(0,str(a.factory_source))
from factory import load_factory
f=load_factory(a.archive)
records=[]
banks=set()
for uid,(m,b) in sorted(f.items(),key=lambda r:r[1][0]["midiBankChangeNumber"]):
    number=m["midiBankChangeNumber"]
    assert number not in banks
    banks.add(number)
    for position,row in enumerate(sorted(b.values(),key=lambda r:r["position"])):
        assert position==row["position"]
        records.append([len(records),str(UUID(uid)),number,m["name"],str(UUID(row["uid"])),position,row["name"],row["programChangeNumber"]])
assert len(records)==640 and len(banks)==12
assert len({(r[1],r[4]) for r in records})==640
assert sorted(banks)==[0]+list(range(2,13))
blob=json.dumps({"schema":"dnbox.sj6.build9.fixed.identity.v1","records":records},ensure_ascii=False,separators=(",",":"),sort_keys=True).encode("utf-8")
print("640 immutable original (bank UUID, program UUID) pairs; bank MIDI number and authored program position order")
print("identity-only manifest byte length",len(blob),"SHA-256",hashlib.sha256(blob).hexdigest())
print("Canonical ordered manifest must be separately pinned and provider authenticated; this is not program data.")
