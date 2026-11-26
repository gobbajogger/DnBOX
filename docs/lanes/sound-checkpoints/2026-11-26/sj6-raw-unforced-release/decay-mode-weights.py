"""Identify exact physical free-response mode weights as diagnostics only."""
import argparse,hashlib,json
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser()
p.add_argument("original",type=Path)
p.add_argument("compact",type=Path)
a=p.parse_args()
j=json.loads((a.original/'capture.json').read_text())
r=np.load(a.original/'output.npy',allow_pickle=False)
assert j['mode']=='dry' and j['wanted']['program']=='Raw J106 101' and j['rate']==44100
assert hashlib.sha256(r.tobytes()).hexdigest()==j['pcm_sha256']
m=json.loads((a.compact/'manifest.json').read_text())
k0=m['abs_begin_frame'];n0=m['noteoff_abs_frame'];n1=n0+705
assert n0==j['schedule'][1]['frame']==38434
b=(a.compact/'center-plus-hp-dc.f32').read_bytes()
assert hashlib.sha256(b).hexdigest()==m['PCM_SHA256']['center-plus-hp-dc.f32']
x=np.frombuffer(b,'<f4').reshape(-1,2).T.astype(np.float64)
assert m['abs_end_excluded_frame']-k0==x.shape[1]
h0,h1,h2=[float.fromhex(v) for v in ['0x1.ffa2ap-1','0x1.10427p-19','0x1.ff4564p-1']]
z=max(np.roots([1,-1-h2+h1,h2]),key=lambda c:c.imag)
p1=float.fromhex('0x1.fd70a4p-1');p2=.999
def basis(idx):
    t=idx-n1; v=z**t
    return np.column_stack((v.real,v.imag,p1**t,p2**t))
train=np.arange(n1,n0+2205)
pure_late=np.arange(n0+2205,n0+8820)
def db(s):return float(20*np.log10(max(1e-15,float(s))))
def result(v,base):
    out=[]
    for series in v:
        T=basis(train)
        obs=series[train-base]
        curve=np.linalg.lstsq(T,obs,rcond=None)[0]
        pred=np.sum(basis(pure_late)*curve,axis=1)
        true=series[pure_late-base]
        parts=curve[None,:]*basis(pure_late)
        out.append(dict(
           physical_mode_coefficients=curve.tolist(),
           train_error_dBr=db(np.linalg.norm(np.sum(T*curve,axis=1)-obs)/np.linalg.norm(obs)),
           untouched_50_to_200ms_error_dBr=db(np.linalg.norm(pred-true)/np.linalg.norm(true)),
           holdout_mode_rms_dBFS=[db(np.sqrt(np.mean(parts[:,i]**2))) for i in range(4)],
           signed_start_value_16ms=float(sum(curve[[0,2,3]]))))
    return out
out=dict(gate_off=n0,mode_reference=n1,training=[int(train[0]),int(train[-1]+1)],
         holdout=[int(pure_late[0]),int(pure_late[-1]+1)],
         names=['HP10_cosine','HP10_sine','Crusher_DC','output_DC'],
         original=result(r.astype(np.float64),0),compact=result(x,k0))
print(json.dumps(out,indent=2))
