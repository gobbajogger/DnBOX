import ctypes as c,json,math
import numpy as np
fp=np.ctypeslib.ndpointer(dtype='float32',flags='C_CONTIGUOUS')
n=24576;rng=np.random.default_rng(17);res=[]
for mode in range(3):
 lib=c.CDLL('/tmp/sj6-compact-hb%d.so'%mode)
 lib.probe.argtypes=[fp,fp,fp,c.c_uint]
 for note in [45,57,72,81,93,105]:
  for wav in ('sine','triangle','bend-sine','impulse'):
   hz=440*2**((note-69)/12);t=np.arange(n*4)
   a=np.ones(n*4)*hz/176400
   if wav=='bend-sine':a*=2**(np.clip((t-16000)/16000,-1,1)/6)
   phase=np.cumsum(a)
   z=(2*np.abs((phase+.25)%1-.5)-.5)*2 if wav=='triangle' else np.sin(2*np.pi*phase)
   z[:400]=0;z[70000:]=0
   if wav=='impulse':
    z[:]=0; z[400]=1; z[22567]=-1
   dst=np.zeros(n,'f4');ref=dst.copy()
   lib.probe(np.asarray(z,'f4'),dst,ref,n)
   e=dst-ref;window=slice(200,17000)
   amp=lambda v:np.linalg.norm(v[window]) if wav!='impulse' else np.linalg.norm(v)
   res.append(dict(mode=mode,note=note,wave=wav,error_dbr=20*math.log10(amp(e)/amp(ref)),
                   level_db=20*math.log10(amp(dst)/amp(ref)),peak=float(np.abs(e).max()),
                   tail_max=float(np.abs(dst[20000:]).max()),bytes=lib.footprint()))
print(json.dumps(res,indent=2))
