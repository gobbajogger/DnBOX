import ctypes as c
import json
import math
import random
import struct
import sys
r=random.Random(17003)
lib=c.CDLL(sys.argv[1])
cf=c.c_float
cu=c.c_uint
ca=c.POINTER(cf)
for f in (lib.step_contig,lib.step_split):
 f.argtypes=[cu,cu,cu,cf,ca,ca]
 f.restype=cf
lib.state_equal.argtypes=[cu]
lib.state_equal.restype=cu
shape=(cf*5)(.1,.5,.27,.9,.03)
frames=180000
active=frozen=denied=0
for p in range(2):
 for v in range(6):lib.reset(p,v)
for n in range(frames):
 p=r.randrange(2);v=r.randrange(6)
 if n%917==21:lib.reset(p,v)
 m=r.randrange(64)
 if n%59==0:v=6+r.randrange(4)
 if n%61==0:p=2
 g=(cf*4)(.004+r.random()*3,.00035606283927336335,(n%101)/100,1)
 x=12*(2*r.random()-1)
 if n%107<3:x=0.
 a,b=[f(p,v,m,x,g,shape) for f in (lib.step_contig,lib.step_split)]
 assert struct.pack('<f',a)==struct.pack('<f',b) and math.isfinite(a)
 if p>=2 or v>=6:
  denied+=1
  assert struct.pack('<f',b)==b'\0\0\0\x80'
 elif m&(1<<v):active+=1
 else:
  frozen+=1
  assert struct.pack('<f',b)==b'\0\0\0\x80'
 if n%307==0:
  assert lib.state_equal(0)==lib.state_equal(1)==1
assert lib.state_equal(0)==lib.state_equal(1)==1
print(json.dumps({'scientific_scope':'host state layout only', 'frames':frames,'active':active,'frozen':frozen,'invalid_owners':denied,'all_outputs_and_active_state_words_equal':True},indent=2))
