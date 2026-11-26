import ctypes as c
import hashlib
import json
from pathlib import Path
import sys
import numpy as np

base = Path(sys.argv[1])
libs = [c.CDLL(str(base / x)) for x in ('oracle.so',sys.argv[2])]
fptr = np.ctypeslib.ndpointer(np.float32, flags='C_CONTIGUOUS')
for lib in libs:
    lib.physical_replay.argtypes = [fptr, fptr, fptr, fptr, c.c_uint]
    lib.physical_replay.restype = None

def f32(v):
    return np.ascontiguousarray(v, dtype='<f4')

def compare(label, coeff, param, x):
    y = []
    for lib in libs:
        out = np.empty_like(x)
        lib.physical_replay(x, out, param, coeff, len(x))
        assert np.isfinite(out).all()
        y.append(out)
    a, b = (z.astype('f8') for z in y)
    diff = a - b
    def db(x):
        return 20 * np.log10(max(float(x), 1e-100))
    result = {'case': label, 'frames': len(x), 'dbr': db(np.linalg.norm(diff)/np.linalg.norm(a)),
              'peak_error': float(max(abs(diff))),
              'baseline_sha256': hashlib.sha256(y[0].tobytes()).hexdigest(),
              'candidate_sha256': hashlib.sha256(y[1].tobytes()).hexdigest()}
    return result

rng = np.random.default_rng(4186)
t = np.arange(8192, dtype='f8') / 44100.
input_signal = .4*np.sin(t*129*2*np.pi) + .22*np.sin(t*7391*2*np.pi) + .1*rng.standard_normal(t.size)
input_signal[4096:6144] = 0
results = []
# Independent fixed-coefficient synthetic fixtures, not original constants or rendering.
coefsets = [(0.1,0.5,0.27,0.9,0.03), (0.01,0.91,0.3,0.995,0.1),
            (0.12,0.31,0.75,0.88,0.3), (0.82,0.98,0.9,0.995,0.03)]
for index, coeffs in enumerate(coefsets):
    for g in [.004, .31277143955230713, 1.8208369016647339, 3.]:
        for lvl in [1e-6, .05, .5, 6.]:
            for blend in [0., 1.]:
                p = f32([g, .00035606283927336335, blend, 1])
                results.append(compare([index,g,lvl,blend], f32(coeffs), p, f32(input_signal*lvl)))
summary = {'cases': len(results), 'words': sum(r['frames'] for r in results),
           'worst': max(results,key=lambda r:r['dbr']), 'best': min(results,key=lambda r:r['dbr']),
           'scope': 'synthetic fixed all-pass coefficients only; not original renders or target qualification'}
print(json.dumps({'summary':summary,'results':results},indent=2))
