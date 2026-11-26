"""Locate held Sub fundamental without signed alignment or gain fitting."""
from pathlib import Path
import io,json,sys,tarfile,hashlib
import numpy as np
o=Path(sys.argv[1]); ab=Path(sys.argv[2]); cu=Path(sys.argv[3])
n=44100;i,j=24576,37634
def strongest(x):
    assert np.isfinite(x).all() and x.shape==(j-i,)
    p=abs(np.fft.rfft(x*np.hanning(len(x)),n))
    return int(np.argmax(p[15:300]))+15
def d(p):
    return strongest(np.fromfile(p,'<f4').reshape(-1,2)[i:j,0])
with tarfile.open(o) as t:
    p='native-warm-brainwave-fanin-authored-sub-only'
    x=np.load(io.BytesIO(t.extractfile(p+'/output.npy').read()),allow_pickle=False)
    m=json.load(t.extractfile(p+'/capture.json'))
    assert hashlib.sha256(x.tobytes()).hexdigest()==m['pcm_sha256']
whole=dict(native_ranged=strongest(x[0,i:j]),compact_before=d(ab/'sub-only.private.f32'),
           compact_after=d(cu/'sub-only.private.f32'),units='Hz FFT-bin')
assert [whole[k] for k in ['native_ranged','compact_before','compact_after']]==[33,66,33],whole
print(json.dumps(whole,indent=2))
