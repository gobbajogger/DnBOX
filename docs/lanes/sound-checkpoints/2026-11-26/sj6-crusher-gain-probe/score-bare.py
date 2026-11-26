import hashlib,json,sys
from pathlib import Path
import numpy as np
d=Path(sys.argv[1])
m=json.loads((d/'bare.json').read_text())
assert m['component']=='aufx/dist/appl' and m['version']>0 and len(m['arms'])==28
arms={}
for a in m['arms']:
    y=np.load(d/a['file'],allow_pickle=False)
    assert y.dtype==np.dtype('<f4') and np.isfinite(y).all()
    assert hashlib.sha256(y.tobytes()).hexdigest()==a['pcm_sha256']
    assert a['before']==a['after'] and a['factory']['5']==a['before']['5'] and a['factory']['14']==a['before']['14']
    assert sum(a['blocks'])==y.shape[1] and a['before']['4']==0
    arms[(a['shape'],a['input_index'],a['policy'])]=(a['peak_in'],y[:,m['on']+4096:m['off']-1024].astype(np.float64))
def db(x):return 20*np.log10(max(x,1e-15))
def rel(x,r):return db(np.linalg.norm(x-r)/np.linalg.norm(r))
summary={}
for shape in ('sine','harmonic'):
    level_y=lambda i,p:arms[(shape,i,p)]
    _,base=level_y(2,'wet'); _,repeat=level_y(6,'wet')
    rows=[]
    for i in range(7):
        l,wet=level_y(i,'wet'); _,dry=level_y(i,'dry')
        rows.append(dict(input_peak=l,wet_vs_dry_dBr=rel(wet,dry),wet_gain_dB=db(np.linalg.norm(wet)/np.linalg.norm(dry)),
                     wet_normalized_residual_dBr=rel(wet/l,base/0.2)))
    summary[shape]=dict(repeat_dBr=rel(repeat,base),ladder=rows)
print(json.dumps(summary,indent=2))
