"""Offline NOR model for a directory primary + mutually exclusive slot-2 shadow.
Does not access, install, or update firmware or a real device.
"""
import struct,zlib
PAGE=256
SZ=4096
ERASED=b'\xff'*SZ
def wrap(gen,data):
 h=struct.pack('<4sIH',b'DBD4',gen,len(data))
 return h+struct.pack('<I',zlib.crc32(h+data))+data
def parse(data):
 sig,g,n,crc=struct.unpack_from('<4sIHI',data)
 if sig!=b'DBD4' or not g or not n or n>510:return
 result=data[14:14+n]
 if zlib.crc32(data[:10]+result)==crc:return g,result
def choose(primary,shadow):
 q=[r for p in (primary,shadow) if (r:=parse(p))]
 if not q:raise ValueError('no directory')
 if len(q)==2 and q[0][0]==q[1][0] and q[0]!=q[1]:raise ValueError('generation conflict')
 return max(q)
def commit(address,value):
 yield address,0,ERASED
 # Size/payload/CRC first. Tag is programmed only after complete verification.
 for i in range(4,len(value),PAGE):
  yield address,i,value[i:i+PAGE]
 yield address,0,value[:4]
def advance(old,new):
 return list(commit(1,new))+list(commit(0,new))
def op(storage,i,o,d):
 if len(d)==SZ:
  assert not o and d==ERASED;storage[i]=d;return
 src=storage[i]
 assert all(a&b==b for a,b in zip(src[o:o+len(d)],d))
 storage[i]=src[:o]+d+src[o+len(d):]
def restore_primary(p,s):
 g,body=choose(p,s)
 if parse(p)==(g,body):return []
 return list(commit(0,wrap(g,body)))
def run():
 a,b=bytes(range(256))*1, bytes(range(256))*1+bytes(254)
 pold=wrap(1,a); pnew=wrap(2,b); pad=lambda v:v+ERASED[len(v):]
 strokes=advance(pold,pnew);cases=0
 before=[pad(pold),ERASED]
 for idx,(part,off,data) in enumerate(strokes):
  for cut in range(len(data)+1):
   state=before.copy()
   # Model arbitrary prefix cut within NOR program. Erase can be torn; prefix
   # erase produces 1 bits only and does not alter the other durable page.
   if len(data)==SZ:state[part]=data[:cut]+state[part][cut:]
   else:op(state,part,off,data[:cut])
   selection=choose(*state)
   if idx>=3 and parse(state[1])==(2,b): assert selection==(2,b)
   else: assert selection==(1,a)
   for args in restore_primary(*state):op(state,*args)
   assert parse(state[0])==selection
   # A sample may now erase/reuse the shared backup. Durable primary retains
   # exact selected directory, regardless of interrupted FSMP backup/write.
   state[1]=b'FSMP'+ERASED[4:]
   assert choose(*state)==selection
   cases+=1
  op(before,part,off,data)
 assert parse(before[0])==parse(before[1])==(2,b)
 # FSMP remains untouched during an already-pending sample rollback, and
 # ambiguous equal-generation conflicting durable metadata is rejected.
 fsmp=b'FSMP'+ERASED[4:]
 assert choose(pad(pold),fsmp)==(1,a)
 try:choose(pad(pold),pad(wrap(1,b)))
 except ValueError:pass
 else:raise AssertionError('ambiguous retry')
 print({'byte_prefix_power_cuts':cases,'result':'old or new directory intact, primary normalized before simulated sample use','physical_gain':4096,'conditional_payload':14*4096,'remaining_original_constraints':'must implement exclusivity, native installer compatibility and sample path recovery'})
if __name__=='__main__':run()
