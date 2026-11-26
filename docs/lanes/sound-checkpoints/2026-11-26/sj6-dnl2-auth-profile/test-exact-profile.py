"""Synthetic codec/authority boundaries only; no claim these are authored banks."""
import ctypes as C, hashlib, pathlib, random, struct, subprocess, tempfile
from encode import encode
D=pathlib.Path(__file__).resolve().parent
P=C.c_void_p
class Plan(C.Structure):
 _fields_=[('base',C.c_uint32),('has',C.c_uint8),('profile',C.c_uint8)]
TRUST=C.CFUNCTYPE(C.c_int,P,P,P,C.c_size_t)
ref=struct.unpack('<I',struct.pack('<f',.3345831036567688))[0]
g=[(28,0),(107,0),(108,0),(109,0),(180,ref)]
fixtures=[g,
 [x for x in g if x[0]!=109],
 [(a,1 if a==109 else w) for a,w in g],
 [x for x in g if x[0]!=180],
 g+[(180,ref)], [(a,0x3fc00000 if a==180 else w) for a,w in g],
 [(a,0x3f800000 if a==107 else w) for a,w in g], g]
bank=encode(3,b'SJTEST00','Witness fixture',[(i.to_bytes(8,'little'),str(i),r) for i,r in enumerate(fixtures)])
def capsule(d,mask=255):return b'SJP1'+bytes((1,1,len(fixtures),0))+hashlib.sha256(d).digest()+bytes((mask,))
with tempfile.TemporaryDirectory() as td:
 so=pathlib.Path(td)/'test.so'
 subprocess.run(['cc','-O2','-shared','-fPIC','-fsanitize=undefined','-fno-sanitize-recover=all',str(D/'dnbank.c'),str(D/'exact-profile.c'),str(D/'fused.c'),'-o',so],check=True)
 l=C.CDLL(str(so));l.dnbank_open.argtypes=[P,P,C.c_size_t]
 l.sj6_exact_stage.argtypes=[P,C.c_uint,P,C.c_size_t,TRUST,P,C.POINTER(Plan)]
 B=C.create_string_buffer(128); raw=C.create_string_buffer(bank);assert l.dnbank_open(B,raw,len(bank))
 accepted={capsule(bank),capsule(bank,1)}
 @TRUST
 def verify(_ctx,b,p,n):
  return int(b==C.addressof(B) and C.string_at(p,n) in accepted)
 unwired=C.cast(None,TRUST)
 def score(index, data, checker=verify, expected=None):
  out=Plan(0xfefefefe,7,7)
  res=l.sj6_exact_stage(B,index,data,0 if data is None else len(data),checker,None,C.byref(out))
  actual=(bool(res),out.base,out.has,out.profile)
  if expected is not None:assert actual==expected,(index,actual,expected)
  return actual
 assert len(capsule(bank))==41
 yes=(True,ref,1,1); exact_180=(True,ref,1,0); failed=(False,0xfefefefe,7,7)
 both=capsule(bank)
 for i,v in enumerate([yes,exact_180,exact_180,(True,0,0,0),failed,failed,exact_180,yes]):score(i,both,expected=v)
 score(0,both,unwired,exact_180)
 score(0,None,expected=exact_180)
 score(0,both[:-1],expected=exact_180)
 score(0,capsule(bank[:-1]),expected=exact_180)
 score(7,capsule(bank,1),expected=exact_180)
 for j in range(41):
  w=bytearray(both);w[j]^=0x40
  score(0,bytes(w),expected=exact_180)
 class Words(C.Structure):
  _fields_=[('base',C.c_uint32),('seen',C.c_uint8)]
 l.dnbank_read.argtypes=[P,C.c_uint,P,P,P]
 l.sj6_exact_finish.argtypes=[C.POINTER(Words),C.c_uint]
 for i in range(len(fixtures)):
  name=C.create_string_buffer(48)
  bag=Words()
  assert l.dnbank_read(B,i,name,C.cast(l.sj6_exact_emit,P),C.byref(bag))==len(fixtures[i])
  for trust in (0,1,2):
   expected=score(i,both,verify if trust==1 else unwired)
   mode=(-1 if not expected[0] else 2 if expected[3] else 1 if expected[2] else 0)
   assert l.sj6_exact_finish(C.byref(bag),trust)==mode,(i,mode)
   if expected[0] and expected[2]:assert bag.base==expected[1]
 print('Fused collector/finalizer parity against complete capsule validator on same checked DNL2 iterator | no persistent state')
 print('actual cab25d44 DNL2 float32 decode; exact #180 = 0x%08x; profile trust/bank SHA/whitelist, absent & nonzero Lo-Fi, bypass, duplicate & range guards PASS' % ref)
 print('capsule bytes for 1 / 640 / 1024 program banks: 41 / 120 / 168, excluding outer signature/envelope')
