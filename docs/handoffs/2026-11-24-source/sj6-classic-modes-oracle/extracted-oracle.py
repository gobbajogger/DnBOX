"""Independent binary32 instruction-order oracle for prepared classic AU subtype 0.
No call into C until the comparison; original Mach-O arithmetic is decoded and
verified separately by dispatch.py. Both sides use host IEEE libm.
"""
import ctypes as ct
import math, random, struct, sys

f=lambda x: struct.unpack('<f',struct.pack('<f',x))[0]
L=ct.CDLL('libm.so.6')
for name,typ,args in [('fma',ct.c_double,[ct.c_double]*3),('fmaf',ct.c_float,[ct.c_float]*3),('sinf',ct.c_float,[ct.c_float])]:
    obj=getattr(L,name);obj.restype=typ;obj.argtypes=args
def wf(x,b):
    while x<0: x=f(x+b)
    while x>=b: x=f(x-b)
    return x
sine=tuple(f(math.sin(2.*(i/256.)*math.pi)) for i in range(256))
def sample_lfo(pos):
    x=f(256.*wf(pos,1.))
    a=int(x);d=f(x-a)
    return f(L.fma(1.-d,sine[a],f(sine[(a+1)%256]*d)))

class OriginalDelay:
    def __init__(self):
        self.values=[0.]*1058
        self.cursor=0
    def use(self,ms,inp,feed):
        n=len(self.values)
        want=f(44100.*min(ms,24.)/1000.)
        read=wf(f(self.cursor-min(want,float(n))),float(n))
        x=int(read); frac=f(read-x)
        left=self.values[x]; right=self.values[(x+1)%n]
        wet=L.fmaf(f(1.-frac),left,f(frac*right))
        self.values[self.cursor]=L.fmaf(feed,wet,inp)
        self.cursor=(self.cursor+1)%n
        return wet
class OriginalRamp:
    def __init__(self,v,dur):
        self.pending=self.prev=self.current=self.origin=f(v)
        self.start=0;self.dur=dur
    def get(self,t):
        if self.prev!=self.pending:
            self.prev=self.pending;self.origin=self.current;self.start=t
        if self.current!=self.prev:
            delta=t-self.start
            if delta>=self.dur:
                self.start=0; self.current=self.prev
            elif delta>=0:
                self.current=L.fmaf(f(self.prev-self.origin),f(delta/self.dur),self.origin)
        return self.current
class OriginalModeI:
    def __init__(self,init,dur):
        self.r=tuple(OriginalRamp(x,dur) for x in init)
        self.delay=OriginalDelay(),OriginalDelay()
        self.ph=0.;self.on=False;self.mode=0
        self.center=f(f(f(3.518)+24.)*.5)
        self.span=f(f(24.-f(3.518))*.5)
    def command(self,k,u):
        self.r[[0,1,3][k-42]].pending=f({42:lambda:16.*pow(u,3.33),43:lambda:.97*u,44:lambda:u}[k]())
    def enter(self,mode):
        self.on=mode!=0
        if not mode:return
        self.mode=mode
        rate_read=f(pow((.513,.863,1.726)[mode-1]/16.,1./3.33))
        self.r[0].pending=f(16.*pow(rate_read,3.33))
        self.r[1].pending=f(.97*f(.97*.202))
        self.r[3].pending=f(.555 if mode==3 else .505)
    def process(self,at,a,b):
        if not self.on:return a,b
        result=[[],[]]
        for off in range(0,len(a),8):
            t=at+off
            speed,depth,feedback,wet=(r.get(t) for r in self.r)
            dphase=f(speed/44100.)
            halfperiod=2*math.pi*speed
            fbleft=f(feedback*L.fmaf(L.sinf(f(halfperiod*t/44100.)),f(.05),1.))
            fbright=f(feedback*L.fmaf(L.sinf(f(halfperiod*f(f(t)+f(.2))/44100.)),f(.05),1.))
            for j in range(off,min(len(a),off+8)):
                mods=sample_lfo(self.ph),sample_lfo(f(self.ph+.25))
                self.ph=wf(f(self.ph+dphase),1.)
                for c,y,fb,m in zip(range(2),(a[j],b[j]),(fbleft,fbright),mods):
                    ms=L.fmaf(f(self.span*depth),m,self.center)
                    active=self.delay[c].use(ms,y,fb)
                    result[c].append(f(f(1.707)*L.fmaf(f(1.-wet),y,f(wet*active))))
        return result
class Raw(ct.Structure):
    _fields_=[('ring',(ct.c_float*1058)*2),('write',ct.c_uint*2),('phase',ct.c_float)]
class Ramp(ct.Structure):
    _fields_=[('pending',ct.c_float),('prev',ct.c_float),('current',ct.c_float),('origin',ct.c_float),('at',ct.c_uint64),('dur',ct.c_uint)]
class Wrapper(ct.Structure):
    _fields_=[('core',Raw),('p',Ramp*4),('on',ct.c_uint),('mode_ii',ct.c_uint)]
C=ct.CDLL(sys.argv[1]); PTR=ct.POINTER
C.classic_wrapper_prepare.argtypes=[PTR(Wrapper),PTR(ct.c_float),ct.c_uint]
C.classic_wrapper_start.argtypes=[PTR(Wrapper),ct.c_int]
C.classic_wrapper_command.argtypes=[PTR(Wrapper),ct.c_uint,ct.c_float]
C.classic_wrapper_enter.argtypes=[PTR(Wrapper),ct.c_uint]
C.classic_wrapper_process.argtypes=[PTR(Wrapper),ct.c_uint64,PTR(ct.c_float),PTR(ct.c_float),ct.c_size_t]
total=0;counts=[0]*4
for owner in range(2):
    initial=(.513,.202,.505,.4)
    duration=10000 if not owner else 311
    rng=random.Random(42+owner)
    src=OriginalModeI(initial,duration); cand=Wrapper()
    C.classic_wrapper_prepare(ct.byref(cand),(ct.c_float*4)(*initial),duration)
    stamp=0
    mode=0
    for block in range(330):
        if block in (0,43,94,129,164,205,236,250,271):
            mode={0:1,43:2,94:0,129:3,164:2,205:1,236:0,250:2,271:3}[block]
            expected_phase=cand.core.phase
            expected_rings=bytes(cand.core)
            src.enter(mode)
            assert C.classic_wrapper_enter(ct.byref(cand),mode)
            assert bytes(cand.core)==expected_rings and cand.core.phase==expected_phase
            assert [p.pending for p in cand.p]==[p.pending for p in src.r]
            untouched=bytes(cand)
            assert not C.classic_wrapper_enter(ct.byref(cand),4) and bytes(cand)==untouched
        if block in (12,53,77,139,152,184,217,280,310):
            for k in (42,43,44):
                u=f(rng.random());src.command(k,u)
                assert C.classic_wrapper_command(ct.byref(cand),k,u)
        n=rng.choice([1,7,8,9,31,96,128])
        a=[f(rng.uniform(-.5,.5)) for _ in range(n)]
        b=[f(rng.uniform(-.5,.5)) for _ in range(n)]
        got_a,got_b=(ct.c_float*n)(*a),(ct.c_float*n)(*b)
        aa,bb=src.process(stamp,a,b)
        C.classic_wrapper_process(ct.byref(cand),stamp,got_a,got_b,n)
        assert bytes(got_a)==struct.pack('<%df'%n,*aa),(owner,block,stamp,'L')
        assert bytes(got_b)==struct.pack('<%df'%n,*bb),(owner,block,stamp,'R')
        assert cand.core.phase==src.ph and list(cand.core.write)==[v.cursor for v in src.delay],(owner,block,'state')
        stamp+=n;total+=n;counts[mode]+=n
assert all(n>1500 for n in counts)
print(f'PASS {total} exact independent source-decoded stereo frames, OFF/I/II/Both {counts}, two owners with binding, retained history/ramp state and active edits')
