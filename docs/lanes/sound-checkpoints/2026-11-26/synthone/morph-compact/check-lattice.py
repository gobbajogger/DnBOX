import argparse,ctypes,hashlib,json
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser()
p.add_argument('waves',type=Path);p.add_argument('directory',type=Path)
a=p.parse_args(); d=a.directory.resolve(); h=hashlib.sha256(a.waves.read_bytes()).hexdigest()
assert h=='0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5'
full=np.fromfile(a.waves,dtype='<f4').reshape(52,4097)[:,:4096]
lib=ctypes.CDLL(str(d/'morph.so'))
ref=ctypes.CDLL(str(d/'float.so'))
ck=lib.morph_lattice_test
ck.restype=ctypes.c_float
for l in (lib,ref):
 l.exact_morph_sample.argtypes=[ctypes.POINTER(ctypes.c_uint32),ctypes.c_float,ctypes.c_float,ctypes.c_int]
 l.exact_morph_sample.restype=ctypes.c_float
import os
os.environ["S1_EXACT_MORPH_WAVES"]=str(a.waves.resolve())
assert lib.exact_morph_load()==ref.exact_morph_load()==1
vals=np.array([[ck(r,i) for i in range(4096)] for r in range(52)])
err=vals-full.astype('f8')
spec=json.loads((d/'format.json').read_text())
assert lib.morph_rodata_bytes()==spec['bytes']
assert np.max(abs(err))<=spec['tolerance']+1e-7
assert np.array_equal(full[:4],vals[:4])
bands=np.array([0,8.178783416748047,16.357566833496094,32.71513366699219,65.43026733398438,131.25,262.5,525,1050,2205,4410,11025,22050],dtype='f4')
car=np.random.default_rng(41709)
cycles=[]
sums=den=0.;worst=0
for k,b in enumerate(bands):
 for tag,rate in enumerate([np.nextafter(b,np.float32(-np.inf)),b,np.nextafter(b,np.float32(np.inf))] if k else [440.]):
  r=np.float32(rate)
  for morph in [0,0.01001,.143,.33333334,.503,.6666667,.863,1,1.25004]:
   phase=int(car.integers(0,1<<24))
   x=ctypes.c_uint32(phase);y=ctypes.c_uint32(phase)
   for i in range(811):
    w=ref.exact_morph_sample(ctypes.byref(x),r,morph,k!=0)
    z=lib.exact_morph_sample(ctypes.byref(y),r,morph,k!=0)
    assert x.value==y.value
    delta=abs(w-z); worst=max(worst,delta);sums+=delta*delta;den+=w*w
   cycles.append((k,tag,morph))
print(json.dumps(dict(size=spec['bytes'], lattice_points=int(full.size), source_sha256=h,sample_max_error=float(np.max(abs(err))),sample_rms_error=float(np.sqrt(np.mean(err*err))), phase_samples=len(cycles)*811, continuous_sample_max_error=worst, continuous_rms_fraction=np.sqrt(sums/den),rms_by_row=np.sqrt(np.mean(err*err,axis=1)).tolist()),indent=2))
