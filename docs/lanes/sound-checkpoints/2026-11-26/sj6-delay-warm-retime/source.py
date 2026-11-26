import ctypes as C,json,numpy as n,contextlib,io
with contextlib.redirect_stdout(io.StringIO()):
 from test import M,origin,render,state,get
P=C.c_void_p
F=n.ctypeslib.ndpointer('f4',ndim=1,flags='C_CONTIGUOUS')
a=C.CDLL('./original.so');a.sj6_new.restype=P;a.sj6_new.argtypes=[C.c_double];a.sj6_params.argtypes=[P,C.c_double,C.c_float,C.c_float,C.c_int];a.sj6_process.argtypes=[P,C.c_size_t]+[F]*4;a.sj6_free.argtypes=[P]
rows=[]
for sr in (44100,48000):
 for nfb in (0,64,127):
  s=.99*nfb/127;ctl=lambda r:float(n.float32(n.float32(1.5/127)*r))
  old=a.sj6_new(sr);a.sj6_params(old,ctl(21),s,.1,0)
  x=origin(sr,40017);p=get(M,sr,0,21,nfb);q=get(M,sr,0,21,nfb)
  y=render(M,p,x,127);ny=render(M,q,-x,63);assert n.array_equal(y,-ny)
  yy=n.empty_like(x);a.sj6_process(old,x.shape[1],*x,*yy)
  a.sj6_params(old,ctl(42),s,.1,0);M.set(p,42,nfb,127,0,120.);M.set(q,42,nfb,127,0,120.)
  zeros=n.zeros((2,10017),'f4');out=n.empty_like(zeros)
  a.sj6_process(old,zeros.shape[1],*zeros,*out)
  r=render(M,p,zeros,127);nr=render(M,q,-zeros,61)
  assert n.array_equal(r,-nr), (sr,nfb)
  assert (abs(out[:,:800]).max(axis=1)>0).all() and (abs(r[:,:800]).max(axis=1)>0).all()
  rows.append(dict(fs=sr,feedback=nfb,reference_first800_peaks=abs(out[:,:800]).max(axis=1).tolist(),compact_first800_peaks=abs(r[:,:800]).max(axis=1).tolist(),signed_inversion_exact=True))
  M.free(p);M.free(q);a.sj6_free(old)
print(json.dumps(rows,indent=2))
