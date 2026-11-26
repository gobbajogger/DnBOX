import hashlib,json,sys
from pathlib import Path
import numpy as np
bare,first,matrix,live=map(Path,sys.argv[1:5])
def read(p,a):
    y=np.load(p,allow_pickle=False)
    assert y.dtype==np.dtype('<f4') and np.isfinite(y).all()
    assert hashlib.sha256(y.tobytes()).hexdigest()==a['pcm_sha256']
    return y.astype(np.float64)
def h(y,at=24576,n=22050):
    return np.fft.rfft(y[0,at:at+n])[round(n/44100*440)]/(n/2)
def db(z):return float(20*np.log10(abs(z)))
def phase(z):return float(np.angle(z,deg=True))
m=json.loads((bare/'bare.json').read_text())
assert m['component']=='aufx/dist/appl' and m['version']==67072 and len(m['arms'])==48
by={}
for a in m['arms']:
    assert a['before']==a['after'] and a['before']['4']==0 and set(a['blocks'])=={128}
    by.setdefault(a['peak_in'],{})[(round(a['amount'],2),a['policy'])]=h(read(bare/a['file'],a))
peaks=np.array(sorted(by))
ratio=np.array([by[x][(6.06,'m3')]/by[x][(0,'m3')] for x in peaks])
enable=np.array([1.3*by[x][(0,'m3')]/by[x][(0,'m0')] for x in peaks])
def pressure(z,curve,independent):
    phases=np.angle(curve,deg=True)
    order=np.argsort(phases)
    estimate=float(np.interp(phase(z),phases[order],peaks[order]))
    magnitude_range=np.abs(curve)
    mag_pred=float(np.interp(estimate,peaks,magnitude_range))
    return {'peak_phase_estimate':estimate,'dB_at_estimate':db(mag_pred),'observed_dB':db(z),
            'observed_phase_deg':phase(z),'magnitude_delta_dB':db(z)-db(mag_pred),
            'other_curve_at_estimate':[float(np.interp(estimate,peaks,abs(independent))),float(np.interp(estimate,peaks,np.angle(independent,deg=True)))]}
def official(folder):
    out={}
    for c in sorted(folder.glob('*/capture.json')):
        a=json.loads(c.read_text()); p={x['address']:x['identifier'] for x in a['parameters']}
        assert [p[str(i)] for i in (107,108,109)]==['neoncrushEnable','neoncrushAmount','neoncrushBitRate']
        assert '110' not in p and a['component_version']==67072 and a['rate']==44100
        assert sum(a['buffers'])==a['frames']
        out[(a['ablation_arm'],a['repeat'])]=read(c.parent/'output.npy',a)
    return out
primary=official(first); m=official(matrix); l=official(live)
assert len(primary)==8 and len(m)==9 and len(l)==4
result={}
for label in ['f250','f500','f1000']:
    d=primary[('crusher-active-'+label+'-dry',False)]
    w=primary[('crusher-active-'+label+'-wet',False)]
    result[label]=pressure(h(w)/h(d),ratio,enable)
d=m[('crusher-matrix-disabled-a0-m0',False)]
en=m[('crusher-matrix-on-a0-m0',False)]
r=h(en)/h(d)
z=h(l[('crusher-live-enable',False)],34816,4410)/h(l[('crusher-live-enable',False)],22528,4410)
z0=h(l[('crusher-live-baseline',False)],34816,4410)/h(l[('crusher-live-baseline',False)],22528,4410)
result.update(mid_enable_static=pressure(r,enable,ratio),mid_enable_held=pressure(z/z0,enable,ratio),
              matched_bare_table=[{'input_peak':float(x),'amount_ratio_dB':db(a),'amount_phase_deg':phase(a),
                                   'enable_ratio_dB':db(e),'enable_phase_deg':phase(e)} for x,a,e in zip(peaks,ratio,enable)])
print(json.dumps(result,indent=2))
