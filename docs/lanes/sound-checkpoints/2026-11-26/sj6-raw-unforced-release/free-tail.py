"""Test a *causal physical* zero-input hypothesis, not a fitted waveform."""
import argparse,hashlib,json
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser()
p.add_argument('original',type=Path)
args=p.parse_args()
def import_raw(folder):
    m=json.loads((folder/'capture.json').read_text())
    assert m['wanted']['program']=='Raw J106 101' and m['rate']==44100
    q=np.load(folder/'output.npy',allow_pickle=False)
    assert q.dtype==np.dtype('<f4') and np.isfinite(q).all()
    assert hashlib.sha256(q.tobytes()).hexdigest()==m['pcm_sha256']
    assert m['active']==m['after']
    # No synth tuning or gain changes. Assert the published dry edits on the A/B.
    if m['mode']=='dry':
        assert all(x['value']==0 for x in m['active'] if int(x['address']) in [11,16,17,22,23,24,40,41,107])
    return q[0].astype(np.float64),m
x,m=import_raw(args.original/'program-2-dry')
off=m['schedule'][1]['frame']; on=m['schedule'][0]['frame']
assert (on,off)==(16384,38434)
a,b,c=[float.fromhex(v) for v in ['0x1.ffa2ap-1','0x1.10427p-19','0x1.ff4564p-1']]
dc1=float.fromhex('0x1.fd70a4p-1');dc2=.999
hp=np.roots([1,-1-c+b,c])
def norm(v):return np.linalg.norm(v)
def db(v):return float(20*np.log10(max(1e-15,float(v))))
def suffix(z):
    out=np.zeros_like(z)
    hx=dx=dy=hy=x1=y1=x2=y2=0.
    for i,u in enumerate(z):
        d=u-hx; v=a*(d-dx)-b*hy+c*dy
        hy+=v; hx=u;dx=d;dy=v
        o1=hy-x1+dc1*y1; x1=hy;y1=o1
        o2=o1-x2+dc2*y2; x2=o1;y2=o2
        out[i]=o2
    return out.astype(np.float32).astype(np.float64)
def modal(z,start,stop,k):
    # On a truly unforced suffix a stride-k capture obeys a homogeneous
    # recurrence, with pole locations raised exactly to power k.
    polys=np.poly(np.r_[hp,dc1,dc2]**k)
    assert np.max(np.abs(np.imag(polys)))==0
    polys=np.real(polys)
    ix=np.arange(start+4*k,stop)
    v=z[ix]
    r=np.sum([polys[j]*z[ix-j*k] for j in range(5)],axis=0)
    return dict(output_rms_dBFS=db(norm(v)/np.sqrt(len(v))), modal_residual_dBr=db(norm(r)/norm(v)), physical_poles_stride=k)
def hfreq(z):
    win=z[on+8192:off-650]
    hz=np.fft.rfftfreq(441000,1/44100)
    c=np.abs(np.fft.rfft(win*np.hanning(len(win)),n=441000))
    r=np.where((hz>=22)&(hz<=290))[0]
    return float(hz[r[np.argmax(c[r])]])
f=hfreq(x)
t=np.arange(len(x),dtype=np.float64)
z=np.zeros_like(x)
z[on:off]=.33*np.sin(2*np.pi*f*(t[on:off]-on)/44100)+.15*np.sin(2*np.pi*2*f*(t[on:off]-on)/44100+.8)
free=suffix(z)
q=z.copy()
q[off:]=.1*np.sin(2*np.pi*f*(t[off:]-on)/44100)*np.exp(-(t[off:]-off)/2300)
continued=suffix(q)
out={'native_sha':m['pcm_sha256'],'off_frame':off,'dominant_hz_before_off':f}
for tag,w0,w1 in [('16_to_50_ms',705,2205),('50_to_100ms',2205,4410),('100_to_200ms',4410,8820)]:
    out[tag]={}
    for name,u in [('native_raw_dry',x),('simulated_true_unforced_tail',free),('simulated_continuing_source',continued)]:
        out[tag][name]=[modal(u,off+w0,off+w1,k) for k in [16,32,64]]
out["onset_of_unforced_tail"]={}
for name,u in [('native',x),('synthetic_removed_at_noteoff',free),('synthetic_continues',continued)]:
    out["onset_of_unforced_tail"][name]=[
        {"window_start_ms":first*1000/44100,**modal(u,off+first,off+first+512,32)}
        for first in [0,64,128,192,256,320,384,448,512,576,640,704]
    ]
print(json.dumps(out,indent=2))
