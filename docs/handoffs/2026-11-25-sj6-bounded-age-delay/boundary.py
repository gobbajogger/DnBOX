"""At every history-edge spot-check fresh and aged wet read; no production."""
import ctypes as C, subprocess, tempfile,json,math
from pathlib import Path
import numpy as np
D=Path(__file__).resolve().parent
with tempfile.TemporaryDirectory() as tmp:
 out=Path(tmp)/'win.so'
 subprocess.run(['cc','-O2','-std=c11','-ffp-contract=off','-shared','-fPIC',str(D/'window.c'),'-o',str(out)],check=True)
 so=C.CDLL(str(out));so.state_bytes.restype=C.c_size_t
 arr=np.ctypeslib.ndpointer(np.float32,1,flags='C_CONTIGUOUS')
 so.stream_mix.argtypes=[C.c_void_p,C.c_size_t,arr,arr,arr,arr,C.c_float,C.c_float,C.c_float]
 so.seek_age.argtypes=[C.c_void_p,C.c_float,C.POINTER(C.c_float),C.POINTER(C.c_float)]
 st=C.create_string_buffer(so.state_bytes()); z=np.zeros(131072,np.float32); out=z.copy();x=np.full_like(z,.125)
 for _ in range(70):so.stream_mix(st,len(x),x,z,out,out,25000,0,0)
 lo=C.c_float();hi=C.c_float()
 def seek(a):return so.seek_age(st,a,C.byref(lo),C.byref(hi)),lo.value,hi.value
 assert so.state_bytes()<=4096
 ages=sorted(set(range(1,100000)).union(range(100000,8372224,97),{8372224}))
 bad=[a for a in ages if (q:=seek(a))[0]<0 or q[1:]!=(.125,0.)]
 a=[(v,*seek(v)) for v in (1,46,47,11025,11111,11150,16000,44100,66150,705600,8372224)]
 print(json.dumps(dict(ages_checked=len(ages),holes=len(bad),earliest_bad=bad[:15],advanced_samples=70*len(x),selected=a),indent=2))
 assert not bad
