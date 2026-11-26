"""Signed resampling bandwidth check for existing generated SuperJ6 renders."""
import hashlib
import json
from pathlib import Path
import sys
import numpy as np
from scipy.signal import resample_poly

PIN = '1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
COMPONENT = 'aumu/j6ct/AuPr'
CUE = [(16384, 48), (38434, 48), (49459, 60), (49459,64), (49459,67),
       (82534,60), (82534,64), (82534,67)]
WINDOWS = {
    'attack':(16384,18589), 'held':(20794,38434), 'first_release':(38434,39094),
    'chord':(53869,82534), 'final_release':(82534,83194), 'full':(16384,148684),
}
FACTORS = [2,3,4,6,8,12,16,24,32,48,64,96,128]

def read(root, tag):
    base = root / ('native-warm-brainwave-fanin-authored-' + tag)
    y = np.load(base/'output.npy', allow_pickle=False)
    m = json.loads((base/'capture.json').read_text())
    assert y.dtype == np.float32 and y.shape == (2,148684)
    assert np.isfinite(y).all() and m['frames'] == 148684
    assert m['rate'] == 44100 and m['master_mode'] == 'authored'
    assert m['installed_au_sha256'] == PIN and m['component'] == COMPONENT
    assert hashlib.sha256(y.tobytes()).hexdigest() == m['pcm_sha256']
    assert [(e['frame'],e['note']) for e in m['schedule']] == CUE
    return y.astype('f8'), m

def db(ratio):
    return float(20*np.log10(max(ratio,1e-100)))

def score(got, y):
    d = {}
    for name,(a,b) in WINDOWS.items():
        x,z = got[:,a:b],y[:,a:b]
        norm = np.linalg.norm(z)
        assert norm > 0
        d[name] = dict(signed_dbr=db(np.linalg.norm(x-z)/norm), level_db=db(np.linalg.norm(x)/norm))
    return d

def run(root):
    a, ma = read(root,'sub-only')
    b, mb = read(root,'sub-only-repeat')
    for k in ['parameters','parameter_readback_after','schedule','s4_pinned_identity','rate']:
        assert ma[k] == mb[k]
    p = []
    for f in FACTORS:
        dec = resample_poly(a,1,f,axis=1,window=('kaiser',9.0),padtype='line')
        for bits in (0,8,12,16):
            step = 2.0**(1-bits) if bits else None
            words = dec if not bits else np.rint(dec/step)
            if bits:
                assert np.max(words) < 2**(bits-1) and np.min(words) >= -2**(bits-1)
            w = words if not bits else words*step
            up = resample_poly(w,f,1,axis=1,window=('kaiser',9.0),padtype='line')
            up = up[:, :a.shape[1]]
            p.append(dict(factor=f, encoded_stereo_sample_words=w.size,
                          bits=bits, theoretical_stereo_bytes=0 if not bits else (w.size*bits+7)//8,
                          assumed_zero_phase_integer_origin=True,
                          metrics=score(up,a)))
    return dict(installed_capture_pin=PIN, capture_pcm_sha=ma['pcm_sha256'],
                capture_repeat_pcm_sha=mb['pcm_sha256'],
                reference_repeat_floor=score(b,a), sample_rate=ma['rate'], trials=p,
                scope='already rendered Sub-only Brainwave terminal, not actual raw sample zones or native decoder; no publishing original or derivative data')
if __name__ == '__main__':
    print(json.dumps(run(Path(sys.argv[1])),indent=2,allow_nan=False))
