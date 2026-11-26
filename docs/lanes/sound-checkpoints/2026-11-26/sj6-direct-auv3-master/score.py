"""Score the four captures from AVAudioEngine's direct SuperJ6 AUv3 host."""
import hashlib
import json
from pathlib import Path
import numpy as np

def read(arm, mode):
    path = Path(f'native-warm-brainwave-master-{mode}-{arm}')
    state = json.loads((path/'capture.json').read_text())
    sig = np.load(path/'output.npy', allow_pickle=False)
    assert sig.shape == (2, 148684) and np.isfinite(sig).all()
    assert state['pcm_sha256'] == hashlib.sha256(sig.astype('<f4').tobytes()).hexdigest()
    assert state['installed_au_sha256'] == '1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
    assert state['component'] == 'aumu/j6ct/AuPr' and state['component_version'] == 67072
    assert state['preset_name'] == 'BRAINWAVE MONO BASS' and state['rate'] == 44100
    return state, sig.astype('float64')

result = {}
for arm in ('main-only', 'sub-only'):
    a, x = read(arm, 'authored')
    b, y = read(arm, 'half')
    assert a['schedule'] == b['schedule'] and b['actual_master']*2 == a['actual_master']
    db = lambda value: float(20*np.log10(value))
    result[arm] = {'PCM_SHA256': [a['pcm_sha256'], b['pcm_sha256']],
                   'master': [a['actual_master'], b['actual_master']], 'windows': {}}
    for window, start, end in [('whole', 0, len(x[0])), ('single-hold', 21000, 38000),
                               ('chord-hold', 55000, 80000), ('after-release', 82534, 148684)]:
        v, w = x[:,start:end], y[:,start:end]
        nv, nw = np.linalg.norm(v), np.linalg.norm(w)
        result[arm]['windows'][window] = {'bounds': [start,end], 'half_vs_authored_dB': db(nw/nv),
            'double_half_residual_dBr': db(np.linalg.norm(2*w-v)/nv), 'correlation': float(np.sum(v*w)/(nv*nw))}
print(json.dumps(result,indent=2,sort_keys=True))
