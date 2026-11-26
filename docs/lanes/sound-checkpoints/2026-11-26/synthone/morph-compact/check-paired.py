import ctypes,struct,json
from pathlib import Path
import numpy as np
d=Path(__file__).resolve().parent/'private/shared-tight'
a,b=[ctypes.CDLL(str(d/p)) for p in ('morph.so','morph-paired.so')]
for func in (a.exact_morph_sample,b.exact_morph_sample):
 func.argtypes=[ctypes.POINTER(ctypes.c_uint32),ctypes.c_float,ctypes.c_float,ctypes.c_int]
 func.restype=ctypes.c_float
edges=[440.,8.,15.,30.,60.,120.,250.,500.,1000.,2150.,4400.,11000.,22000.]
tests=0
for k,f in enumerate(edges):
 for mix in (0.,.17444,1/3,.48,2/3,.88,1.):
  for p in range(4096):
   phase=(p<<12)|((p*1031)&4095)
   x=ctypes.c_uint32(phase);y=ctypes.c_uint32(phase)
   u=a.exact_morph_sample(ctypes.byref(x),f,mix,k!=0)
   v=b.exact_morph_sample(ctypes.byref(y),f,mix,k!=0)
   assert struct.pack('<f',u)==struct.pack('<f',v) and x.value==y.value,(k,mix,p,u,v)
   tests+=1
print(json.dumps(dict(phase_chord_cases=tests,output_bits_exact=True,phase_bits_exact=True)))
