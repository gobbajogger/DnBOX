"""Older authenticated ARM64 AKBooster on signed source-independent Float32 input."""
from emu import Emu
from hashlib import sha256
from pathlib import Path
import numpy as np
import paths,json
def sh(x):return sha256(x).hexdigest()
h=sh(Path(paths.BIN).read_bytes())
assert h=='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
e=Emu()
n=257
a=(np.random.default_rng(684).uniform(-1.9,1.9,(2,n))).astype('<f4')
a[0,:8]=[0,-0.,1,-1,.25,-.75,1.5,-1.9]
a[1,:8]=[-0.,0,-1,1,-.25,.75,-1.5,1.9]
create=0x10038c0e0; setparam=0x10038c1c4; run=0x10038c2b4
def arm(l,r,active,ramp=False):
 p=e.call(create,2,doubles=(44100.,))
 e.call(setparam,p,0,int(not ramp),floats=(l,))
 e.call(setparam,p,1,int(not ramp),floats=(r,))
 e.call(setparam,p,2,1,floats=(0.001,))
 e.call(0x10038c2a0 if active else 0x10038c2ac,p)
 lists=[e.calloc(0x40),e.calloc(0x40)]
 ibuf=[e.calloc(4*n),e.calloc(4*n)]
 obuf=[e.calloc(4*n),e.calloc(4*n)]
 for head,buf in zip(lists,(ibuf,obuf)):
  e.w32(head,2)
  for i,j in enumerate(buf):
   e.w32(head+8+i*16,1);e.w32(head+12+i*16,n*4);e.w64(head+16+i*16,j)
 for j in range(2):e.wr(ibuf[j],a[j].tobytes())
 e.w64(p+24,lists[0]);e.w64(p+32,lists[1])
 if ramp:
  for at,b in ((0,31),(31,86),(117,140)): e.call(run,p,b,at)
 else: e.call(run,p,n,0)
 z=np.stack([np.frombuffer(e.rd(k,n*4),dtype='<f4') for k in obuf])
 steady=a*np.array([l if active else 1.,r if active else 1.],dtype=np.float32)[:,None]
 if not ramp: assert z.tobytes()==steady.tobytes()
 else: assert z[:,128:].tobytes()==steady[:,128:].tobytes() and np.any(z[:,:16]!=steady[:,:16])
 return {'expected_same_input_gain':[l if active else 1.,r if active else 1.], 'actual_gain_at_unity_sample':[float(z[0,2]),float(-z[1,2])], 'samples':z.size,'signed_peak':z.max(axis=1).astype(float).tolist(),'output_sha256':sh(z.tobytes()),'bit_exact_steady':not ramp,'live_ramp_probe':[z[0,i].item()/a[0,i].item() for i in (2,7,8,15,16,31,32,63,64,127,128)] if ramp else None}
arms={k:arm(*v) for k,v in {
 'master_old_default':(.95,.95,True),'unity':(1,1,True),
 'muted':(0,0,True),'crusher_active_setting':(1.3,1.3,True),
 'classic_active_setting':(1.707,1.707,True),'stopped_asymmetric_guard':(.12,2.3,False),
 'asymmetric_live':(.95,1.3,True,True)}.items()}
print(json.dumps(dict(binary_sha256=h,input_pcm_sha256=sh(a.tobytes()),scope='old ARM64 DSP creation/set/process; known input at actual Booster kernel; no simulated Apple mixers, no attribution to current AU',arms=arms),indent=2))
