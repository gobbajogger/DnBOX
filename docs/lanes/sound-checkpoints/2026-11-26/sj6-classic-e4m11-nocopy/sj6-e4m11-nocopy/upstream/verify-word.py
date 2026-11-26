import ctypes,sys
import numpy as np
unit=ctypes.CDLL(sys.argv[1]); unit.enc.argtypes=[ctypes.c_uint32]; unit.enc.restype=ctypes.c_uint16
unit.dec.argtypes=[ctypes.c_uint16]; unit.dec.restype=ctypes.c_uint32
rng=np.random.default_rng(7089)
x=np.concatenate([rng.integers(0,2**32,500000,dtype=np.uint32),np.arange(0x3b7ff000,0x3b801000,dtype=np.uint32),np.arange(0x42fff000,0x43000800,dtype=np.uint32)])
mag=(x&0x7fffffff).view(np.float32).astype(np.float64)
y=np.array([unit.dec(i) for i in range(32768)],dtype=np.uint32).view(np.float32).astype(np.float64)
assert np.all(y[1:]>y[:-1])
right=np.clip(np.searchsorted(y,mag),1,32767); left=right-1
with np.errstate(invalid='ignore'):
    a=mag-y[left];b=y[right]-mag
    want=np.where((a<b)|((a==b)&((left&1)==0)),left,right).astype(np.uint16)
want[~np.isfinite(mag)]=0
want|=(x>>16).astype(np.uint16)&np.uint16(0x8000)
got=np.asarray([unit.enc(int(k)) for k in x],dtype=np.uint16)
np.testing.assert_array_equal(got,want)
print(f"{len(x)} finite/exception fixtures preserve nearest representable magnitude (ties even), saturation and sign")
