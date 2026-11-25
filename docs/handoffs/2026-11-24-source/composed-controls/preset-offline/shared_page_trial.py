"""Offline slotted shared flash-sector trial. Nothing is installed or exported."""
import sys,struct,zlib,json,hashlib,math
from pathlib import Path
from dataclasses import dataclass
p=Path(__file__).resolve()
d=p.parent
sys.path.insert(0,str(p.parent/'source'))
s=(d/'shared_dict.py').read_text()
s=s.replace("sys.path.insert(0,str(pathlib.Path(__file__).parent/'sources/source'))","")
s=s.replace('def frequent(banks,name,minimum):','def frequent(banks,name,minimum,witness):',1)
needle='    extents[target]=encode_bank(e,bid,fresh)'
assert s.count(needle)==1
s=s.replace(needle,needle+'\n    witness(z,dict(extents,**{}),banks,target,old,extents[target],decode_bank,fresh)',1)
codec={"__file__":str(d/'shared_dict.py'),"__name__":"codec_trial"}
exec(compile(s,str(d/'shared_dict.py'),"exec"),codec)
S=4096
D=256
def ceil(n,a):return (n+a-1)//a*a
def sim(model,replaced,banks,target,old,new,decode,fresh):
 orig=dict(replaced);orig[target]=old
 handles={};buf=bytearray()
 def append(key,data):
  off=ceil(len(buf),4);buf.extend(b'\xff'*(off-len(buf)))
  handles[key]=(off,len(data),zlib.crc32(data));buf.extend(data)
 def read(off,n,crc,storage):
  v=bytes(storage[off:off+n]);assert len(v)==n and zlib.crc32(v)==crc
  return v
 def directory(version,extents):
  body=json.dumps(sorted((k,*v)for k,v in extents.items()),separators=(',',':')).encode()
  hdr=struct.pack('<4sIII',b'DNP4',version,len(body),zlib.crc32(body))
  assert len(hdr)+len(body)<=S-D
  return hdr+body
 def encode_dir(version,extents):
  body=directory(version,extents)
  return body+b'\xff'*(S-D-len(body))+struct.pack('<II',version,zlib.crc32(body))+b'\xff'*(D-8)
 def choose(slots):
  options=[]
  for a in slots:
   if len(a)!=S:continue
   magic,g,n,crc=struct.unpack_from('<4sIII',a)
   if magic!=b'DNP4' or n>S-D-16:continue
   v,c=struct.unpack_from('<II',a,S-D)
   body=a[16:16+n]
   if v!=g or zlib.crc32(body)!=crc or zlib.crc32(a[:16+n])!=c:continue
   try:mat={k:(o,n,c)for k,o,n,c in json.loads(body)}
   except ValueError:continue
   options.append((g,mat))
  return max(options,key=lambda q:q[0]) if options else None
 ids={k:banks[k][0].hex() for k in banks}
 append('model',b'\0'*16+model)
 for k,data in orig.items():append(ids[k],data)
 initial=len(buf); baseline=dict(handles)
 first=encode_dir(1,baseline); stale=bytes([255])*S
 assert choose([first,stale])==(1,baseline)
 for k in orig:
  assert decode(k[0],banks[k][0],read(*baseline[ids[k]],buf))==banks[k][1]
 append(ids[target],new)
 second=encode_dir(2,handles)
 def writes(data):
  # Journal data pages first. Commit record is the final programmed word(s).
  for i in range(0,S,D):yield i,data[i:i+D]
 attempts=0
 for cut in range(len(second)+1):
  # Model all torn writes at byte granularity in the inactive directory.
  trial=second[:cut]+stale[cut:]
  chosen=choose([first,trial])
  # Expected only one old or fully valid new generation; never a mixture.
  assert chosen in ((1,baseline),(2,handles))
  if chosen[0]==2:
   assert read(*chosen[1][ids[target]],buf)==new
  else:
   assert read(*chosen[1][ids[target]],buf)==old
  attempts+=1
 # Removing the target only changes the next journal pointer: its data
 # becomes stale (not erased) until an independent safe GC pass.
 after={k:v for k,v in handles.items() if k!=ids[target]}
 assert choose([encode_dir(3,after),second])==(3,after)
 assert decode(target[0],banks[target][0],read(*handles[ids[target]],buf))==fresh
 for k in orig:
  if k!=target:assert read(*after[ids[k]],buf)==orig[k]
 print(json.dumps(dict(data_initial_bytes=initial, data_initial_sector_bytes=ceil(initial,S),
    data_initial_sector_count=ceil(initial,S)//S, with_ab_directory_bytes=ceil(initial,S)+2*S,
    update_append_bytes=len(buf)-initial, transient_data_sector_bytes=ceil(len(buf),S),
    verified_presets=sum(len(x[1])for x in banks.values()),torn_directory_cases=attempts,
    shipping=False,reclaim_note='GC and bounded target decoder not implemented'),indent=2))
if __name__ == '__main__':
 j=codec['load_j6'](sys.argv[1])[0];s1=codec['s1_load_zip'](sys.argv[2])[0]
 assert len(j)==640 and len(s1)==906
 bank=codec['grouped'](j+s1)
 assert len(bank)==25
 bench=codec['frequent'](bank,'lzma',2,sim)
 print(json.dumps({k:v for k,v in bench.items() if k!='banks'}))
