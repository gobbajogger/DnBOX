import hashlib,json,sys
from pathlib import Path
import numpy as np
c,t=map(Path,sys.argv[1:])
d=c/'native-j6-init-gain-master-0.5'
capture=json.loads((d/'capture.json').read_text())
n=np.load(d/'output.npy',allow_pickle=False)
assert hashlib.sha256(n.tobytes()).hexdigest()==capture['pcm_sha256']=='e3884e0b9ba777d056d91d3a05e85f03cda1d3aef497b3a98854570649542453'
q=slice(11025,22050)
n=n[:,q]
rms=lambda a:np.sqrt(np.mean(a.astype('float64')**2,axis=1))
b={}
for mode in range(4):
    x=np.fromfile(t/f'defaults-{mode}.f32','<f4').reshape(2,132300)[:,q]
    b[mode]={'held_relative_level_db':(20*np.log10(rms(x)/rms(n))).tolist(),
             'held_signed_residual_dbr':(20*np.log10(rms(x-n)/rms(n))).tolist()}
assert 2.8 < b[1]['held_relative_level_db'][0]-b[0]['held_relative_level_db'][0] < 2.9
assert -6.1 < b[2]['held_relative_level_db'][0]-b[0]['held_relative_level_db'][0] < -6
print(json.dumps({'compact_defaults': (t/'defaults-0.log').read_text().splitlines()[:2],
    'native_control_readbacks': {r['identifier']:r['value'] for r in capture['parameters']
        if r['identifier'] in ('neonpulseVolume','neonsawVolume','neonsustain','neonsustain2')},
    'cases': b},indent=2))
