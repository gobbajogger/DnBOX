from pathlib import Path
import hashlib,json,sys
import numpy as np
p,native,baseline=map(Path,sys.argv[1:])
n=np.load(native,allow_pickle=False)
h=lambda a:hashlib.sha256(a.tobytes()).hexdigest()
assert n.shape==(2,132300) and h(n)=='e3884e0b9ba777d056d91d3a05e85f03cda1d3aef497b3a98854570649542453'
def arr(x):return np.fromfile(x,'<f4').reshape(2,-1)
rms=lambda a:np.sqrt(np.mean(a.astype('d')**2,axis=-1))
db=lambda a,b:(20*np.log10(rms(a)/rms(b))).tolist()
out={}
windows={'attack':[0,2205],'held':[11025,22050],'initial_release':[22050,26460]}
for mode in (0,3):
 t=arr(p/f'terminal{mode}.f32')
 mix=arr(p/f'mix{mode}.f32')
 assert h(t)==h(arr(baseline/f'defaults-{mode}.f32'))
 g=np.array([1,0 if mode else 1],dtype='f4')
 raw=np.sum(mix*g[:,None],axis=0)
 expected=np.sum((mix*(g/np.float32(6))[:,None])*np.float32(.5),axis=0)
 assert all(np.array_equal(t[ch],expected) for ch in range(2))
 # Arithmetic counterfactual, not a proposed change or original mixer reconstruction.
 hypothetical=raw*np.float32(.5)
 rows={}
 for name,(i,j) in windows.items():
  q=slice(i,j)
  rows[name]={'terminal_gain_vs_pre_pan_db':db(t[:,q],raw[q]),
   'terminal_level_vs_native_db':db(t[:,q],n[:,q]),
   'no_div6_arithmetic_level_vs_native_db':db(hypothetical[q],n[:,q]),
   'no_div6_arithmetic_signed_error_dbr':db(hypothetical[q]-n[:,q],n[:,q])}
 out[str(mode)]=dict(tap_sha=h(mix),terminal_sha=h(t),windows=rows)
print(json.dumps(dict(native_terminal_sha=h(n),cases=out),indent=2))
