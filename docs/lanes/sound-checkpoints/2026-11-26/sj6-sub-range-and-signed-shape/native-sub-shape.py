import argparse,hashlib,json
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser()
p.add_argument("reference",type=Path)
p.add_argument("--compact",type=Path)
a=p.parse_args()
def db(x):return float(20*np.log10(max(1e-15,abs(x))))
def read(p):
    m=json.loads((p/'capture.json').read_text())
    y=np.load(p/'output.npy',allow_pickle=False)
    assert y.dtype==np.dtype('<f4') and np.isfinite(y).all()
    assert hashlib.sha256(y.tobytes()).hexdigest()==m['pcm_sha256'] and sum(m['buffers'])==y.shape[1]
    assert m['rate']==44100 and m['warm_frames']==16384 and m['ablation_arm']=='sub-only'
    return np.mean(y.astype(np.float64),axis=0),m
def h(sig,start,stop,pitch,origin):
    t=(np.arange(start,stop)-origin)/44100.
    n=np.arange(1,13)
    gate=np.hanning(stop-start)
    return np.sum(np.exp(-2j*np.pi*pitch*n[:,None]*t)*(sig[start:stop]*gate),axis=1)*(2/gate.sum())
def describe(sig):
    onset,off=16384,38434
    nfft=441000
    mag=np.abs(np.fft.rfft(sig[onset+9000:off-1000]*np.hanning(off-1000-onset-9000),n=nfft))
    peak=int(mag[235:750].argmax()+235); mm=np.log(mag[peak-1:peak+2])
    part=.5*(mm[0]-mm[2])/(mm[0]-2*mm[1]+mm[2]); f=(peak+part)*44100/nfft
    coeff=h(sig,onset+8700,off-700,f,onset)
    units=np.arange(1,len(coeff)+1)
    rel_coeff=coeff*np.exp(-1j*units*np.angle(coeff[0]))/abs(coeff[0])
    attack=[]
    # One nominal fundamental period per window. Measure rather than normalize attack to a tuned row.
    for st in [0,1500,3500,6500,11000,15200,19200]:
        c=h(sig,onset+st,onset+st+1800,f,onset);attack.append([st,db(c[0])])
    for st in [0,1500,3500,6500,8500]:
        c=h(sig,off+st,off+st+1500,f,onset);attack.append([22050+st,db(c[0])])
    return dict(steady_f_Hz=float(f),H1_dBFS=db(coeff[0]),H1_phase_deg=float(np.angle(coeff[0],deg=True)),
        harmonics_relative_phase_deg=[float(v) for v in np.angle(rel_coeff,deg=True)],
        harmonics_relative_magnitude_dBc=[db(c) for c in rel_coeff],
        absolute_envelope_dBFS=attack)
x,m=read(a.reference/'native-warm-brainwave-fanin-authored-sub-only')
xr,mr=read(a.reference/'native-warm-brainwave-fanin-authored-sub-only-repeat')
assert m['schedule']==mr['schedule']
def established_high_zone(sig):
    # Last chord destination is key 67, low Range chooses 55 / root 54.
    # Use an established absolute frame range, separate from the pending commit.
    lo,hi=58000,79500
    z=sig[lo:hi]*np.hanning(hi-lo)
    mag=np.abs(np.fft.rfft(z,n=441000))
    j=int(850+np.argmax(mag[850:1090]))
    q=np.log(mag[j-1:j+2])
    f=(j+.5*(q[0]-q[-1])/(q[0]-2*q[1]+q[-1]))*.1
    c=h(sig,lo,hi,f,16384)
    n=np.arange(1,len(c)+1)
    rel=c*np.exp(-1j*n*np.angle(c[0]))/abs(c[0])
    return {"Hz":float(f),"H1_dBFS":db(c[0]),"H3_H5_H7_H9_dBc":[db(rel[j]) for j in [2,4,6,8]],
            "H3_H5_H7_H9_relative_degrees":[float(np.angle(rel[j],deg=True)) for j in [2,4,6,8]],
            "frames":[lo,hi]}
out={'original':describe(x),'original_repeat':describe(xr),'original_pcm_sha':m['pcm_sha256'],
     'established_next_zone':{'original':established_high_zone(x),'original_repeat':established_high_zone(xr)},
     'signed_repeat_dBr':db(np.linalg.norm(x-xr)/np.linalg.norm(x))}
if a.compact:
    y=np.fromfile(a.compact,'<f4')
    assert y.size==x.size*2
    out['compact_range_corrected']=describe(y.reshape(-1,2).astype(np.float64).mean(axis=1))
    yc=y.reshape(-1,2).astype(np.float64).mean(axis=1)
    out['established_next_zone']['compact_range_corrected']=established_high_zone(yc)
    out['compact_signed_dBr']=db(np.linalg.norm(x-yc)/np.linalg.norm(x))
    out['signed_windows']={}
    for at,end in [(0,128),(128,512),(512,2048),(2048,8192),(8192,22050),(22050,33075)]:
        sl=slice(16384+at,16384+end)
        nr,nc=np.linalg.norm(x[sl]),np.linalg.norm(yc[sl])
        out['signed_windows'][f'{at}:{end}']={'compact_level_minus_ref_dB':db(nc/nr),
            'signed_zero_shift_correlation':float(np.sum(x[sl]*yc[sl])/(nr*nc))}
    for name,v in [('original',x),('compact_range_corrected',yc)]:
        out[name]['first_zero_cross_frames']=[int(i+1) for i in np.flatnonzero(np.signbit(v[16384:20000][1:])!=np.signbit(v[16384:20000][:-1]))]
print(json.dumps(out,indent=2))
