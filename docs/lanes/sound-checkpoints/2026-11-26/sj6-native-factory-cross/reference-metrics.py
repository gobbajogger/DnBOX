import hashlib,json,sys
from pathlib import Path
import numpy as np
d=Path(sys.argv[1]); arms={}
for p in d.glob('program-*/capture.json'):
    a=json.loads(p.read_text())
    y=np.load(p.parent/'output.npy',allow_pickle=False)
    assert y.dtype==np.dtype('<f4') and np.isfinite(y).all()
    assert hashlib.sha256(y.tobytes()).hexdigest()==a['pcm_sha256']
    assert a['active']==a['after'] and sum(a['blocks'])==a['frames']
    arms[(a['wanted']['program_uid'],a['mode'])]=(a,y[:,a['warm_frames']:].astype(np.float64))
def db(v):return float(20*np.log10(max(v,1e-15)))
def rms(z):return float(np.sqrt(np.mean(z*z)))
windows={'note':(4096,22050),'chord':(39000,66150),'early_tail':(66150,88200),'late_tail':(88200,132300)}
results={}
for uid in sorted({x[0] for x in arms}):
    a,wet=arms[(uid,'authored')]; _,dry=arms[(uid,'dry')]; _,rep=arms[(uid,'authored-repeat')]
    r={}
    for label,(s,e) in windows.items():
        v,w,z=wet[:,s:e],dry[:,s:e],rep[:,s:e]
        r[label]={'wet_rms_dBFS':db(rms(v)),'dry_rms_dBFS':db(rms(w)),
          'wet_minus_dry_dB':db(rms(v))-db(rms(w)),'authored_repeat_rms_delta_dB':db(rms(z))-db(rms(v))}
    results[a['wanted']['program']]=r
print(json.dumps(results,indent=2))
