# SuperJ6 dry path: compact normalization isolated; installed mixer transfer still unobserved

Use the signed current original Main receiver output as a common input (private, no asset exports). At unity level the exact compact C reducer, extracted from dnbx_core.c, has about −21.5836 dB per centered output channel. It is a divide by six followed by a center pan weight of one half. At hard pan the active side is −15.5630 dB and the other is identically silent. This quantifies the reducer, but does not establish the original installed mixer's numerical transfer or prove that it alone causes the ~12 dB terminal difference sitting between these paths.

The separate live fader probe changed both original receiver gains from .808 to .404, off, then restored on a known warmed stimulus. The fader acts before IIR and the immediate signed input coefficient changes exactly as expected; the filter's prior state rings down on its own. The actual warmed post-voice-sum receiver stays unity, and all compared buffer words matched across 96 signed blocks. Compact already applies its fader before IIR. Do not move the fader, adjust master, or use an arbitrary scale to claim parity.

Existing installed AU captures are terminal audio only; the required paired tap is described below. Keep new test audio private. No production changes.
# Brainwave mixer gain identification

Saved archive SHA d3821515187f4c086cde7ecbff57bafbae216fbd86becd1ff356fb1fe4a5df9f.
Its archived Swift runJ6 (lines 153–156 and 248) attaches the AU to the **outer**
engine and records renderOffline only. The utility's standalone effect/player
path (lines 43–94) is not the instrument graph. Thus no installed sampler or
oscSection PCM is present. Same-cue detached Main output is a proxy, not a
measurement of the loaded sampler. Separate standalone known-input filter/mixer
receipts do not identify the installed shared graph. Even exact known invertible
filters yield only the input of that verified subchain; an unobserved shared
linear gain G permits the mixer output to be scaled by k and G by 1/k with no
change to any isolated/combined terminal arm. Therefore additivity/repeat bounds
do not bound absolute oscSection transfer without independent installed state.

Minimal deterministic probe, only from code with legitimate access to the internal
engine: same exact Brainwave OSC1-only arm including 16,384 warm frames and
stored callback plan; fresh owner, three other roles muted. Observe actual sampler
out and oscSection output bus zero before Unison/HP in the same render. Record
AVAudioTime sample positions, negotiated format and actual mixer destination
pan/volume plus output volume. Reject discontinuous or different stamps;
do not resample, synchronize by cross-correlation or silently take an outer
engine mixer tap. Save only private raw tap files and receipt. Use
`python sj6-reducer-absolute/score_direct.py <tap-directory>` to measure fixed
windows and test predetermined unity and compact-center hypotheses.
A stock public-only AU bench cannot install these taps by adding Swift calls to
the outer AVAudioEngine. No new Mac session or application changes attempted.

### probe.py

```python
"""Run exact pinned compact reduction on genuine current receiver's signed output (private in RAM)."""
from pathlib import Path
import ctypes as C, subprocess, tempfile
p0=Path('sj6-mix-boundary-recheck/wrapper.py')
exec(compile(p0.read_text().split('def run(')[0],str(p0),'exec'))
donor=Path('sj6-fold32-recovery/exact-donor/firmware/src/dsp/dnbx_core.c')
sha=hashlib.sha256(donor.read_bytes()).hexdigest()
assert sha=='70d0aa0ea1cf2192e50cd51b45dffe01533cde368c0a1778959d39312b0ce759'
src=donor.read_text()
tail=src[src.index('        left[k]=0; right[k]=0;'):].split('\n    }',1)[0]
with tempfile.TemporaryDirectory() as t:
 q=Path(t); (q/'r.c').write_text('typedef struct {float gain,pan;} dnbx_layer_params;\n#define DNBX_LAYERS 2\nvoid reduce(float *mix,float *left,float *right, float g,float pan){struct {struct {dnbx_layer_params params;} layer[2];} a={{{{g,pan}},{{g,pan}}}}; const unsigned k=0; __typeof__(a)*s=&a;'+tail+'\n}')
 subprocess.run(['cc','-O2','-ffp-contract=off','-shared','-fPIC',str(q/'r.c'),'-o',str(q/'r.so')],check=True)
 fn=C.CDLL(str(q/'r.so')).reduce
 F=C.c_float; P=C.POINTER(F);fn.argtypes=[P,P,P,F,F];fn.restype=None
 arms=[]
 for role in ['oscSampler1','oscSampler2']:
  d=sampler_owner(role);w=wrapped(d)
  for _ in range(128):w(128)
  e.call(0x100389c84,d.dsp,36,86,doubles=(0.,))
  a=np.concatenate([w(128) for _ in range(48)],axis=1)
  e.call(0x100389c8c,d.dsp,36,0)
  b=np.concatenate([w(128) for _ in range(24)],axis=1)
  arms.append(np.concatenate((a,b),axis=1))
 assert not e.unknown and all(np.array_equal(x[0],x[1]) and np.max(x)>0 and np.min(x)<0 for x in arms)
 result={}
 for name,indices in [('Main1',[0]),('Main2',[1]),('Both',[0,1])]:
  ref=sum(arms[i][0] for i in indices)
  result[name]={}
  for pan in [-1.,0.,1.]:
   out=[]
   for vals in zip(*(arms[i][0] if i in indices else np.zeros_like(ref) for i in range(2))):
    l,r=F(),F();fn((F*2)(*vals),C.byref(l),C.byref(r),1.,pan);out.append([l.value,r.value])
   x=np.float32(out).T
   def rms(a):return float(np.sqrt(np.mean(a.astype('f8')**2)))
   result[name][str(pan)]={n:dict(receiver_rms=rms(ref[s]),compact_rms=[rms(z[s]) for z in x],signed_residual_rms=[rms(z[s]-ref[s]) for z in x]) for n,s in [('attack',slice(0,512)),('held',slice(1024,6144)),('release',slice(6144,6704))]}
 print(json.dumps(dict(scope='unity layer gain. Exact C reducer fed true mono-equivalent receiver output; Both unity sum is a defined test comparator, not capture of installed Apple fan-in. No PCM export.',compact_sha=sha,receiver_sha=macho.EXPECTED_SHA,results=result),indent=2))
```

### score_direct.py

```python
"""Score two synchronous private NPY taps; never infer a mixer transfer from terminal PCM."""
import hashlib,json,sys
from pathlib import Path
import numpy as np
root=Path(sys.argv[1])
m=json.loads((root/'mixer-taps.json').read_text())
assert m['installed_au_sha256']=='1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
assert m['internal_graph'] is True and m['role'] in ('Main1','Main2','Sub','Noise')
assert m['muted_input_roles']==sorted(set(('Main1','Main2','Sub','Noise'))-{m['role']})
assert m['sample_rate']==44100
assert m['source_format']==m['mixer_format']=='stereo-planar-f32/44100'
assert m['source_frame']==m['mixer_frame']==0
assert m['source_bus']==m['mixer_bus']==0
assert m['mixer_tap_position']=='oscSection output before Unison and instrument HiPass'
for name in ('mixer_output_volume','input_destination_pan','input_destination_volume'):
 assert np.isfinite(float(m[name]))
x=[]
for key in ('sampler','oscSection'):
 a=(root/(key+'.npy')).read_bytes()
 assert hashlib.sha256(a).hexdigest()==m[key+'_npy_sha256']
 import io
 y=np.load(io.BytesIO(a),allow_pickle=False)
 assert y.dtype==np.dtype('<f4') and y.shape==(2,148684) and np.isfinite(y).all()
 x.append(y.astype('f8'))
def observe(s):
 a,b=[z[:,s] for z in x]
 norm=lambda z:np.sqrt(np.mean(z*z,axis=1))
 assert np.all(norm(a)>1e-8), 'source signal missing at this window'
 return dict(source_rms=norm(a).tolist(),mixer_rms=norm(b).tolist(),absolute_transfer=(norm(b)/norm(a)).tolist(),same_channel_unity_null_rms=norm(b-a).tolist(),compact_center_null_rms=norm(b-a/12).tolist())
res={n:observe(slice(l,r)) for n,l,r in [('first_attack',16384,16896),('held',28672,37634),('gate_release',38434,38994),('chord_held',67534,82534),('final_release',82534,83094)]}
print(json.dumps(res,indent=2))
```
