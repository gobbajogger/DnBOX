import sys,struct,json,math
from pathlib import Path
import macho
from capstone import Cs,CS_ARCH_ARM64,CS_MODE_ARM
from unicorn import Uc,UC_ARCH_ARM64,UC_MODE_ARM,UC_HOOK_CODE
from unicorn.arm64_const import *
macho.EXPECTED_SHA='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
im=macho.Image(Path(sys.argv[1]));b=im.b;B=0x100000000;rb=dict(im.rebases)
def cstr(a):return b[a-B:b.index(0,a-B)].decode()
def rel(a):return a+struct.unpack_from('<i',b,a-B)[0]
assert cstr(rb[0x1007c24f8])=='_TtC8AudioKit11AKDecimator'
assert rb[0x1007c2500]==0x10060f5a8
assert struct.unpack_from('<II',b,0x60f5a8)==(0x8000000c,15)
methods={cstr(rb[rel(a)]):rel(a+8) for a in range(0x10060f5b0,0x10060f5b0+15*12,12)}
assert [methods[k] for k in ('setDecimation:','setRounding:','setMix:')]==[0x1004adcf4,0x1004add98,0x1004ade3c]
syms={vm:im.imports[i][0] for vm,i,_ in im.binds}
assert syms[0x100755fa0]=='_AudioUnitSetParameter'
c=Cs(CS_ARCH_ARM64,CS_MODE_ARM)
def at(a):
 x=next(c.disasm(b[a-B:a-B+4],a));return x.mnemonic,x.op_str
assert at(0x1004adea4)==('mov','x8, #0x4059000000000000')
assert at(0x1004adeac)==('fmul','d8, d0, d1')
assert at(0x1004adee4)==('fcvt','s0, d8')
assert at(0x1004adef8)==('bl','#0x1005f90c0')
assert at(0x1005f90c4)==('ldr','x16, [x16, #0xfa0]')
Q=lambda d:struct.unpack('<Q',struct.pack('<d',d))[0]
F=lambda q:struct.unpack('<f',struct.pack('<I',q&0xffffffff))[0]
TOF=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
# Run the exact receiver instructions. Stub Swift ARC/exclusivity, AVAudioUnit
# resolution and the OS boundary; record the actual call ABI instead of DSP.
u=Uc(UC_ARCH_ARM64,UC_MODE_ARM)
touched=[]
for p in (0x1004ad000,0x100810000,0x1000f1000,0x1004a6000):
 u.mem_map(p,0x1000);u.mem_write(p,b[p-B:p-B+0x1000])
for p in (0x1004ae000,0x1005fa000,0x1005fb000,0x1005f9000,0x300000,0x400000,0x100603000,0x100604000,0x100605000,0x100606000,0x1007d4000,0x10082e000):
 u.mem_map(p,0x1000)
obj=0x300200;end=0x400100
for a,x in ((0x1008107e0,0x10),(0x1008107e8,0x18),(0x1008107f0,0x20),(0x100810800,0x28),(obj+0x28,0x12345678)):
 u.mem_write(a,struct.pack('<Q',x))
def cb(u,a,s,_):
 if a in {0x1005fa338,0x1005fa02c,0x1005fb7c0,0x1005f90c0,0x1005fa428,0x1005fa020}:
  if a==0x1005f90c0:
   touched.append([u.reg_read(x) for x in (UC_ARM64_REG_X0,UC_ARM64_REG_X1,UC_ARM64_REG_X2,UC_ARM64_REG_X3,UC_ARM64_REG_X4)]+[F(u.reg_read(UC_ARM64_REG_S0))])
  u.reg_write(UC_ARM64_REG_PC,u.reg_read(UC_ARM64_REG_LR))
def put(a,x):u.mem_write(a,struct.pack('<Q',x))
for a,x in ((0x10082ea30,0x30),(0x1007d4c60,0x40),(0x1007d4c70,0x44),
            (obj+0x30,0x300400),(0x300428,obj),(0x300430,0x300600)):put(a,x)
boost=[]
def router(u,a,s,_):
 d={0x100605440:'setMix:',0x100603c40:'setDecimation:',0x1006063c0:'setRounding:'}
 if a in d:u.reg_write(UC_ARM64_REG_PC,methods[d[a]])
 elif a==0x100604640:
  boost.append(struct.unpack('<d',struct.pack('<Q',u.reg_read(UC_ARM64_REG_D0)))[0])
  u.reg_write(UC_ARM64_REG_PC,u.reg_read(UC_ARM64_REG_LR))
 elif a==0x1005fa0d4:
  x,y=(struct.unpack('<d',struct.pack('<Q',u.reg_read(reg)))[0] for reg in (UC_ARM64_REG_D0,UC_ARM64_REG_D1))
  u.reg_write(UC_ARM64_REG_D0,Q(math.pow(x,y)))
  u.reg_write(UC_ARM64_REG_PC,u.reg_read(UC_ARM64_REG_LR))
u.hook_add(UC_HOOK_CODE,cb)
u.hook_add(UC_HOOK_CODE,router)
outputs=[]
for label,param in [('setDecimation:',3),('setRounding:',4),('setMix:',15)]:
 for v in [-.1,0,.0606,.125,.5,1,1.1]:
  u.reg_write(UC_ARM64_REG_SP,0x300ff0);u.reg_write(UC_ARM64_REG_LR,end)
  u.reg_write(UC_ARM64_REG_X0,obj);u.reg_write(UC_ARM64_REG_D0,Q(v))
  u.emu_start(methods[label],end,count=2000)
  r=touched.pop();assert not touched
  assert r[:5]==[0x12345678,param,0,0,0] and r[5]==TOF(100*min(max(v,0),1)),r
  outputs.append([label,v,r[5]])

def panel(id,v):
 dest={107:0x1000f1440,108:0x1000f1598,109:0x1000f1500}[id]
 u.reg_write(UC_ARM64_REG_SP,0x300ff0);u.reg_write(UC_ARM64_REG_LR,end)
 u.reg_write(UC_ARM64_REG_X20,obj)
 u.reg_write(UC_ARM64_REG_S0,struct.unpack('<I',struct.pack('<f',v))[0])
 u.emu_start(dest,end,count=4000)
 r=[(x[1],x[5]) for x in touched];touched.clear()
 g=list(boost);boost.clear()
 return r,g
seq=[]
for i,v in [(107,0),(108,.25),(109,1),(107,1),(108,0),(109,.5),(107,0)]:
 r,g=panel(i,v)
 if i==107:assert r==[(15,30 if v else 0)] and g==[1.3 if v else 1.]
 if i==108:assert r==[(15,TOF(100*math.pow(v,1.5)))] and not g
 if i==109:assert r==[(3,TOF(100*(.0606*math.pow(v,1.23)))),(4,0.)] and not g
 seq.append([i,v,r,g])
print(json.dumps(dict(digest=im.sha,methods={k:hex(methods[k]) for k in ('setDecimation:','setRounding:','setMix:')},exact_original_CPU_receiver_checks=outputs,
 max_source_109_percent=TOF(100*.0606),original_conductor_and_receiver_tape=seq),indent=2))
