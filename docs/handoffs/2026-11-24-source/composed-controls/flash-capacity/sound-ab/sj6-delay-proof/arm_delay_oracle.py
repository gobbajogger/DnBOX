"""Run the original Build9 delay instructions through a small, fail-closed scalar
ARM64 interpreter. Host replacements: memory allocator supplied by the test
fixture, and the binary's imported powf. Not a full AU host."""
import ctypes as C
import hashlib
import math
import re
import struct
from pathlib import Path

SHA='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
BASE=0x100000000
M=0x20000000
INIT_READ=0x10034aa84
SET_PING=0x10034aa44
SET_FB=0x10034aa64
SET_DRY=0x10034aabc
RENDER=0x10034aac4
SET_AU=0x10035a234
POWF=0x1005d9aa4
F=struct.Struct('<f')
def f32(v): return F.unpack(F.pack(v))[0]
lm=C.CDLL('libm.so.6')
for name in ('powf','fmaf'):
    fn=getattr(lm,name); fn.restype=C.c_float
    fn.argtypes=[C.c_float]*(2 if name=='powf' else 3)
split=re.compile(r',\s*(?![^\[]*\])').split
fmt={'s':'f','d':'d','w':'I','x':'Q','b':'B'}
def number(s): return int(s.lstrip('#'),0)

class CPU:
    def __init__(self,path):
        self.image=Path(path).read_bytes()
        if hashlib.sha256(self.image).hexdigest()!=SHA:
            raise ValueError('unexpected private AU version')
        self.mem=bytearray(3_000_000)
        self.dec={}
        self.g={}
        self.flags=(False,True)
        llvm=C.CDLL('libLLVM-19.so')
        for k in ('TargetInfo','Target','TargetMC','Disassembler'):
            getattr(llvm,'LLVMInitializeAArch64'+k)()
        llvm.LLVMCreateDisasm.restype=C.c_void_p
        llvm.LLVMDisasmInstruction.argtypes=(C.c_void_p,C.c_char_p,C.c_uint64,C.c_uint64,C.c_char_p,C.c_size_t)
        llvm.LLVMDisasmInstruction.restype=C.c_size_t
        self.llvm=llvm
        self.dc=llvm.LLVMCreateDisasm(b'aarch64-apple-darwin',None,0,None,None)
        if not self.dc: raise RuntimeError('ARM64 disassembler unavailable')

    def r(self,k):
        if k.startswith('#'):return float(k[1:]) if '.' in k else number(k)
        if k in ('xzr','wzr'):return 0
        k=k.split('.')[0]
        if k[0] in 'sxwdv' and k[1:].isdigit():
            v=self.g.get(('x' if k[0] in 'xw' else 'v')+k[1:],0)
            return int(v)&0xffffffff if k[0]=='w' else v
        return self.g[k]
    def w(self,k,v):
        if k in ('xzr','wzr'):return
        k=k.split('.')[0]
        if k[0] in 'sxwdv' and k[1:].isdigit():
            t=k[0]
            if t=='w':v=int(v)&0xffffffff
            if t=='s':v=f32(v)
            k=('x' if t in 'xw' else 'v')+k[1:]
        self.g[k]=v
    def load(self,a,t):
        return struct.unpack_from('<'+t,self.image if a>=BASE else self.mem,a-(BASE if a>=BASE else M))[0]
    def put(self,a,t,v):
        struct.pack_into('<'+t,self.mem,a-M,v)
    def addr(self,s,post=None):
        base,*offset=s.rstrip('!')[1:-1].split(', ')
        delta=self.r(offset[0]) if offset else 0
        if offset and offset[0].startswith('w') and len(offset)>1:
            if offset[1].startswith('sxtw') and delta>=2**31:delta-=2**32
        if len(offset)>1 and '#' in offset[1]:delta<<=number(offset[1].split()[-1])
        a=self.r(base)+delta
        if s.endswith('!'):self.w(base,a)
        if post is not None:self.w(base,self.r(base)+number(post))
        return a
    def compare(self,d):
        self.flags=(d<0,d==0)
    def cond(self,k):
        n,z=self.flags
        return {'lt':n,'mi':n,'lo':n,'le':n or z,'ls':n or z,'ne':not z,
                'eq':z,'gt':not(n or z),'ge':not n}[k]
    def insn(self,a):
        if a not in self.dec:
            b=C.create_string_buffer(240);i=a-BASE
            if self.llvm.LLVMDisasmInstruction(self.dc,self.image[i:i+4],4,a,b,240)!=4:
                raise ValueError(f'cannot decode {a:#x}')
            op,args=b.value.decode().strip().split(None,1) if b.value.strip()!=b'ret' else ('ret','')
            self.dec[a]=op,split(args) if args else []
        return self.dec[a]
    def call(self,pc,*argv,f=None):
        self.g={'sp':M+len(self.mem)-64,'x30':0}
        for i,x in enumerate(argv):self.w('x'+str(i),x)
        if f is not None:self.w('d0',f)
        r,w=self.r,self.w
        for _ in range(10_000_000):
            if pc==0:return
            if pc==POWF:
                w('s0',lm.powf(r('s0'),r('s1')));pc=r('x30');continue
            op,a=self.insn(pc)
            q=pc;pc+=4
            if op=='ret':pc=r('x30')
            elif op in ('b','bl'):
                if op=='bl':w('x30',pc)
                pc=q+r(a[0])
            elif op.startswith('b.'):
                if self.cond(op[2:]):pc=q+r(a[0])
            elif op in ('cbz','tbz'):
                if (r(a[0])==0 if op=='cbz' else not (r(a[0])>>r(a[1]))&1):pc=q+r(a[-1])
            elif op=='cset':w(a[0],int(self.cond(a[1])))
            elif op in ('cmp','fcmp'):self.compare(r(a[0])-r(a[1]))
            elif op in ('csel','fcsel'):w(a[0],r(a[1] if self.cond(a[3]) else a[2]))
            elif op=='fcvtzs' and a[0].startswith('s'):
                w(a[0],struct.unpack('<f',struct.pack('<i',int(r(a[1]))))[0])
            elif op=='scvtf' and a[1].startswith('s'):
                w(a[0],struct.unpack('<i',F.pack(r(a[1])))[0])
            elif op in ('mov','mov.16b','fmov','fcvt','fcvtzs','ucvtf','scvtf'):w(a[0],r(a[1]))
            elif op in ('add','sub','fadd','fsub','fmul','fdiv'):
                u,v=r(a[1]),r(a[2])
                w(a[0],u+v if op.endswith('add') else u-v if op.endswith('sub') else u*v if op=='fmul' else u/v)
            elif op=='fmadd':w(a[0],lm.fmaf(*(r(s) for s in a[1:])))
            elif op=='ubfiz':w(a[0],(r(a[1])&((1<<r(a[3]))-1))<<r(a[2]))
            elif op=='adrp':w(a[0],(q&~4095)+r(a[1]))
            elif op in ('ldr','ldrb','ldrsw','str','strb','stp','ldp'):
                pair=op in ('stp','ldp'); count=1+pair
                at=self.addr(a[count],a[count+1] if len(a)>count+1 else None)
                for k in a[:count]:
                    t='B' if op.endswith('b') else 'i' if op=='ldrsw' else fmt[k[0]]
                    if op.startswith('ld'):w(k,self.load(at,t))
                    else:self.put(at,t,r(k))
                    at+=struct.calcsize('<'+t)
            else:raise NotImplementedError((hex(q),op,a))
        raise RuntimeError('instruction cap exceeded')

class Original:
    def __init__(self, cpu, rate):
        self.c=cpu; self.capacity=int(rate*3); self.state=M+128
        cpu.mem[:]=bytes(len(cpu.mem))
        self.wrapper=M+512
        cpu.put(self.wrapper+56,'Q',self.state)
        for i,f in enumerate((0.,3.,0.,1.,0.,1.)):cpu.put(self.wrapper+64+4*i,'f',f)
        for i,o in enumerate((24,72)):
            s=self.state+o
            cpu.put(s,'d',rate); cpu.put(s+8,'d',3000)
            cpu.put(s+24,'Q',M+1024+4*self.capacity*i)
            cpu.put(s+32,'I',self.capacity)
        self.buff=M+1024+8*self.capacity+64
    def set(self,sec,fb,dry,ping):
        for param, value in enumerate((sec,fb,dry,float(ping))):
            # Immediate AU setters (no ramp); take the actual caller units and
            # clamp behavior from the AU's state, then call native DSP setters.
            self.c.call(SET_AU,self.wrapper,param,1,f=f32(value))
        self.c.call(INIT_READ,self.state,f=self.c.load(self.state+144,'f')*1000.)
        self.c.call(SET_FB,self.state,f=self.c.load(self.state+192,'f'))
        self.c.call(SET_DRY,self.state,f=self.c.load(self.state+240,'f'))
    def process(self,l,r):
        n=len(l); assert n==len(r)
        b=self.buff; ptr=[b+32+n*4*i for i in range(4)]
        for i,a in enumerate(ptr):self.c.put(b+i*8,'Q',a)
        for a,x in zip(ptr,(l,r)):
            self.c.mem[a-M:a-M+4*n]=struct.pack('<'+'f'*n,*x)
        self.c.call(RENDER,self.state,n,b,b+16)
        return tuple(struct.unpack_from('<'+'f'*n,self.c.mem,a-M) for a in ptr[2:])
