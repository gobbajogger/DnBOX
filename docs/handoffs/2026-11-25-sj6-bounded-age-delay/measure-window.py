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
 subprocess.run(['cc','-O2','-std=c11','-ffp-contract=off','-shared','-fPIC',str(d/'window.c'),'-o',str(so)],check=True)
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
 n=44100;N=2048
 db=lambda a,b:float(20*np.log10(max(np.linalg.norm(a),1e-19)/max(np.linalg.norm(b),1e-19)))
 results=[]
 for f in [220,1000,3000,6000]:
  t=np.arange(N);x=np.zeros((2,n),np.float32);x[0,:N]=.12*np.sin(np.pi*t/N)**2*np.sin(2*np.pi*f*t/44100)
  dry=render(x,11025,0)
  assert np.array_equal(dry.view('u4'),x.view('u4'))
  wet=render(x,11025,.9)
  assert np.array_equal(wet.view('u4'),render(x,11025,.9,True).view('u4'))
  y=wet-dry;assert (y[1]==0).all()
  old=bench.compact(r/'fit-s1-ab/sj6.so','delay',x,True)-x
  ref=bench.audited_delay_reference(x,True,r/'sj6-multirate')-x
  for k in (1,2):
   j=k*11025;s=slice(j-32,j+N+32)
   results.append((f,k,round(db(old[0,s],ref[0,s]),3),round(db(y[0,s],ref[0,s]),3),
                   round(db(y[0,s]-ref[0,s],ref[0,s]),3)))
 print(json.dumps(dict(bytes=lib.state_bytes(),bypass_equal=True,partitions_equal=True,
         legend=['freq','echo','fitted_level_db','trial_level_db','trial_null_db'],results=results),indent=2))
