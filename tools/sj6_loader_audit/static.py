from pathlib import Path
import struct,sys,bisect,json,hashlib
import os
from dependencies import load_capstone
capstone, dependency_receipt = load_capstone()
ROOT=Path(os.environ.get('SJ6_AUDIT_OUTPUT','sound-v2-evidence/legacy-loader-reviewed'))
ROOT.mkdir(parents=True,exist_ok=True)
b=Path(os.environ.get('SJ6_ORIGINAL_ARM64','superj6-qualification/private/original-graph/SuperJ6-AU-original-arm64.bin')).read_bytes();sha=hashlib.sha256(b).hexdigest();assert sha=='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
segs=[];starts=[];at=32
for _ in range(struct.unpack_from('<I',b,16)[0]):
 cmd,size=struct.unpack_from('<II',b,at)
 if cmd==0x19:
  name,va,vs,off,fs=struct.unpack_from('<16sQQQQ',b,at+8);segs.append((va,fs,off))
  for j in range(struct.unpack_from('<I',b,at+64)[0]):
   sn,sg,v,n,o=struct.unpack_from('<16s16sQQI',b,at+72+80*j)
   if sn.rstrip(b'\0')==b'__text':textseg=(v,n,o)
 if cmd==0x26:
  q,n=struct.unpack_from('<II',b,at+8);end=q+n;a=0x100000000
  while q<end:
   v=shift=0
   while True:
    c=b[q];q+=1;v|=(c&127)<<shift;shift+=7
    if not c&128:break
   if not v:break
   a+=v;starts.append(a)
 at+=size
cs=capstone.Cs(capstone.CS_ARCH_ARM64,capstone.CS_MODE_LITTLE_ENDIAN);cs.detail=True

def offset(a):
 return next(off+a-va for va,n,off in segs if va<=a<va+n)
def data(a,n):return b[offset(a):offset(a)+n]
def func(a):return starts[bisect.bisect_right(starts,a)-1]
def dis(a,n):return '\n'.join(f'{x.address:#x} {x.mnemonic} {x.op_str}' for x in cs.disasm(data(a,n),a))
if __name__=='__main__':
 if sys.argv[1]=='dis':
  a=int(sys.argv[2],0);n=int(sys.argv[3],0);print(dis(a,n))
