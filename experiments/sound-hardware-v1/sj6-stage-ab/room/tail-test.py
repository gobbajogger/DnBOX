import ctypes as C, numpy as np,sys,json
from pathlib import Path
P=Path(sys.argv[1]); n=24576; close=6144
x=np.zeros((2,n),dtype='f4');x[0,0]=.25;x[1,37]=-.125
ptr=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
res=[]
for name in ('c','tail'):
 L=C.CDLL(str(P/(name+'.so')))
 F=L.render_room;F.argtypes=[C.c_uint,C.c_uint,C.c_uint,ptr,ptr,C.c_uint];F.restype=C.c_float
 G=L.render_room_close;G.argtypes=[C.c_uint,ptr,ptr,C.c_uint];G.restype=C.c_float
 y=x.copy(); assert F(1,80,1,y[0],y[1],n)>0
 z=x.copy(); assert G(close,z[0],z[1],n)>0
 off=x.copy(); assert F(1,80,0,off[0],off[1],n)>0 and off.tobytes()==x.tobytes()
 res.append((y,z))
assert res[0][0].tobytes()==res[1][0].tobytes()
assert res[0][0].tobytes()==res[1][1].tobytes()
assert not np.any(res[0][1][:,close:]) and np.any(res[1][1][:,close:])
print(json.dumps(dict(frames=n,close=close,steady_active_exact=True,cold_off_exact=True,tail_continues_exactly_for_this_zero_send_test=True,state_bytes_delta=0,scope="host ablation only, no original tone parity or target timing")))
# Exact post-gate input has silence windows like the original: compare the
# proposed true send-close procedure with a running zero-fed reference.
from hashlib import sha256
c=6143; o=9971; c2=14211; o2=16607
x[1,o+35]=.25;x[0,o2+4]=-.31
sr=C.CDLL(str(P/'c.so')).render_room_toggle
st=C.CDLL(str(P/'tail.so')).render_room_toggle
for f in [sr,st]:
 f.argtypes=[C.c_uint,ptr,ptr,C.c_uint];f.restype=C.c_int
ref=x.copy(); candidate=x.copy(); restart=x.copy(); rejected=x.copy()
assert sr(0,ref[0],ref[1],n)==1
assert st(1,candidate[0],candidate[1],n)==1
assert st(1,restart[0],restart[1],n)==1
assert sr(1,rejected[0],rejected[1],n)==1
assert ref.tobytes()==candidate.tobytes()==restart.tobytes()
assert rejected[:,:c].tobytes()==ref[:,:c].tobytes()
assert all(np.any(ref[:,a:b]) and not np.any(rejected[:,a:b]) for a,b in [(c,o),(c2,o2)])
assert np.any(ref[:,o:]) and np.any(ref[:,o2:])
print(json.dumps(dict(frames=n,close=[c,c2],reopen=[o,o2],candidate_exact=True,fresh_owner_restart_exact=True,
  nonzero_tails_on_both_closed_intervals=True,exact_signed_sha=sha256(ref.tobytes()).hexdigest(),scope='same full-wet gated inlet; no held dry output equivalence asserted')))
