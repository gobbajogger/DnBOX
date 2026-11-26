import ctypes as C, sys,json,hashlib,os, numpy as np
from pathlib import Path
o=Path(sys.argv[1]); x=Path('sj6-current-faders/probe.py'); ns={};exec(compile(x.read_text().split('cases={}')[0],str(x),'exec'),ns);e=ns['e']
co=(C.c_float*5)(*[e.rf(t) for t in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)])
p=(C.c_float*4)(*json.load(open('sj6-raw-main-ota/physical.json'))['physical_controls'][0])
d=Path('sj6-cross-preset/sub-range.private-output/2-dry.private.txt')
lib=C.CDLL(str(o/'combined.so'))
lib.select_ota.argtypes=[C.c_uint,C.c_void_p,C.c_void_p]
lib.select_ota(3,p,co)
lib.run_render.argtypes=[C.c_int,C.POINTER(C.c_char_p)]
f=o/'dry.f32';a=(C.c_char_p*3)(b'probe',str(d).encode(),str(f).encode())
assert lib.run_render(3,a)==0
pcm=np.fromfile(f,'<f4').reshape(-1,2).T.copy();f.unlink()
h=lambda x:hashlib.sha256(x).hexdigest()
assert h(pcm.tobytes())=='5bb4c2aa9bd9ea8a87b2ba505ce93c7e6f756505cd213c2b100515e805713d9c'
dtype=np.dtype([(k,'<u4') for k in ('frame','key','slot','gate','stage')]+[(k,'<f4') for k in ('amp','sub','mix')])
lib.release_trace.restype=C.c_void_p
n=lib.release_trace_count();assert n==123675
z=np.frombuffer(C.string_at(lib.release_trace(),n*dtype.itemsize),dtype=dtype)
ans={}
for k in np.unique(z['key']):
 w=z[z['key']==k]; rel=w[w['gate']==0]; amp=w[w['amp']!=0]; stage=w[w['stage']==7]
 ans[int(k)]=dict(key=int(k),slots=list(map(int,np.unique(w['slot']))),first=int(w[0]['frame']),last=int(w[-1]['frame']),count=len(w),gate_off=int(rel[0]['frame']),last_nonzero_amp=int(amp[-1]['frame']),zero_calls=int(np.count_nonzero(w['amp']==0)),
   signed_amp_at_gate=rel[:6]['amp'].tolist(),
   signed_amp_close=[(int(t['frame']),float(t['amp']),float(t['mix']),float(t['sub'])) for t in rel[550:]],
   signed_derivatives=list(map(float,np.diff(rel['amp'].astype('f8'))[:20])))
print(json.dumps(ans,indent=2))
