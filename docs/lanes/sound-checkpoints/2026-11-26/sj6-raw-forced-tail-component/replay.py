from pathlib import Path
from scipy.signal import lfilter
import hashlib,json,sys
import numpy as np
o,c=map(Path,sys.argv[1:]); m=json.loads((c/'manifest.json').read_text());v0=np.load(o/'output.npy',allow_pickle=False)
om=json.loads((o/'capture.json').read_text())
assert hashlib.sha256(v0.tobytes()).hexdigest()==om['pcm_sha256']
start,end,off=(m[q] for q in ('abs_begin_frame','abs_end_excluded_frame','noteoff_abs_frame'))
assert om['schedule'][1]['frame']==off==38434
def raw(k):
 b=(c/k).read_bytes();assert hashlib.sha256(b).hexdigest()==m['PCM_SHA256'][k]
 return np.frombuffer(b,'<f4').reshape(-1,2).T.astype('f8')
x=raw('center-only.f32'); y=raw('center-plus-hp-dc.f32')
v=v0[:,start:end].astype('f8')
p=off+560-start; e=off+1024-start
# Same chronological state in both arms by computing only the additive
# difference relative to unchanged, with zero delta state at divergence.
# Works only for the previously scoped fixed linear physical suffix.
fd=np.zeros_like(x)
fd[:,p:]=x[:,p:]
hp0,hp1,hp2=map(float.fromhex,['0x1.ffa2ap-1','0x1.10427p-19','0x1.ff4564p-1'])
crush=float.fromhex('0x1.fd70a4p-1')
z=lfilter([hp0,-2*hp0,hp0],[1.,-1-hp2+hp1,hp2],fd,axis=1)
z=lfilter([1.,-1.],[1.,-crush],z,axis=1)
z=lfilter([1.,-1.],[1.,-.999],z,axis=1)
assert np.count_nonzero(z[:,:p])==0 and np.max(abs(z))>0
def score(a,b):
 n=np.linalg.norm
 return {'signed_dbr':float(20*np.log10(n(a-b)/n(b))),'level_db':float(20*np.log10(n(a)/n(b)))}
rows={}
for tag,lo,hi in [('owner_release',off-start,p),('next_464_frames',p,e),('remaining_tail',e,end-start),('whole_release',off-start,end-start)]:
 f=(slice(None),slice(lo,hi))
 rows[tag]={'baseline':score(y[f],v[f]),'conditional_no_extra_drive':score((y-z)[f],v[f]),
            'extra_drive_rms':float(np.sqrt(np.mean(z[f]**2)))}
print(json.dumps({'scope':'historical signed compact physical-suffix proof only; no reset, current-branch or original shared-tap claim', 'windows':rows},indent=2,allow_nan=False))
