import sys,macho,paths
from capstone import *
p=sys.argv[1];macho.EXPECTED_SHA='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850';paths.BIN=p
import objcmeta as o
im=o.im;b=im.b;B=macho.BASE
for cls in ['_TtC10SuperJ6_AU14AKTouchPadView','_TtC10SuperJ6_AU13PadController']:print(cls,o.ivars_any(cls))
c=Cs(CS_ARCH_ARM64,CS_MODE_ARM)
for aa in sys.argv[2:]:
 a,e=map(lambda x:int(x,16),aa.split(':'))
 for i in c.disasm(b[a-B:e-B],a):print(f'{i.address:x} {i.mnemonic} {i.op_str}')
from unicorn import Uc,UC_ARCH_ARM64,UC_MODE_ARM,UC_HOOK_CODE
from unicorn.arm64_const import *
import struct,json
def ins(a):
 i=next(c.disasm(b[a-B:a-B+4],a));return i.mnemonic,i.op_str
def eq(a,m,op): assert ins(a)==(m,op),(hex(a),ins(a))
for a in (0x1000d9298,0x1000d929c):eq(a,'b','#0x1000d8388')
eq(0x1000d83c4,'bl','#0x1000d9154')
eq(0x1000d89bc,'blr','x22')
eq(0x1000d91f8,'blr','x21')
eq(0x1002b2654,'ldrb','w0, [x20, x8]')
eq(0x1002b264c,'add','x8, x8, #0x390')
eq(0x1002b2784,'add','x8, x8, #0x398')
u=Uc(UC_ARCH_ARM64,UC_MODE_ARM)
for page in (0x1000d9000,0x1007da000,0x1005fa000,0x200000,0x201000):
 u.mem_map(page,4096)
 if page==0x1000d9000:u.mem_write(page,b[page-B:page-B+4096])
mem=0x200000
def read(a,n):return bytes(u.mem_read(a,n))
def ptr(a,v):u.mem_write(a,struct.pack('<Q',v))
def doubles(a,x,y):u.mem_write(a,struct.pack('<dd',x,y))
for key,v in {0x740:33,0x730:32,0x748:64,0x750:136,0x758:168}.items():ptr(0x1007da000+key,v)
events=[]
def hook(uc,a,n,_):
 if a in (0x1005fa338,0x1005fa5a8,0x1005fa590,0x201020):
  if a==0x201020:
   events.append([uc.reg_read(UC_ARM64_REG_W0),uc.reg_read(UC_ARM64_REG_W1),uc.reg_read(UC_ARM64_REG_X20),
     *[struct.unpack('<d',struct.pack('<Q',uc.reg_read(r)))[0] for r in (UC_ARM64_REG_D0,UC_ARM64_REG_D1)]])
  uc.reg_write(UC_ARM64_REG_PC,uc.reg_read(UC_ARM64_REG_LR))
u.hook_add(UC_HOOK_CODE,hook)
result=[]
for index in (0,1):
 x=mem+256*index;ctx=mem+0x800+index
 vals=(float(index+1)/4,float(index+2)/4)
 ptr(x+64,0x201020);ptr(x+72,ctx)
 u.mem_write(x+136,struct.pack('<d',vals[0]));u.mem_write(x+168,struct.pack('<d',vals[1]))
 for access,touch in ((0,0),(0,1),(1,1)):
  u.mem_write(x+32,bytes([access,touch]));events.clear()
  for reg,v in ((UC_ARM64_REG_X20,x),(UC_ARM64_REG_X0,0x1007da740),(UC_ARM64_REG_X1,0x1007da730),(UC_ARM64_REG_SP,mem+0xff0),(UC_ARM64_REG_LR,0x201000)):u.reg_write(reg,v)
  u.emu_start(0x1000d9154,0x201000,count=100)
  assert read(x+32,2)==bytes([access,0])
  assert events==([[1,0,ctx,*vals]] if touch and not access else [])
  result.append([index+1,access,touch,events[:]])
print(json.dumps(result))
