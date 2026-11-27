#!/usr/bin/env python3
"""Predict safe held-out dry release windows using only pinned poles + observed initial output.

This does not extract a private stage or fit any filter, phase shift, gain or
preset. The local observability solve estimates only starting state from
terminal samples in the chosen prefix. Multi-rate time stamps stay frozen.
"""
import hashlib,json,sys
from pathlib import Path
import numpy as np
native=Path(sys.argv[1])
m=json.loads((native/'capture.json').read_text())
y=np.load(native/'output.npy',allow_pickle=False)
assert hashlib.sha256(y.tobytes()).hexdigest()==m['pcm_sha256']
assert m['mode']=='dry' and m['rate']==44100 and m['frames']==148684
assert m['active']==m['after'] and m['wanted']['program_uid']=='9F6F6313-DD03-437F-B2EC-BCF7904DF871'
assert all(x['value']==0 for x in m['active'] if int(x['address']) in [11,16,17,22,23,24,40,41,107])
assert {e['frame'] for e in m['schedule'] if e['action']=='off'}=={38434,82534}
hp_g=float.fromhex('0x1.10427p-19');hp_r=float.fromhex('0x1.ff4564p-1')
a=max(np.roots([1,-1-hp_r+hp_g,hp_r]),key=lambda p:p.imag)
d=float.fromhex('0x1.fd70a4p-1');e=.999
def basis(n):
    q=np.arange(n)
    f=a**q
    return np.array((f.real,f.imag,d**q,e**q)).T
basis_len=8192
A=basis(basis_len)
# More than four observations for conditioning; only this prefix is observed.
train=512
B=A[:train:8]
pinv=np.linalg.pinv(B)
def db(x):
    return float(20*np.log10(max(1e-18,float(x))))
def check(z):
    z=z.astype('f8')
    assert np.isfinite(z).all()
    init=np.einsum("ij,j->i",pinv,z[:train:8])
    est=np.einsum("ij,j->i",A,init)
    assert np.isfinite(est).all()
    return [dict(begin=i,end=j,nulldbr=db(np.linalg.norm(est[i:j]-z[i:j])/np.linalg.norm(z[i:j]))) for i,j in ((512,1536),(1536,4096),(4096,8192))]
proof={}
for key,off in (('solo',38434),('chord',82534)):
    rows={}
    for s in (0,128,256,384,448,512,560,576,640,768,1024):
        rows[str(s)]=[check(u[off+s:off+s+basis_len]) for u in y]
    proof[key]=rows
known=np.einsum("ij,j->i",A,np.array((.013,-.007,.002,.004))).astype('f4')
tone=.002*np.exp(-np.arange(basis_len)/5500)*np.cos(np.arange(basis_len)*2*np.pi*65.406/44100)
assert all(p['nulldbr'] < -115 for p in check(known))
assert check(known+tone)[0]['nulldbr'] > -38
print(json.dumps(dict(test='Only coefficients of initial state estimated from fixed prefix; all windows scored are held out; no gain or phase fit.',condition=float(np.linalg.cond(B)),
 positive=check(known),negative=check(known+tone),rows=proof),indent=2))
