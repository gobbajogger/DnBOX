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
import zlib
from pack_synthone_factory import Bits
assert len(oldrows)==694
old_idx=collections.defaultdict(list)
for i,(name,raw) in enumerate(oldrows): old_idx[p(raw)].append(i)
programs=[(name,[(i,f32(w)) for i,w in raw]) for name,raw in oldrows]
banks=[]
cur=32
for _ in range(oldblob[10]):
 first,count,pos=struct.unpack_from("<HHB",oldblob,cur)
 end=oldblob.index(0,cur+5)
 banks.append((oldblob[cur+5:end].decode(),first,count,pos))
 cur=end+1
assert len(banks)==12
extra=[]; appended={}; picker=[]; meta=[]; uids=collections.defaultdict(set); old_alias=0; amb=0
lookup={(r.bank,r.pos,r.uid,r.name):r for r in rows}
with zipfile.ZipFile(ARCHIVE) as z:
 for fn in z.namelist():
  if not (fn.startswith('Synth one/') and fn.endswith('.json')): continue
  bank=pathlib.Path(fn).stem
  app=json.loads(z.read(fn))
  for a,obj in enumerate(app):
   # Preserve ZIP directory order and exactly the serialized original array order, not position-sort.
   pos,uid,label=obj["position"],str(obj["uid"]),obj["name"]
   r=lookup[(bank,pos,uid,label)]; raw=p(r.fields); uids[uid].add(raw)
   if raw in old_idx:
    same_name=[i for i in old_idx[raw] if oldrows[i][0]==label]
    idx=min(same_name or old_idx[raw])
    old_alias+=1; amb+=len(old_idx[raw])>1
   else:
    if raw not in appended:
     appended[raw]=694+len(extra); extra.append((label,[(int(i),f32(w)) for i,w in r.fields],raw))
    idx=appended[raw]
   assert idx<1024
   picker.append(idx)
   meta.append([bank,a,pos,uid,label])  # never merged, private witness in RAM only
assert len(meta)==len(set(tuple(x) for x in meta))==906 and len(extra)==213 and old_alias==693
assert sum(len(contents)>1 for contents in uids.values())==11
assert 43 not in picker and all(p(oldrows[43][1])!=raw for _,_,raw in extra)
banks.append(("Detached payload extension",694,len(extra),0))
programs.extend((name,commands) for name,commands,_ in extra)
blob,sizes=pack(programs,banks,order)
assert len(programs)==lib.s1pack_programs(blob,len(blob))==907
decoded=[decode(blob,i) for i in range(907)]
assert decoded[:694]==oldrows
assert all(p(decoded[694+i][1])==x[2] for i,x in enumerate(extra))
bits=Bits()
for idx in picker: bits.put(idx,10)
packed=bits.v
assert len(packed)==1133
def unpack10(buf,i):
 return sum(((buf[(i*10+b)//8]>>(7-(i*10+b)%8))&1)<<(9-b) for b in range(10))
assert [unpack10(packed,i) for i in range(906)]==picker
for j,(bank,a,pos,uid,label) in enumerate(meta):
 assert p(decoded[unpack10(packed,j)][1])==p(lookup[(bank,pos,uid,label)].fields)
all_meta=json.dumps(meta,ensure_ascii=False,separators=(",",":")).encode()
labels=b"".join(m[4].encode()+b"\0" for m in meta)
out={"legacy_first_pack_bytes":len(blob),"picker_10bit_bytes":len(packed),"pack_plus_picker_bytes":len(blob)+len(packed),
     "legacy_ordinals_unchanged":694,"append_unique_payloads":213,"ordered_pairs_checked":907*144,
     "picker_archive_order_identity_rows":len(meta),"picker_legacy_references":old_alias,"same_content_ambiguous_picker_references":amb,
     "distinct_content_reused_uid_groups":11,"legacy_only_preserved_ordinal":44,
     "standalone_uid_label_bank_position_index_json_bytes":len(all_meta),"standalone_identity_json_zlib_host_bytes":len(zlib.compress(all_meta,9)),
     "standalone_label_utf8_nul_bytes":len(labels),"standalone_label_zlib_host_bytes":len(zlib.compress(labels,9)),
     "section_bytes":sizes}
pair=next(indices[:2] for indices in old_idx.values() if len(indices)>1 and oldrows[indices[0]][0]!=oldrows[indices[1]][0])
(OUT/"aliases.bin").write_bytes(struct.pack("<HH",*(i+1 for i in pair)))
(OUT/"combined.pack").write_bytes(blob);(OUT/"check.bin").write_bytes(b"".join(p(row) for _,row in decoded)+b"".join(struct.pack("<H",i) for i in range(694)))
print(json.dumps(out,sort_keys=True))
