from pathlib import Path
import ctypes as C, math, random, subprocess, struct, tempfile
here=Path(__file__).resolve().parent
c=C.c_double
libm=C.CDLL('libm.so.6')
libm.fma.argtypes=[c,c,c];libm.fma.restype=c
with tempfile.TemporaryDirectory() as t:
    so=t+'/exact.so'
    subprocess.check_call(['cc','-Wall','-Wextra','-Werror','-O2','-fPIC','-shared',str(here/'exact_fused.c'),'-o',so])
    l=C.CDLL(so);f=l.sj6_fused;f.argtypes=[c,c,c,C.POINTER(c)]
    rng=random.Random(0x5ee6)
    def check(m,y,b):
        out=c(); assert f(m,y,b,C.byref(out)),(m,y,b)
        want=libm.fma(m,y,b)
        assert struct.pack('d',want)==struct.pack('d',out.value),(m,y,b,out.value,want)
    for i in range(100000):
        check(rng.uniform(.5,1),math.ldexp(rng.uniform(.5,1),rng.randrange(-35,1)),
              math.ldexp(rng.uniform(-1,1),rng.randrange(-35,1)))
    for e in range(-24,4):
      for a in (0.,1.,math.nextafter(1.,2.)):
       for b in (2**(e-53),2**(e-54),-2**e,math.nextafter(-2**e,0.),math.nextafter(-2**e,-math.inf)):
        check(1.,math.ldexp(a,e),b)
    # one and two neighbouring exact polyphase cancellation/rounding cases
    for i in range(10000):
        m=rng.uniform(.5,1); y=rng.uniform(.5,1); q=m*y
        for b in (q,math.nextafter(q,0.),math.nextafter(q,1.)):check(m,y,-b)
    print('130420 signed/cancellation/tie witnesses exact')
