import argparse,hashlib,json
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser()
p.add_argument('bare',type=Path)
p.add_argument('solidify',type=Path)
a=p.parse_args()
by={}
for arm in json.loads((a.bare/'bare.json').read_text())['arms']:
    if arm['policy']!='m0':continue
    assert arm['before']==arm['after'] and arm['before']['15']==0
    y=np.load(a.bare/arm['file'],allow_pickle=False)
    assert y.dtype==np.dtype('<f4') and np.isfinite(y).all()
    assert hashlib.sha256(y.tobytes()).hexdigest()==arm['pcm_sha256']
    assert sum(arm['blocks'])==y.shape[1]
    by.setdefault(arm['peak_in'],{})[round(arm['amount'],2)]=y
assert len(by)==12
for modes in by.values():
    assert modes[0].tobytes()==modes[6.06].tobytes()
n=sum(v[0].size for v in by.values())
j=json.loads(a.solidify.read_text())
assert j['mode']=='dry' and j['wanted']['program']=='BASS: Solidify'
auth={int(x['address']):x['value'] for x in j['authored']}
dry={int(x['address']):x['value'] for x in j['active']}
assert auth[107]==1 and dry[107]==0 and 0<dry[108]<1 and dry[109]==0
assert all(dry[i]==0 for i in [11,16,17,22,23,24,28,40,41,59])
print(json.dumps({
  'reference_arm':'Solidify dry',
  'author_crusher_enable':auth[107],
  'dry_crusher_enable':dry[107],
  'dry_latent_amount':dry[108],
  'dry_bitrate':dry[109],
  'physically_matched_mix0_level_pairs':len(by),
  'signed_stereo_words_bit_identical':n,
  'guard':'Original disable was explicitly applied; no statement about an initialized public zero without a transition.'
},indent=2))
