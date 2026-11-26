"""Evaluate whether relative harmonics change with an actual amplitude sweep."""
import hashlib,json,sys
from pathlib import Path
import numpy as np
wave,sine=map(Path,sys.argv[1:3]); read=lambda p:json.loads(p.read_text())
case='native-jupiter-sine-gain-authored-compressor-'
for_set={'sine':(sine,case)}
for wave_name in ('tri','saw','pulse'):for_set[wave_name]=(wave,case+wave_name+'-')
result={}
for shape,(root,prefix) in for_set.items():
 acts={}
 for p in root.glob(prefix+'*'):
  m=read(p/'capture.json')
  if not m['ablation_arm'].endswith('-off'):continue
  data=np.load(p/'output.npy',allow_pickle=False)
  assert hashlib.sha256(data.tobytes()).hexdigest()==m['pcm_sha256']
  f=float(m['osc2_fader']); r=bool(m['repeat'])
  start,stop=25000,55000
  v=data[:,start:stop].astype(float); z=2*np.pi*440*(np.arange(start,stop)-16384)/44100
  X=np.stack([trig(k*z) for trig in (np.sin,np.cos) for k in range(1,7)]+[np.ones(len(z))],1)
  a=np.linalg.lstsq(X,v.T,rcond=None)[0]; c=a[6:12]-1j*a[:6]
  relative=np.abs(c)/np.abs(c[:1])
  phase=np.angle(c/np.exp(1j*np.arange(1,7)[:,None]*np.angle(c[:1])),deg=True)
  acts[f,r]={'fader':f,'repeat':r,'rms':np.sqrt(np.mean(v*v,axis=1)).tolist(),'db':(20*np.log10(np.maximum(relative,1e-15))).tolist(),'phase_invariant_degrees':phase.tolist()}
 rows=[acts[k] for k in sorted(acts)]
 full=acts[1.,False];half=acts[.5,False]
 drive=20*np.log10(np.array(full['rms'])/half['rms'])
 diff=np.array(full['db'])-half['db']
 result[shape]={'half_to_full_level_db':drive.tolist(),'half_to_full_harmonic_change_db':diff.tolist(),'scopes':rows}
print(json.dumps(result,indent=2))
