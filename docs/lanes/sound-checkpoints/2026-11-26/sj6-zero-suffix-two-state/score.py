import ctypes,hashlib,json,sys
from pathlib import Path
import numpy as np
lib=ctypes.CDLL(sys.argv[1]); s=lib.mini_run
s.argtypes=[ctypes.c_uint,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_size_t,ctypes.c_uint]
path=Path(sys.argv[2])
def read(p):
    j=json.loads((p/'capture.json').read_text())
    x,y=(np.load(p/k,allow_pickle=False) for k in ['input.npy','output.npy'])
    for a,key in [(x,'input_pcm_sha256'),(y,'output_pcm_sha256')]:
        assert a.dtype==np.dtype('<f4') and hashlib.sha256(a.tobytes()).hexdigest()==j[key] and np.isfinite(a).all()
    assert j['sample_rate']==44100
    return x,y
def run(i,x):
    a,b=np.empty_like(x),np.empty_like(x)
    for c in range(len(x)):
        s(i,x[c].ctypes.data,a[c].ctypes.data,x.shape[1],0)
        s(i,x[c].ctypes.data,b[c].ctypes.data,x.shape[1],1)
    assert a.tobytes()==b.tobytes()
    return a
def score(a,b):
    z=a.astype(np.float64)-b
    return dict(max_abs=float(np.max(np.abs(z))),signed_dBr=float(20*np.log10(np.linalg.norm(z)/np.linalg.norm(b))))
out={}
for tag in ['before','candidate']:
    x,h=read(path/'chain'/(tag+'-hpf'))
    h2,y=read(path/'chain'/(tag+'-crusher'))
    assert h.tobytes()==h2.tobytes()
    out[tag]=dict(hp=score(run(0,x),h),crusher=score(run(1,h),y),pair=score(run(2,x),y))
for tag in ['native-main','native-sub']:
    x,y=read(path/'stems'/tag)
    out[tag]=score(run(1,x),y)
print(json.dumps(out,indent=2))
