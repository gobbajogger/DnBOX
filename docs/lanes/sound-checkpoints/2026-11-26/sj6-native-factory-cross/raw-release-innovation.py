#!/usr/bin/env python3
"""Localize the last detectable dry source drive using fixed physical poles."""
import hashlib,json,sys
from pathlib import Path
import numpy as np
p=Path(sys.argv[1])
m=json.loads((p/'capture.json').read_text())
x=np.load(p/'output.npy',allow_pickle=False)
assert x.dtype==np.dtype('<f4') and x.shape==(2,148684) and np.isfinite(x).all()
assert hashlib.sha256(x.tobytes()).hexdigest()==m['pcm_sha256']
assert m['mode']=='dry' and m['rate']==44100 and m['active']==m['after']
assert m['wanted']['program_uid']=='9F6F6313-DD03-437F-B2EC-BCF7904DF871'
assert {e['frame'] for e in m['schedule'] if e['action']=='off'}=={38434,82534}
assert all(z['value']==0 for z in m['active'] if int(z['address']) in [11,16,17,22,23,24,40,41,107])
G=float.fromhex('0x1.10427p-19')
R=float.fromhex('0x1.ff4564p-1')
poles=np.r_[np.roots([1,-1-R+G,R]),float.fromhex('0x1.fd70a4p-1'),.999]
result={}
for label,t in [('first',38434),('chord',82534)]:
    result[label]={}
    for k in (2,4,8,16,32,64):
        c=np.poly(poles**k).real
        w=x[:,t-4*k:t+2400].astype(np.float64)
        z=np.sum([c[j]*w[:,(4-j)*k:(4-j)*k+2400] for j in range(5)],axis=0)
        lsb=np.abs(np.spacing(x[:,t-4*k:t+2400])).astype(np.float64)
        bound=np.sum([abs(c[j])*.5*lsb[:,(4-j)*k:(4-j)*k+2400] for j in range(5)],axis=0)
        quiet=np.maximum(5e-8,np.quantile(abs(z[:,1280:]),.999,axis=1,keepdims=True)*4)
        high=abs(z)>np.maximum(bound*4,quiet)
        detect=np.where(high[:,:1280].any(axis=0))[0]
        result[label][k]=dict(last_endpoint=int(detect[-1]) if len(detect) else None,
                              stencil_support=4*k,threshold=quiet.ravel().tolist())
expected={2:566,4:574,8:590,16:622,32:686,64:814}
assert all({k:v['last_endpoint'] for k,v in row.items()}==expected for row in result.values())
print(json.dumps(dict(scope="Last observed stencil endpoints are exactly final reachable +558 plus each stencil width. Not a private gate or tap measurement.", result=result),indent=2))
