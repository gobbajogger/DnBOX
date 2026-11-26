import ctypes as c, collections, hashlib, pathlib, struct, sys, zlib
from test_install import load_j6,s1_load_zip
from install_bank import sid,native_field
from lite_bank import encode_lite
class Stage(c.Structure):
 _fields_=[('words',c.c_uint32*220),('ids',c.c_uint8*220),('name',c.c_char*48),('n',c.c_uint)]
dll=c.CDLL(str(pathlib.Path(__file__).with_name('render.so')))
stage=dll.stage_program
dll.staged_adopt.argtypes=[c.POINTER(Stage),c.POINTER(c.c_uint64)]
dll.legacy_holdout.argtypes=[c.c_char_p,c.c_uint,c.c_uint]
dll.legacy_holdout.restype=c.c_uint64
old=pathlib.Path(__file__).with_name('old.pack').read_bytes()
H=hashlib.sha256()
renders={}
seen_uid=collections.Counter()
wet=[]
dll.late_reject.argtypes=[c.POINTER(Stage),c.c_char_p,c.POINTER(Stage),c.c_char_p,c.POINTER(c.c_uint64)]

stage.argtypes=[c.c_char_p,c.c_size_t,c.c_uint,c.c_char_p,c.c_char_p,c.c_char_p,c.c_uint,c.POINTER(Stage)]
side=dll.sidecar_pitch
side.argtypes=[c.POINTER(Stage),c.c_char_p,c.c_char_p,c.c_uint,c.POINTER(c.c_uint32),c.c_uint,c.POINTER(c.c_float)]
import zipfile, math, json
z=zipfile.ZipFile(sys.argv[2])
meta={row['uid']:row for f in z.namelist() if f.startswith('Synth one/') and f.endswith('.json') for row in json.loads(z.read(f))}
selected=set()
paths=[pathlib.Path(p) for p in sys.argv[1:3]]
assert [hashlib.sha256(p.read_bytes()).hexdigest() for p in paths]==['ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8','68ad8b5fa0adca8b5da2b03f9303e9321c6a20907f5945e3fe879bc6494009a4']
groups=collections.defaultdict(list)
for p in load_j6(paths[0])[0]+s1_load_zip(paths[1])[0]:
 groups[p.engine,p.bank].append(p)
def check(blob, tag, bid, pid, schema, okay):
 out=Stage()
 c.memset(c.byref(out),255,c.sizeof(out))
 rc=stage(blob,len(blob),tag,bid,pid,schema,len(schema),c.byref(out))
 assert rc==int(okay)
 if not okay: assert bytes(out)==bytes(c.sizeof(out))
 return out
def repaired(blob):
 struct.pack_into('<I',blob,8,zlib.crc32(blob[:8]+blob[12:]))
 return bytes(blob)
counts=collections.Counter(); negatives=0; bankhash=[]
for (e,b),ps in groups.items():
 blob=encode_lite(ps)
 tag={'Synth One':2,'SuperJ6':3}[e]; bid=sid('bank',e,ps[0].bank_uid or b)
 assert blob[16:24]==bid and blob[12]==tag
 seen=collections.Counter()
 for p in ps:
  seen[p.uid]+=1
  pid=sid('preset',e,p.bank_uid or b,p.uid,seen[p.uid])
  fields=[(native_field(e,k),w) for k,w in p.fields]
  schema=bytes(k for k,w in fields)
  out=check(blob,tag,bid,pid,schema,True)
  assert out.n==len(fields) and out.name.decode()==p.name
  assert list(zip(out.ids[:out.n],out.words[:out.n]))==fields
  if tag==2:
   h=c.c_uint64()
   before=dll.active_owner()
   assert dll.staged_adopt(c.byref(out),c.byref(h))==1 and h.value and dll.active_owner()!=before
   seen_uid[p.uid]+=1
   fields_f={k:struct.unpack("<f",struct.pack("<I",w))[0] for k,w in fields}
   m=meta[p.uid]; raw=m.get('tuningMasterSet')
   custom=raw and len(raw) in (3,19)
   if custom or raw is None or len(raw)==12:
    rr=sorted(abs(v)*2**(-math.floor(math.log2(abs(v)))) for v in (raw or []))
    equ=rr and len(rr)==12 and all(abs(x-2**(j/12))<=1e-6 for j,x in enumerate(rr))
    k=(2,len(raw)) if custom else (0,0) if raw is None else (1,12) if equ else None
    if k is not None and k not in selected:
     ID=bid+pid
     words=[struct.unpack('<I',struct.pack('<f',x))[0] for x in rr] if k[0]==2 else []
     a=(c.c_uint32*len(words))(*words); hz=(c.c_float*128)()
     assert side(c.byref(out),ID,bytes(16),k[0],a,len(words),hz)==0
     if words:
      q=a[0];a[0]=0x7fc00000
      assert side(c.byref(out),ID,ID,k[0],a,len(words),hz)==0
      a[0]=q
     assert side(c.byref(out),ID,ID,k[0],a,len(words),hz)==1
     if k[0]==0:rr=[1.125,1.25,1.5,1.75]
     a4=int(fields_f[143])
     a4=a4 if 410<=a4<=470 else 440
     ref=lambda n:a4*2**((n-69)/12) if k[0]==1 else min(22050.,rr[(n-60)%len(rr)]*2**((n-60)//len(rr))*a4*2**(-9/12))
     err=max(abs(1200*math.log2(hz[n]/ref(n))) for n in range(128))
     # Exact law on custom and inherited scale; ordinary native exp2 alternative has documented numeric tolerance.
     assert err < (.05 if k[0]==1 else .002),(k,m['name'],err)
     print('tuning',k,m['name'],'A4',a4,'all MIDI 0..127, peak cents',err,'boundaries',[(n,round(hz[n],5)) for n in (59,60,61,59+len(rr),60+len(rr))])
     selected.add(k)
   renders[p.uid]=h.value
   fields_f={k:struct.unpack('<f',struct.pack('<I',w))[0] for k,w in fields}
   if fields_f[37] and fields_f[40]>0 and fields_f[41] and fields_f[44]>0 and len(wet)<8:
    wet.append((out,bid+pid))
   H.update(struct.pack('<Q',h.value))
   saved_id=out.ids[0];out.ids[0]=255
   before=dll.active_owner()
   assert dll.staged_adopt(c.byref(out),c.byref(h))==0 and dll.active_owner()==before
   out.ids[0]=saved_id
  counts[e]+=1;counts['writes']+=out.n
 bad=bytearray(blob);bad[-1]^=1
 off=bytearray(blob);struct.pack_into('<H',off,44,65535)
 bad_schema=bytes([255])+schema[1:]
 tests=[(blob[:-1],tag,bid,pid,schema),(bytes(bad),tag,bid,pid,schema),(repaired(off),tag,bid,pid,schema),(blob,5-tag,bid,pid,schema),(blob,tag,bytes(8),pid,schema),(blob,tag,bid,bytes(8),schema),(blob,tag,bid,pid,bad_schema),(blob,tag,bid,pid,schema[:-1])]
 for a in tests:check(*a,False)
 negatives+=len(tests)
 bankhash.append(hashlib.sha256(blob).hexdigest())
print(dict(counts),len(groups),'banks',negatives,'rejected','derived bank hashes combined',hashlib.sha256(''.join(bankhash).encode()).hexdigest())

# Match the exact pinned legacy factory compilation's order and source IDs.
import json
base=pathlib.Path(sys.argv[3])
manifest=json.loads((base/'MANIFEST.json').read_bytes())['files']
programs=[]
for item in sorted(manifest,key=lambda f:f['name']):
 raw=(base/item['name']).read_bytes()
 assert hashlib.sha256(raw).hexdigest()==item['sha256']
 programs.extend(sorted(json.loads(raw),key=lambda p:p['position']))
matched=0
legacy_uid=collections.Counter(p['uid'] for p in programs)
for i,p in enumerate(programs):
 if i%53==0 and p['uid'] in renders:
  assert legacy_uid[p['uid']]==seen_uid[p['uid']]==1
  h=dll.legacy_holdout(old,len(old),i)
  assert h and h==renders[p['uid']], (i,p['name'],hex(h),hex(renders[p['uid']]))
  matched+=1
assert matched>=3
print('Synth One finite 4096-frame stereo scenarios',counts['Synth One'],'dispatch failures preserved active owner',counts['Synth One'],'old pack exact PCM hashes',matched,'cumulative signed PCM hashes',H.hexdigest())

assert len(wet)==8
for i,(x,key) in enumerate(wet):
 y,replacement=wet[(i+1)%len(wet)]
 h=c.c_uint64()
 rc=dll.late_reject(c.byref(x),key,c.byref(y),replacement,c.byref(h))
 assert rc==1,(i,rc)
 print('late rejection case',i,'continuation digest',format(h.value,'016x'))
print('identity + FX parity cases',len(wet),'post-reject stereo frames per case',sum(1+(i*47)%128 for i in range(64,192)),'legacy checked unique UID holdouts',matched)

assert selected=={(0,0),(1,12),(2,3),(2,19)}
