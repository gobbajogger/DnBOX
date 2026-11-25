from pathlib import Path
import ctypes as C, tempfile, subprocess, sys,json
import numpy as np
r=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(r/'sj6-fx-bench'))
import bench
d=Path(__file__).resolve().parent
F=np.ctypeslib.ndpointer(np.float32,1,flags='C_CONTIGUOUS')
with tempfile.TemporaryDirectory() as tmp:
 so=Path(tmp)/'a.so'
 subprocess.run(['cc','-O2','-std=c11','-ffp-contract=off','-shared','-fPIC',str(d/'window-aa.c'),'-o',str(so)],check=True)
 lib=C.CDLL(str(so));lib.state_bytes.restype=C.c_size_t
 lib.stream_mix.argtypes=[C.c_void_p,C.c_size_t,F,F,F,F,C.c_float,C.c_float,C.c_float]
 def render(x,age,wet,partitions=False):
  st=C.create_string_buffer(lib.state_bytes())
  y=np.empty_like(x)
  if not partitions:lib.stream_mix(st,x.shape[1],*x,*y,age,.99*64/127,wet)
  else:
   i=0
   while i<x.shape[1]:
    j=min(i+149,x.shape[1]); z=np.ascontiguousarray(x[:,i:j]);o=np.empty_like(z)
    lib.stream_mix(st,j-i,*z,*o,age,.99*64/127,wet);y[:,i:j]=o;i=j
  return y
 results=[]
 for delay in (11025,11152,11156,11157,11162,11200,11400):
  for f in (220,1000,3000):
   t=np.arange(2048);x=np.zeros((2,16000),np.float32);x[0,:len(t)]=.12*np.sin(np.pi*t/len(t))**2*np.sin(2*np.pi*f*t/44100)
   y=render(x,delay,.9)-render(x,delay,0.)
   sl=y[0,delay-32:delay+len(t)+32]
   a=np.arange(len(sl));w=np.hanning(len(sl))
   amp=lambda z:float(abs(np.dot(sl*w,np.exp(-2j*np.pi*z*a/44100))))
   ref=amp(f)
   orig=.90*x[0,:len(t)]
   expected=float(abs(np.dot(orig*np.hanning(len(t)),np.exp(-2j*np.pi*f*t/44100))))
   results.append(dict(delay=delay,hz=f,first_correlated_db=round(20*np.log10(max(ref,1e-15)/expected),3),
        image1410_db=round(20*np.log10(max(amp(1410),1e-15)/expected),3) if f==3000 else None))
 print(json.dumps(results,indent=2))
