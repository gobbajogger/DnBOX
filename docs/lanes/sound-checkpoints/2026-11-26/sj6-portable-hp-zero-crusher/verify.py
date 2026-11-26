"""Compare independent DSP laws against six pinned real Apple component renders."""
import ctypes
import hashlib
import json
from pathlib import Path
import sys

import numpy as np

dylib, chain, stems = sys.argv[1:]
lib=ctypes.CDLL(dylib)
for sym in ("sj6_zero_leaf_reference","sj6_zero_leaf_float","sj6_zero_leaf_stable"):
    s=getattr(lib,sym)
    s.argtypes=[ctypes.c_uint,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_size_t]
    s.restype=None

cases=[(Path(chain)/a,0 if a.endswith("hpf") else 1) for a in ("before-hpf","candidate-hpf","before-crusher","candidate-crusher")]
cases += [(Path(stems)/a,1) for a in ("native-main","native-sub")]

def digest(x):
    return hashlib.sha256(x.astype("<f4").tobytes()).hexdigest()
def metrics(got,want):
    e=got.astype(np.float64)-want
    return {"max_abs":float(np.max(np.abs(e))),"residual_dbr":round(float(20*np.log10(max(1e-28,np.linalg.norm(e))/np.linalg.norm(want))),8)}

stream=lib.sj6_zero_stream
stream.argtypes=[ctypes.c_uint,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_size_t,ctypes.c_uint]
stream.restype=None
def paired(stage,x):
    y,z=[np.empty_like(x) for _ in range(2)]
    for i in range(2):
        for split,g in enumerate((y,z)):
            stream(stage,x[i].ctypes.data,g[i].ctypes.data,x.shape[1],split)
    assert digest(y)==digest(z), "render partitions changed output"
    return y

out={}
for p,stage in cases:
    r=json.loads((p/"capture.json").read_text())
    x=np.load(p/"input.npy",allow_pickle=False)
    y=np.load(p/"output.npy",allow_pickle=False)
    assert x.shape==y.shape==(2,148684) and x.dtype==y.dtype==np.float32 and np.isfinite(x).all() and np.isfinite(y).all()
    assert r["sample_rate"]==44100 and digest(x)==r["input_pcm_sha256"] and digest(y)==r["output_pcm_sha256"]
    assert r["au_component"]==("aufx/hpas/appl" if stage==0 else "aufx/dist/appl")
    if stage==0:
        assert r["native_before"]==r["native_after"]=={"0":10,"1":0}
    else:
        assert r["forced_parameter_values"]=={"3":0.44387006759643555,"4":0,"5":0,"15":0}
    results={}
    for opt in ("reference","float","stable"):
        g=np.empty_like(y)
        for i in range(2):
            getattr(lib,"sj6_zero_leaf_"+opt)(stage,x[i].ctypes.data,g[i].ctypes.data,x.shape[1])
        results[opt]=metrics(g,y)
    results["compact_partition_exact"]=metrics(paired(stage,x),y)
    results["input_sha256"]=digest(x)
    results["output_sha256"]=digest(y)
    out[p.name]=results

for arm in ("before","candidate"):
    h=Path(chain)/(arm+"-hpf")
    c=Path(chain)/(arm+"-crusher")
    x=np.load(h/"input.npy",allow_pickle=False)
    mid=np.load(h/"output.npy",allow_pickle=False)
    read=np.load(c/"input.npy",allow_pickle=False)
    y=np.load(c/"output.npy",allow_pickle=False)
    assert digest(mid)==digest(read)
    out[arm+"-chain"]={"compact":metrics(paired(2,x),y),"input_sha256":digest(x),"output_sha256":digest(y)}
print(json.dumps(out,indent=2,sort_keys=True))
