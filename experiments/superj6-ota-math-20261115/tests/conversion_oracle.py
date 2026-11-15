"""Independent arbitrary-integer binary64->binary32 RNE/gradual oracle."""
from pathlib import Path
import ctypes as C,random,itertools,json,hashlib,struct,sys
R=Path(__file__).resolve().parents[1]
class Result(C.Structure):_fields_=[('bits',C.c_uint32),('flags',C.c_uint32)]
lib=C.CDLL(sys.argv[1]);lib.f64_to_f32_evaluate.argtypes=[C.c_uint64];lib.f64_to_f32_evaluate.restype=Result

def round_even(n,shift):
 if shift<=0:return n<<-shift,0
 q,r=divmod(n,1<<shift);h=1<<(shift-1)
 return q+int(r>h or(r==h and(q&1))),r

def oracle(x):
 sign=x>>63;e=(x>>52)&2047;n=x&((1<<52)-1)
 if e==2047:
  if n:return 0x7fc00000,16 if not(n&(1<<51))else 0
  return (sign<<31)|0x7f800000,0
 if not(e or n):return sign<<31,0
 if e:n|=1<<52;p=e-1075
 else:p=-1074
 floor=n.bit_length()-1+p;qexp=floor-23
 tq,_=round_even(n,qexp-p);tiny=tq.bit_length()-1+qexp < -126
 quantum=max(qexp,-149);q,rem=round_even(n,quantum-p);flags=(1|(2 if tiny else 0))if rem else 0
 if not q:return sign<<31,flags
 floor=q.bit_length()-1+quantum
 if floor>127:return(sign<<31)|0x7f800000,5
 if floor>=-126:
  s=q>>(q.bit_length()-24)if q.bit_length()>=24 else q<<(24-q.bit_length());return(sign<<31)|((floor+127)<<23)|(s&0x7fffff),flags
 return(sign<<31)|q,flags

def pack64(n,p,sign=0):
 if not n:return sign<<63
 b=n.bit_length();e=b-1+p;assert -1022<=e<=1023
 assert b<=53;return(sign<<63)|((e+1023)<<52)|((n<<(53-b))&((1<<52)-1))

def decode32(x):
 e=(x>>23)&255;n=x&0x7fffff
 return(n|0x800000,e-150)if e else(n,-149)

def midpoint(x):
 n,p=decode32(x)
 if x==0x7f7fffff:m,k=1,128
 else:m,k=decode32(x+1)
 base=min(p,k);return pack64((n<<(p-base))+(m<<(k-base)),base-1)

count=0;groups={};dig=hashlib.sha256()
def test(x,g):
 global count
 want,flags=oracle(x);z=lib.f64_to_f32_evaluate(x)
 assert(z.bits,z.flags)==(want,flags),(g,hex(x),hex(z.bits),z.flags,hex(want),flags)
 dig.update(struct.pack('<QII',x,want,flags));count+=1;groups[g]=groups.get(g,0)+1
base=[0,1,2,0xfffffffffffff,0x10000000000000,0x368fffffffffffff,0x3690000000000000,0x3690000000000001,0x36a0000000000000,0x380fffffdfffffff,0x380fffffe0000000,0x380fffffe0000001,0x3810000000000000,0x3ff0000010000000,0x3ff0000010000001,0x3ff0000030000000,0x47efffffefffffff,0x47effffff0000000,0x47effffff0000001,0x7fefffffffffffff,0x7ff0000000000000,0x7ff0000000000001,0x7ff8000000000000,0x7fffffffffffffff]
for x in base:
 for s in[0,1<<63]:test(x|s,'named-boundaries')
for e in range(0,255):
 for f in[0,1,2,0x3fffff,0x7ffffc,0x7ffffd,0x7ffffe,0x7fffff]:
  x=(e<<23)|f
  if x>0x7f7fffff:continue
  mid=midpoint(x)
  for delta in[-2,-1,0,1,2]:
   for sign in[0,1<<63]:test((mid+delta)|sign,'every-F32-exponent-halfway')
rng=random.Random(0xf64032)
for i in range(500000):test(rng.getrandbits(64),'uniform')
for i in range(100000):
 x=rng.randrange(0x7f800000);mid=midpoint(x)
 for delta in[-1,0,1]:
  for sign in[0,1<<63]:test((mid+delta)|sign,'random-halfway')
# Every binary64 exponent at six significand patterns with both signs.
for e in range(2047):
 for f in[0,1,2,(1<<51)-1,1<<51,(1<<52)-1]:
  for sign in[0,1<<63]:test((e<<52)|f|sign,'every-F64-exponent')
out=dict(scope='Independent integer conversion oracle, not host/target/original FP execution',cases=count,groups=groups,seed='0xf64032',result_vector_sha256=dig.hexdigest(),passed=True);(R/'build/CONVERSION-ORACLE.private.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out,indent=2))
