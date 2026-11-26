import ctypes as C, pathlib, numpy as np,json
d=pathlib.Path(__file__).resolve().parent; p=C.c_void_p
fp=np.ctypeslib.ndpointer('f4',ndim=1,flags='C_CONTIGUOUS')
for name in ('equal-clock','heterodyne'):
 s=C.CDLL(str(d/(name+'.so')));s.heterodyne_new.restype=p;s.heterodyne_new.argtypes=[C.c_uint,C.c_float]
 s.heterodyne_params.argtypes=[p,C.c_float,C.c_float,C.c_float,C.c_uint];s.heterodyne_process.argtypes=[p,C.c_size_t]+[fp]*4;s.heterodyne_free.argtypes=[p]
 for fs in (44100,48000):
  n=fs*4; rng=np.random.default_rng(41412)
  x=(rng.uniform(-1,1,(2,2,n))*.125).astype('f4'); x[:,:,fs*3:]=0
  # Explicit signed-zero dry preservation.
  x[0,0,fs*2]=-0.0
  ctl=[(.248,.499,.1,0),(.496,.83,.1,1),(.496,.83,1,1),(.32,.83,.1,0)]
  def arm(interleave,reject):
   ps=[s.heterodyne_new(fs,520) for i in range(2)];z=np.empty_like(x)
   order=[(j,i) for j in range(4) for i in range(2)] if interleave else [(j,i) for i in range(2) for j in range(4)]
   for j,i in order:
    assert s.heterodyne_params(ps[i],*ctl[j])==1
    if reject and j>0:
     for t in (0,.001,.7,1.5,16):
      assert s.heterodyne_params(ps[i],t,.001,.04,1)==0
    sl=slice(j*fs,(j+1)*fs)
    s.heterodyne_process(ps[i],fs,x[i,0,sl],x[i,1,sl],z[i,0,sl],z[i,1,sl])
    if j==2:assert np.array_equal(z[i,:,sl].view('u4'),x[i,:,sl].view('u4'))
   for u in ps:s.heterodyne_free(u)
   assert np.isfinite(z).all()
   return z
  a=arm(False,False);b=arm(True,True)
  assert a.tobytes()==b.tobytes()
  print(json.dumps(dict(variant=name,sr=fs,owners=2,frames_per_owner=n,rejected_requests_do_not_mutate=True,independent_simultaneous_owner_interleaving_bit_exact=True,signed_dry_bypass_exact=True,live_reroute_finite=True,sequence=ctl,scope='abrupt original-kernel setter witness; not wrapper ramps or full product')))
