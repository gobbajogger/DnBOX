import ctypes as C
import hashlib, json, math
from pathlib import Path
import numpy as np
from sj6_stereo_delay import library
D=Path(__file__).resolve().parent
R=44100
fp=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
variants={}
for name in ('base','center','geometry','combined'):
    d=C.CDLL(str(D/(name+'.so')))
    d.compact.argtypes=[C.c_uint]*3+[fp,fp,C.c_uint]
    d.compact.restype=C.c_uint
    variants[name]=d
ref=library()
cases=[]
for raw in (1,11,21,42,85,127):
  sec=float(np.float32(np.float32(1.5/127)*raw))
  frames=int(round(sec*R))
  for fb in (0,64):
    periods=(1,) if fb==0 else (1,2,3)
    n=max(periods)*frames+4096
    for hz in (220,1000,3000,6000):
      length=min(2400,frames//2)
      t=np.arange(length)/R
      burst=(np.sin(2*np.pi*hz*t)*np.hanning(length)).astype('float32')
      x=np.zeros((2,n),np.float32)
      x[0,:length]=.4*burst
      wet=np.empty_like(x)
      st=ref.sj6_new(R)
      ref.sj6_params(st,sec,(.99/127)*fb,.1,0)
      ref.sj6_process(st,n,*x,*wet)
      ref.sj6_free(st)
      for name,d in variants.items():
        y=x.copy()
        stride=d.compact(raw,fb,127,*y,n)
        opposite=float(np.max(np.abs(y[1])))
        assert opposite==0
        for k in periods:
          start=k*frames
          ct=y[0,start-100:start+length+100].astype(float)
          rt=wet[0,start-100:start+length+100].astype(float)
          norm=lambda z: float(np.dot(z,z))
          db=lambda e: 10*math.log10(max(1e-32,e)/norm(rt))
          # Frequency selectivity separates surviving desired carrier from total aliased energy.
          z=np.exp(2j*np.pi*hz*np.arange(len(ct))/R)
          spur_fft=np.abs(np.fft.rfft(ct*np.hanning(len(ct))))
          fft_freq=np.fft.rfftfreq(len(ct),1/R)
          outside=np.abs(fft_freq-hz)>max(120,3*R/len(ct))
          peak_out=float(np.max(spur_fft[outside]))
          carrier=max(1e-16,float(abs(ct@z)))
          cases.append(dict(raw=raw,ms=round(1000*sec,4),fb=fb,hz=hz,repeat=k,variant=name,stride=stride,level=round(db(norm(ct)),3),
                            null=round(db(norm(ct-rt)),3),carrier=round(20*math.log10(carrier/abs(rt@z)),3),spur_hz=round(float(fft_freq[outside][np.argmax(spur_fft[outside])]),1),
                            zero_crosstalk=opposite==0))
print(json.dumps(dict(base_include_sha256=hashlib.sha256((D/'base/juno6_delay_native.inc').read_bytes()).hexdigest(),cases=cases),indent=2))
