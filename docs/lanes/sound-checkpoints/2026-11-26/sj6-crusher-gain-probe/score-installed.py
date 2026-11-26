import hashlib,json,sys
from pathlib import Path
import numpy as np
b,n=map(Path,sys.argv[1:3])
def read(data,digest):
    y=np.load(data,allow_pickle=False)
    assert y.dtype==np.dtype('<f4') and np.isfinite(y).all()
    assert hashlib.sha256(y.tobytes()).hexdigest()==digest
    return y.astype(np.float64)
def harm(y,on=16384):
    seg=y[0,on+8192:on+8192+22050]
    return np.fft.rfft(seg)[np.arange(1,13)*220]
def ratio(w,d):return 20*np.log10(max(abs(w[0])/abs(d[0]),1e-15))
def residual(w,d):return 20*np.log10(max(np.linalg.norm(w-d)/np.linalg.norm(d),1e-15))
bare=json.loads((b/'bare.json').read_text())
rows={}
for a in bare['arms']:
    if a['shape']!='sine':continue
    rows[(a['input_index'],a['policy'])]=(a['peak_in'],harm(read(b/a['file'],a['pcm_sha256'])))
ladder=[]
for i in range(7):
    level,w=rows[(i,'wet')]; _,d=rows[(i,'dry')]
    ladder.append([level,ratio(w,d),float(np.angle(w[0]/d[0],deg=True))])
inst={}
for capture in sorted(n.glob('*/capture.json')):
    a=json.loads(capture.read_text())
    assert a['component']=='aumu/j6ct/AuPr' and a['component_version']==67072 and a['rate']==44100
    assert a['parameters']==a['parameter_readback_after'] and sum(a['buffers'])==a['frames']
    assert a['forced_controls']['107']==a['forced_controls']['109']==1
    wet=a['forced_controls']['108']==1
    f=a['osc2_fader']
    key=(f,wet,a['repeat'])
    inst[key]=read(capture.parent/'output.npy',a['pcm_sha256'])
assert len(inst)==8
results=[]
for f in (0.25,0.5,1.0):
    w,d=inst[(f,True,False)],inst[(f,False,False)]
    m1,m2=harm(w),harm(d)
    results.append(dict(fader=f,fundamental_wet_dry_dB=ratio(m1,m2),fundamental_phase_delta_deg=float(np.angle(m1[0]/m2[0],deg=True)),
                        dry_fund_peak=abs(m2[0])/11025,dry_harmonic_dBc=(20*np.log10(np.abs(m2[1:6]/m2[0]))).tolist(),
                        wet_harmonic_dBc=(20*np.log10(np.abs(m1[1:6]/m1[0]))).tolist()))
print(json.dumps(dict(bare_sine_fundamental_ladder=ladder,installed=results,repeats_dBr={str(w):residual(inst[(.5,w,True)],inst[(.5,w,False)]) for w in [False,True]}),indent=2))
