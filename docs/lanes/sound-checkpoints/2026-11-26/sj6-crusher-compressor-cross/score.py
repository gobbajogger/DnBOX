import hashlib,json,sys
from pathlib import Path
import numpy as np
def read(p,a):
    y=np.load(p,allow_pickle=False)
    assert y.dtype==np.dtype('<f4') and np.isfinite(y).all()
    assert hashlib.sha256(y.tobytes()).hexdigest()==a['pcm_sha256']
    return y[:,24576:46626].astype(np.float64)
def h(y):return np.fft.rfft(y[0])[220]/11025
def rms(y):return np.sqrt(np.mean(np.square(y[0])))
def db(z):return float(20*np.log10(abs(z)))
def polar(z):
    return {'dB':db(z),'deg':float(np.angle(z,deg=True))}
d=Path(sys.argv[1])
inst={}
for p in sorted(d.glob('*/capture.json')):
    a=json.loads(p.read_text())
    assert a['component_version']==67072 and a['parameters']==a['parameter_readback_after'] and a['osc2_fader']==.5
    state,mode=a['ablation_arm'].split('-')[-2:]
    assert a['forced_controls']['107']==int(state=='one') and abs(a['forced_controls']['17']-(.02 if mode=='on' else 0))<1e-8
    assert a['cross_comp_transition']==state+(':'+'normal-only' if state=='cold' else ':0.02->0->target')
    inst[(state,mode,a['repeat'])]=read(p.parent/'output.npy',a)
assert len(inst)==8
b=Path(sys.argv[2])
s=json.loads((b/'result-scored.json').read_text())['analysis']['rms_0_0']
xp=np.asarray(s['bare_response_db']); yp=np.asarray(s['bare_amp'])
assert xp.shape==yp.shape
order=np.argsort(xp)
out={}
for state,rep in [('cold',False),('zero',False),('zero',True),('one',False)]:
    a=inst[(state,'off',rep)]; w=inst[(state,'on',rep)]
    ratio=db(rms(w)/rms(a))
    out[f'{state}_repeat{int(rep)}']={'off_h1_peak':float(abs(h(a))),'on_h1_peak':float(abs(h(w))),
        'on_over_off_h1':polar(h(w)/h(a)),'on_over_off_rms_dB':ratio,
        'compressor_equivalent_peak':float(np.interp(ratio,xp[order],yp[order]))}
out['forced_off_vs_cold_off']=polar(h(inst[('zero','off',False)])/h(inst[('cold','off',False)]))
out['forced_enable_vs_forced_off']=polar(h(inst[('one','off',False)])/h(inst[('zero','off',False)]))
print(json.dumps(out,indent=2))
