"""Independent signed stereo broad holdout; actual current middle & bypass."""
import ctypes as c, sys, json, math
from pathlib import Path
import numpy as np
from unicorn import UC_HOOK_CODE
from current import instantiate
tlib=c.CDLL(sys.argv[1]);scratch=(c.c_float*2)()
a=Path(__file__).with_name('score.py').read_text()
exec(compile(a[a.index('def wire('):a.index('def run(')],'outer-only splice','exec'))
n=8192; X=np.zeros((2,n),dtype='f4')
t=np.arange(4096)/44100.
X[:,:4096]=np.array([.3*np.sin(t*(2*math.pi*(207+100000*t))),-.17*np.sin(t*(2*math.pi*(1301+50000*t)))])
X[0,0]+=.5;X[1,23]-=.4
def run(m,on,width):
    j=instantiate(on)
    j._set(0,m)
    q=wire(j,width) if width else None
    a=np.concatenate([j.process(X[:,i:i+113]) for i in range(0,n,113)],axis=1)
    assert not j.e.unknown
    if q: assert q==([17831, n,None] if on else [0,0,None]),q
    return a
r={}
for m in (1,2,3):
    ref=run(m,True,None)
    assert np.array_equal(X,run(m,False,16))
    def s(x,y):return float(20*np.log10(np.linalg.norm(x.astype('f8')-y)/np.linalg.norm(y)))
    r[m]={}
    for width in (4,8,16):
        x=run(m,True,width)
        r[m][width]={k:s(x[:,a:b],ref[:,a:b]) for k,a,b in (('attack',0,256),('wide_active',256,4096),('ring_tail',4096,8192))}
print(json.dumps(r,indent=2))
