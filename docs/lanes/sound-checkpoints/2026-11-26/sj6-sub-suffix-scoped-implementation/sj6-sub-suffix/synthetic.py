"""Full-rate f64 analytical comparison for synthetic F32 input."""
import sys,json,numpy as np
from scipy.signal import lfilter
a=np.fromfile(sys.argv[1],dtype='<f4').reshape(2,2,44100).astype('f8')
om=2*np.pi*10/44100
c,s=np.cos(om),np.sin(om)
d=1+s/2
x=lfilter([(1+c)/(2*d),-(1+c)/d,(1+c)/(2*d)],[1,-2*c/d,(1-s/2)/d],a[0])
for p in (.995,.999):x=lfilter([1,-1],[1,-float(np.float32(p))],x)
err=np.linalg.norm(a[1]-x,axis=1)
score=20*np.log10(err/np.linalg.norm(x,axis=1))
assert np.all(score<-80),score
print(json.dumps(dict(synthetic_only=True,frequencies_hz=[32.70319566257483,97.99885899543733],whole_signed_transfer_dbr=score.tolist()),indent=2))
