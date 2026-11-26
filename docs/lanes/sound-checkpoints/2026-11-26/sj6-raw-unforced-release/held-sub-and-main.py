import argparse,hashlib,json
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser()
p.add_argument("original",type=Path)
p.add_argument("compact_cue",type=Path)
a=p.parse_args()
m=json.loads((a.original/'capture.json').read_text())
u=json.loads((a.compact_cue/'manifest.json').read_text())
assert m['wanted']['program']=='Raw J106 101' and m['mode']=='dry' and m['rate']==44100
v=np.load(a.original/'output.npy',allow_pickle=False)
assert hashlib.sha256(v.tobytes()).hexdigest()==m['pcm_sha256']
assert m['schedule'][1]['frame']==u['noteoff_abs_frame'] and m['schedule'][0]['frame']==16384
begin,end=u['abs_begin_frame'],u['abs_end_excluded_frame']
stop=u['noteoff_abs_frame']
def raw(f):
    z=(a.compact_cue/f).read_bytes()
    assert hashlib.sha256(z).hexdigest()==u["PCM_SHA256"][f]
    x=np.frombuffer(z,'<f4').reshape(-1,2)
    assert x.shape[0]==end-begin and np.isfinite(x).all()
    return x.mean(axis=1,dtype=np.float64)
x=v.mean(axis=0,dtype=np.float64)
signals={'original_terminal':x[begin:stop],
         'compact_center':raw('center-only.f32')[:stop-begin],
         'compact_with_physical_suffix':raw('center-plus-hp-dc.f32')[:stop-begin]}
hp0,hp1,hp2=map(float.fromhex,['0x1.ffa2ap-1','0x1.10427p-19','0x1.ff4564p-1'])
crush=float.fromhex('0x1.fd70a4p-1')
def H(f):
    z=np.exp(-2j*np.pi*f/44100)
    return hp0*(1-z)**2/((1-z)*(1-hp2*z)+hp1*z)*(1-z)/(1-crush*z)*(1-z)/(1-.999*z)
def row(tag,y,begin,stop):
    t=np.arange(begin,stop)/44100
    # Fix the physical note lattice from the shared MIDI note + original range.
    # Regression removes neighboring harmonics and DC leakage, not an A/B fit.
    semi=32.70319566257483
    phase=2*np.pi*semi*np.arange(1,13)[:,None]*t
    X=np.column_stack([np.ones(t.size),np.cos(phase).T,np.sin(phase).T])
    w=np.hanning(t.size)**.5
    vals=np.linalg.lstsq(X*w[:,None],y*w,rcond=None)[0]
    z=vals[1:13]-1j*vals[13:25]
    if tag=='original_source_equivalent':
        z=z/H(semi*np.arange(1,13))
    r=z*np.exp(-1j*np.arange(1,13)*np.angle(z[0]))/abs(z[0])
    db=lambda l: (20*np.log10(abs(l))).tolist()
    return dict(harmonics=list(range(1,13)),absolute_dBFS=db(z),relative_dBc=db(r),
                relative_phase_deg=np.angle(r,deg=True).tolist(),
                H1_absolute_phase_deg=float(np.angle(z[0],deg=True)))
out={'frames':[begin,stop],'scoped':'held sine-lattice coefficients, no renderer alignment or gain fit'}
for k,y in signals.items():out[k]=row(k,y,begin,stop)
out['original_source_equivalent']=row('original_source_equivalent',signals['original_terminal'],begin,stop)
print(json.dumps(out,indent=2))
