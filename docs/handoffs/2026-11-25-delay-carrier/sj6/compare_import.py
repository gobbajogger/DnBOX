import ctypes as C
import hashlib, json, math
from pathlib import Path
import numpy as np
from sj6_stereo_delay import library
D=Path(__file__).resolve().parent
R=44100
fp=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
variants={}
for name in ('base','center','geometry','combined'):
    d=C.CDLL(str(D/(name+'.so')))
    d.compact.argtypes=[C.c_uint]*3+[fp,fp,C.c_uint]
    d.compact.restype=C.c_uint
    variants[name]=d
