#!/usr/bin/env python3
"""Compare four raw tuning fields in each original record and C reader. Changes no loads."""
import argparse
import ctypes as C
import hashlib
import json
from collections import Counter
from pathlib import Path
import struct
import sys
from uuid import UUID

p = argparse.ArgumentParser()
p.add_argument("factory_source", type=Path)
p.add_argument("archive", type=Path)
p.add_argument("reader", type=Path)
a = p.parse_args()
sys.path.insert(0, str(a.factory_source))
from factory import load_factory

def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(a.archive) == "ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8"
cat = json.loads((a.factory_source.parent / "mapping/results/implementer_brief/public_control_catalog.json").read_text())
names = {r["identifier"]: r["id"] for r in cat["controls"]}
ids = {72, 73, 74, 75}
print("archive", sha(a.archive), "reader", sha(a.reader))

class Cursor(C.Structure):
    _fields_ = [(k, C.c_uint) for k in ("bit", "field", "end")]

lib = C.CDLL(str(a.reader.resolve()))
lib.sj6_select_position.argtypes = (C.c_uint, C.c_uint, C.POINTER(Cursor))
lib.sj6_select_uid.argtypes = (C.c_char_p, C.c_char_p, C.POINTER(Cursor))
lib.sj6_next_field.argtypes = (C.POINTER(Cursor), C.POINTER(C.c_uint), C.POINTER(C.c_uint32))
sums = {i: Counter() for i in ids}
positions = {i: Counter() for i in ids}
new_law = Counter()
rows = 0
f = load_factory(a.archive)
for bankidx,(uid,(_,progs)) in enumerate(sorted(f.items())):
    for pidx,orig in enumerate(sorted(progs.values(),key=lambda r:r["position"])):
        assert pidx == orig["position"]
        ref = [(j, names[r["identifier"]], struct.unpack("<I",struct.pack("<f",r["value"]))[0])
               for j,r in enumerate(orig["parameterValues"]) if names.get(r["identifier"]) in ids]
        assert len(ref) == 4 and {r[1] for r in ref} == ids
        cursor, byid = Cursor(), Cursor()
        assert lib.sj6_select_position(bankidx,pidx,C.byref(cursor))
        assert lib.sj6_select_uid(UUID(uid).bytes,UUID(orig["uid"]).bytes,C.byref(byid))
        assert bytes(cursor) == bytes(byid)
        slot,word = C.c_uint(),C.c_uint32()
        got, i = [],0
        while lib.sj6_next_field(C.byref(cursor),C.byref(slot),C.byref(word)):
            if slot.value in ids: got.append((i,slot.value,word.value))
            i += 1
        assert ref == got, (bankidx,pidx,ref,got)
        for j,k,w in got:
            positions[k][j] += 1
            sums[k][w] += 1
            if k == 72:
                u = struct.unpack("<f",struct.pack("<I",w))[0]
                x = 24 * u - 12
                new_law[int(x+.5) if x>=0 else -int(-x+.5)] += 1
        rows += 1
assert (len(f), rows) == (12,640)
for k in sorted(ids):
    print(f"id={k} rows={sums[k].total()} neutral={sums[k][0x3f000000]} unique={len(sums[k])} positions={dict(positions[k])}")
print("ID72 target semitones:", dict(sorted(new_law.items())))
print("PASS all four exact binary32 fields in all 640 authored records by indexed positions and original UUIDs; no firmware import claims")
