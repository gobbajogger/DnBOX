import ctypes as C, json
import importlib.util
from pathlib import Path
import numpy as np
from sj6_stereo_delay import library
D=Path(__file__).resolve().parent
R=44100
DEL=11025
T=4096
N=5*DEL
fp=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
cand=C.CDLL(str(D/'subband.so'))
cand.subband.argtypes=[C.c_uint,C.c_float,C.c_float]+[fp]*4
ref=library()
outrows=[]
spec=importlib.util.spec_from_file_location('stock_fixture',D.parent/'sj6-fx-bench/bench.py')
stock=importlib.util.module_from_spec(spec)
spec.loader.exec_module(stock)
for f in (220,1000,3000,3400,6000):
 for fb in (0,.499, .8):
    x=np.zeros((2,N),np.float32)
    x[0,:T]=.35*np.hanning(T)*np.sin(2*np.pi*f*np.arange(T)/R)
    a=np.empty_like(x)
    b=np.empty_like(x)
    orig=ref.sj6_new(R)
    try:
      ref.sj6_params(orig,DEL/R,fb,.1,0)
      ref.sj6_process(orig,N,*x,*a)
    finally: ref.sj6_free(orig)
    cand.subband(N,fb,.1,*x,*b)
    assert np.max(np.abs(b[1]))==0.
    alternatives={'subband': b}
    if fb==.499:
      alternatives['stock_fixture']=stock.compact(D.parent/'delay-now/guarded-sj6.so','delay',x,True)
    for mode,b in alternatives.items():
     for rep in ((1,) if fb==0 else (1,2,3,4)):
      lo=rep*DEL-128
      hi=rep*DEL+T+128
      ref_echo=a[0,lo:hi]
      cand_echo=b[0,lo:hi]
      win=np.hanning(hi-lo)
      phase=np.exp(-2j*np.pi*f/R*np.arange(hi-lo))
      a0=abs(phase@(ref_echo*win)); b0=abs(phase@(cand_echo*win))
      ff=np.fft.rfftfreq(16384,1/R)
      Y=np.abs(np.fft.rfft(cand_echo*win,16384))
      Y[np.abs(ff-f)<150]=0
      peak=np.argmax(Y)
      db=lambda x:round(20*np.log10(max(1e-15,float(x))),3)
      outrows.append(dict(mode=mode,f=f,fb=fb,rep=rep,carrier=db(b0/a0),spur=db(Y[peak]/a0),spur_hz=round(ff[peak],1),norm_null=db(np.linalg.norm(cand_echo-ref_echo)/np.linalg.norm(ref_echo))))
print(json.dumps({'history_bytes':cand.subband_size(),'tap_ms':DEL/R*1000,'rows':outrows},indent=2))
