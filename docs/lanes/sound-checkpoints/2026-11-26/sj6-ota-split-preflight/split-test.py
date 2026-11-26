from ctypes import *
from struct import pack
from random import Random
import sys
so=CDLL(sys.argv[1]); fp=POINTER(c_float)
so.physical_tick.argtypes=[fp,c_float,fp,fp]
so.admit_split.argtypes=[fp]*5+[c_float,fp,fp,c_uint,c_uint,c_float]
so.physical_tick.restype=so.admit_split.restype=c_float
rng=Random(731)
arr=lambda n:(c_float*n)()
r=arr(96); a,b,c,e,poles=map(arr,[55,8,3,6,24])
ptr=lambda a,i:cast(byref(a,4*i),fp)
count=0
for i in range(65536):
    v=i%6
    x=rng.uniform(-2,2)
    p=(c_float*4)(.31277143955230713 if i&256 else 1.8208369016647339,.001,1,1)
    co=(c_float*5)(.22,.502,.10,.88,.025)
    # coefficients synthetic; ledger authentic coefficients remain externally guarded
    if i%53==0:
        before=tuple(bytes(z) for z in (a,b,c,e,poles))
        y=so.admit_split(a,b,c,e,ptr(poles,v*4),x,p,co,0,v,-0.)
        assert pack('f',y)==pack('f',-0.)
        assert before==tuple(bytes(z) for z in (a,b,c,e,poles))
    else:
        y=so.admit_split(a,b,c,e,ptr(poles,v*4),x,p,co,1<<v,v,-0.)
        z=so.physical_tick(ptr(r,v*16),x,p,co)
        assert pack('f',y)==pack('f',z)
        count+=1
print(f"{count} signed samples bit-identical across all six independent voices; {65536-count} disabled views left all owner bytes unchanged")
