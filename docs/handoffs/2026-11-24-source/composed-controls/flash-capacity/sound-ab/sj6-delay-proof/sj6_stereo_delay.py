"""Build9 SuperJ6 host delay; other FX in the owner bench are unchanged."""
import ctypes as C
import functools
import math
import os
from pathlib import Path
import subprocess
import tempfile
import numpy as np

@functools.lru_cache(maxsize=1)
def library():
    with tempfile.TemporaryDirectory(prefix='sj6-delay-') as tmp:
        out = Path(tmp)/'delay.so'
        subprocess.run([os.environ.get('CC','cc'), '-std=c11','-O2','-fPIC','-shared',
                        '-ffp-contract=off', str(Path(__file__).with_suffix('.c')), '-o',str(out),'-lm'],check=True)
        d=C.CDLL(str(out))
    fp=np.ctypeslib.ndpointer(dtype=np.float32, ndim=1, flags='C_CONTIGUOUS')
    d.sj6_new.argtypes=[C.c_double]
    d.sj6_new.restype=C.c_void_p
    d.sj6_params.argtypes=[C.c_void_p,C.c_double,C.c_float,C.c_float,C.c_int]
    d.sj6_process.argtypes=[C.c_void_p,C.c_size_t,fp,fp,fp,fp]
    d.sj6_free.argtypes=[C.c_void_p]
    for f in ('sj6_params','sj6_process','sj6_free'): getattr(d,f).restype=None
    return d

def delay(x, o, enabled, sample_rate=48000.):
    if not enabled:
        return x
    p=o.props if o is not None else {}
    def control(name, default):
        v=float(p.get(name,default))
        if not math.isfinite(v):
            raise ValueError(f'unresolved SuperJ6 delay control: {name}')
        return v
    tm=control('time',.3)
    fb=control('feedback',0.)
    # Build9's AudioKit AU forwards this property to the core without
    # inversion. The original core multiplies the dry input by it.
    dry=control('dryWetMix',.5)
    ping=control('pingPong',0.)>.5
    x=np.ascontiguousarray(x,dtype=np.float32)
    if x.ndim!=2 or x.shape[0]!=2:
        raise ValueError('expected planar stereo')
    y=np.empty_like(x)
    d=library(); state=d.sj6_new(sample_rate)
    if not state:
        raise ValueError('unable to allocate stereo delay')
    try:
        d.sj6_params(state,tm,fb,dry,ping)
        d.sj6_process(state,x.shape[1],x[0],x[1],y[0],y[1])
    finally:
        d.sj6_free(state)
    return y.astype(np.float64)
