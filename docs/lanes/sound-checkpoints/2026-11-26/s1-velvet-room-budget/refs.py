import numpy as np,ctypes as c,json
from scipy.signal import sosfilt,butter
r=c.CDLL('./source.so')
p=c.c_void_p;f=np.ctypeslib.ndpointer('f4',flags='C_CONTIGUOUS')
r.ref_new.restype=p;r.ref_drop.argtypes=[p];r.ref_run.argtypes=[p,f,f,f,c.c_uint]
N=176400; sr=44100
def run(fb,ch=0,at=0):
 x=np.zeros((N,2),'f4');x[at,ch]=.25;y=np.empty_like(x);v=r.ref_new()
 r.ref_run(v,x,y,np.full(N,fb,'f4'),N);r.ref_drop(v)
 return y.astype('f8')
def early(y,c):
 d=y[:4400,c];loc=np.where(abs(d)>.002)[0]; groups=np.split(loc,np.where(np.diff(loc)>60)[0]+1)
 return [(int(z[np.argmax(abs(d[z]))]),float(d[z].sum()),float(abs(d[z]).max())) for z in groups if len(z)]
if __name__=='__main__':
 for c in (0,1):
  y=run(.8,c)
  print(c,early(y,c),np.max(abs(y[:3800,1-c])))
