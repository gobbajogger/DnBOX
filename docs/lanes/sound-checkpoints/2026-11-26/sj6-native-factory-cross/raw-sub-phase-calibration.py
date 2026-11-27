"""Verify a separately authorized original calibration profile in the Raw source bench.

Arguments: authorized asset directory, hash-pinned native dry render directory,
and JSON note-to-cents mapping obtained separately. No calibration data, PCM,
or app code shipped here. Use the opposite sign as a negative control.
This independent linear source interpolation test is not the original sampler.
"""
import sys
from pathlib import Path
import hashlib,json
import numpy as np
from scipy.signal import lfilter
sys.path.insert(0,str(Path(__file__).resolve().parent.parent/'sj6-sub-source-rate-distortion'))
from source_probe import read
a=Path(sys.argv[1]); native=Path(sys.argv[2])
bank={k:x for k,_,x,_,_ in read(a/'Sounds/Osc/MySub.sfz',a/'J6P8R4N2S1')}
actual=np.load(native/'output.npy',allow_pickle=False)
m=json.loads((native/'capture.json').read_text())
assert hashlib.sha256(actual.tobytes()).hexdigest()==m['pcm_sha256']
assert m['mode']=='dry' and m['active']==m['after'] and m['rate']==44100
assert [(e['note'],e['frame']) for e in m['schedule'] if e['action']=='on' and e['frame']>16384]==[(60,49459),(64,49459),(67,49459)]
starts=[52000,55500,59000,62500,66000,69500,71534]
N=11000
win=np.hanning(N)
q=np.arange(33075)
norm=sum(win)*.5
rawA,rawB,rawC=map(float.fromhex,['0x1.ffa2ap-1','0x1.10427p-19','0x1.ff4564p-1'])
hpnum=[rawA,-2*rawA,rawA];hpden=[1,-1-rawC+rawB,rawC]
track={}
notes=json.loads(sys.argv[3])
for k,src in [(48,48),(52,54),(55,54)]:
 f=220*2**((k-69)/12)
 step=float(np.float32(2**((k-src)/12 - notes[str(k)]/1200)))
 v=q*step
 j=np.floor(v).astype('i4');w=v-j
 b=bank[src]
 hist=(b[j].astype('f8')*(1-w)+b[j+1]*w)/32768
 h=lfilter(hpnum,hpden,hist)
 h=lfilter([1,-1],[1,-float.fromhex('0x1.fd70a4p-1')],h)
 h=lfilter([1,-1],[1,-.999],h)
 assert np.isfinite(h).all()
 ref=actual[0]
 z=np.zeros(82534);z[49459:]=h
 def proj(sig):
  coeff=[np.sum(win*sig[i:i+N]*np.exp(-2j*np.pi*f*np.arange(i,i+N)/44100))/norm for i in starts]
  return np.unwrap(np.angle(coeff)),np.abs(coeff)
 measured,lev=proj(ref)
 model,ml=proj(z)
 t=(np.array(starts)-starts[0])/44100
 def drift(w):
  p=np.polyfit(t,w,1);return [float(p[0]/(2*np.pi)),float(np.max(abs((w-(t*p[0]+p[1]))*180/np.pi)))]
 track[k]={'input_zone':src,'step':step,'measured_drift_hz,max_phase_residual_degree':drift(measured),
           'raw_asset_linear_interpolation_drift_hz,max_phase_residual_degree':drift(model),
           'observe_relative_phase_drift_hz,max_phase_residual_degree':drift(model-measured)}
print(json.dumps(track,indent=2))
