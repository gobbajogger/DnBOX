import hashlib,json,sys
from pathlib import Path
import numpy as np
bare,inst=map(Path,sys.argv[1:3])
def get(p,config):
    y=np.load(p,allow_pickle=False)
    assert y.dtype==np.dtype('<f4') and np.isfinite(y).all()
    assert hashlib.sha256(y.tobytes()).hexdigest()==config['pcm_sha256']
    return y.astype(np.float64)
def h(y):
    return np.fft.rfft(y[0,16384+8192:16384+8192+22050])[np.arange(1,11)*220]
def polar(z):
    return dict(dB=float(20*np.log10(max(1e-15,abs(z)))),phase_deg=float(np.angle(z,deg=True)))
def ri(z):return [float(z.real),float(z.imag)]
def ratio_set(y):
    mid=y['m5'][0]/y['m0'][0]
    end=y['m1'][0]/y['m0'][0]
    return dict(mid=polar(mid),full=polar(end),mid_over_full_response=ri((mid-1)/(end-1)) if abs(end-1)>1e-10 else None,
                mid_div_full_signal=ri(mid/end),full_signal=ri(end))
b={}
m=json.loads((bare/'bare.json').read_text())
for a in m['arms']:
    assert a['before']==a['after'] and a['before']['4']==0 and set(a['blocks'])=={128}
    b.setdefault((a['peak_in'],a['amount']),{})[a['policy']]=h(get(bare/a['file'],a))
i={}
for c in sorted(inst.glob('*/capture.json')):
    a=json.loads(c.read_text())
    assert a['component_version']==67072 and a['parameters']==a['parameter_readback_after'] and a['osc2_fader']==.5
    y=h(get(c.parent/'output.npy',a))
    _,_,on,amount,mix=a['ablation_arm'].split('-')
    i.setdefault((on,amount,a['repeat']),{})[mix]=y
assert len(m['arms'])==24 and sum(map(len,i.values()))==9
out={'bare':{str(k):ratio_set(v) for k,v in b.items()},
     'installed':{str(k):ratio_set(v) for k,v in i.items() if k[0]=='on' and not k[2]},
     'enabled_dry_over_disabled':polar(i[('on','a0',False)]['m0'][0]/i[('disabled','a0',False)]['m0'][0]),
     'a1_dry_over_a0_dry':polar(i[('on','a1',False)]['m0'][0]/i[('on','a0',False)]['m0'][0]),
     'a1_repeat_full':polar(i[('on','a1',True)]['m1'][0]/i[('on','a1',True)]['m0'][0])}
print(json.dumps(out,indent=2))
