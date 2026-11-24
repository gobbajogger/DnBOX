#!/usr/bin/env python3
import collections, ctypes as c, hashlib, importlib.util, io, json, pathlib, struct, subprocess, sys, zipfile
SOURCE,ARCHIVE,LAB,OLD,OUT=map(pathlib.Path,sys.argv[1:])
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(ARCHIVE)=="68ad8b5fa0adca8b5da2b03f9303e9321c6a20907f5945e3fe879bc6494009a4"
assert sha(LAB)=="efb086a21c26d20ffadef891ebe7c8e4e6e2070bb0f677dce059bf4a1d42f843"
assert sha(OLD)=="c6c39e8425e51ede8e46b94b7272a7c96e2987894f2e9c0b598d312c7fe04d44"
sys.dont_write_bytecode=True
with zipfile.ZipFile(LAB) as lab:
 for n in ("bench_corpus.py","benchmark.py"):
  (OUT/n).write_bytes(lab.read(n))
sys.path[:0]=[str(OUT),str(SOURCE/"tools")]
from bench_corpus import s1_load_zip
from pack_synthone_factory import pack
rows,stats=s1_load_zip(ARCHIVE)
assert len(rows)==906 and len(stats["banks"])==13 and stats["missing_defaults"]==0
p=lambda fields: b"".join(struct.pack("<BI",int(i),w) for i,w in fields)
f32=lambda w: struct.unpack("<f",struct.pack("<I",w))[0]
order=[int(i) for i,_ in rows[0].fields]
assert len(order)==len(set(order))==144 and all([int(i) for i,_ in r.fields]==order for r in rows)
oldblob=OLD.read_bytes()
subprocess.run(["cc","-O2","-shared","-fPIC",SOURCE/"firmware/src/dsp/s1pack/s1pack.c","-o",OUT/"codec.so"],check=True)
lib=c.CDLL(str(OUT/"codec.so")); CB=c.CFUNCTYPE(None,c.c_void_p,c.c_uint,c.c_float)
lib.s1pack_programs.argtypes=[c.c_char_p,c.c_uint]
lib.s1pack_read.argtypes=[c.c_char_p,c.c_uint,c.c_char_p,CB,c.c_void_p]
assert lib.s1pack_programs(oldblob,len(oldblob))==694
def decode(blob,i):
 raw=[]
 callback=CB(lambda _,j,v:raw.append((j,struct.unpack("<I",struct.pack("<f",v))[0])))
 name=c.create_string_buffer(48)
 assert lib.s1pack_read(blob,i,name,callback,None)==144
 return name.value.decode(),raw
oldrows=[decode(oldblob,i) for i in range(694)]
old_index=collections.defaultdict(list)
for i,(_,raw) in enumerate(oldrows): old_index[p(raw)].append(i)
uploads=collections.defaultdict(list)
programs=[]; records=[]; banks=[]; private=[];uids=collections.defaultdict(set)
with zipfile.ZipFile(ARCHIVE) as zz:
 for bank,count in stats["banks"].items():
  arr=json.loads(zz.read("Synth one/"+bank+".json"))
  lo=len(programs)
  items=[r for r in rows if r.bank==bank]
  by_array=sorted(enumerate(arr),key=lambda x:x[1]["position"])
  assert len(items)==len(by_array)==count
  first=items[0].pos
  banks.append((bank,lo,count,first))
  for (array_index,obj),r in zip(by_array,items):
   assert (str(obj["uid"]),obj["position"],obj["name"])==(r.uid,r.pos,r.name)
   raw=p(r.fields); idx=len(programs)
   private.append((bank,array_index,r.pos,r.uid,idx)); uids[r.uid].add(raw)
   program=(r.name,[(int(i),f32(w)) for i,w in r.fields])
   programs.append(program); records.append(raw); uploads[raw].append(idx)
absent=[i for i, (_,data) in enumerate(oldrows) if p(data) not in uploads]
assert len(absent)==1
missing=absent[0]
last_name,last_raw=oldrows[missing]
banks.append(("Retained legacy-only",len(programs),1,0))
programs.append((last_name,[(i,f32(w)) for i,w in last_raw])); records.append(p(last_raw))
blob,sizes=pack(programs,banks,order)
assert lib.s1pack_programs(blob,len(blob))==907
for i,(wanted,_) in enumerate(programs):
 name,raw=decode(blob,i)
 assert name==wanted and p(raw)==records[i]
# Test-executable baseline route: existing old ordinal is kept in its own pack.
# Each old ordinal maps only for cross-audio comparison, not migration or identity assignment.
old_to_new=[uploads[p(data)][0] if i!=missing else 906 for i,(_,data) in enumerate(oldrows)]
assert len(private)==len({(b,a,z,u) for b,a,z,u,_ in private})==906
assert sum(len(rows)-1 for rows in old_index.values())==19
assert sum(len(rows)-1 for rows in uploads.values())==19
assert sum(len(rows)>1 for rows in uids.values())==11
assert sum(len(indices) for words,indices in uploads.items() if words in old_index)==693
assert sum(len(indices) for words,indices in uploads.items() if words not in old_index)==213
(OUT/"union.pack").write_bytes(blob)
(OUT/"check.bin").write_bytes(b"".join(records)+b"".join(struct.pack("<H",i) for i in old_to_new))
print(json.dumps({"authenticated_uploads":906,"original_banks":13,"preserved_identity_rows":906,"retained_distinct_legacy_rows":1,"retained_legacy_base_ordinal":missing+1,"detached_union_rows":907,"verified_exact_id_binary32_pairs":907*144,"identity_dedupe":0,"distinguish_reused_uid_groups":11,"pack_694_bytes":len(oldblob),"pack_907_bytes":len(blob),"pack_907_sections":sizes,"name_bytes_max":max(len(name.encode()) for name,_ in programs)},sort_keys=True))
