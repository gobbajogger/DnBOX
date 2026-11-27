import argparse, hashlib, json
from pathlib import Path
import numpy as np

args = argparse.ArgumentParser()
args.add_argument('captures', type=Path)
args.add_argument('build', type=Path)
a = args.parse_args()
fmt = '<f4'
uid = '8733EFD7-4000-4C31-BC52-5AD9E4B707E1'
binary = '1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
h = lambda x: hashlib.sha256(x.tobytes()).hexdigest()
cue = None
def reference(name, velocity, fixed):
    global cue
    t = a.captures / ('native-j6-init-' + name)
    d = json.loads((t / 'capture.json').read_text())
    assert d['installed_au_sha256'] == binary and d['component'] == 'aumu/j6ct/AuPr'
    assert d['rate'] == 44100 and d['frames'] == 132300 and sum(d['buffers']) == 132300
    assert d['observed_state']['lastLoadedPreset.uid'] == uid
    assert d['preset_name'] == d['preset_name_after'] == 'Your Init Preset'
    assert d['schedule'] == [dict(action='on',frame=0,note=48,velocity=velocity),
                             dict(action='off',frame=22050,note=48,velocity=0)]
    if cue is None: cue = d['buffers']
    assert cue == d['buffers']
    assert d['fixed_velocity'] == fixed
    def values(k):
        return {r['identifier']:r['value'] for r in d[k]}
    baseline, live, after = map(values, ('baseline_parameters','parameters','parameter_readback_after'))
    assert len(live) == 170 and live == after
    assert {k for k in live if live[k] != baseline[k]} == (set() if fixed else {'neonfixedVelocityEnable'})
    x = np.load(t / 'output.npy', allow_pickle=False)
    assert x.dtype == np.dtype(fmt) and x.shape == (2,132300) and h(x) == d['pcm_sha256']
    return x

def compact(mode, velocity):
    f = a.build / f'compact-{mode}-{velocity}.f32'
    x = np.fromfile(f, dtype=fmt).reshape(2,132300)
    assert np.isfinite(x).all()
    return x

def rms(x): return np.sqrt(np.mean(np.square(x.astype('float64')),axis=1))
def level(x,y): return (20*np.log10(rms(x)/rms(y))).tolist()
r = reference('gain-master-0.5',32,True)
nr = {fixed:{v:reference(f'velocity-{label}-{v}',v,fixed) for v in (32,112)}
      for fixed,label in ((True,'on'),(False,'off'))}
assert np.array_equal(r,nr[True][32]) and np.array_equal(r,nr[True][112])
q = slice(11025,22050)
b = {}
for m,ref in ((0,nr[False]),(3,nr[True]),(6,nr[True])):
    x,y = (compact(m,v) for v in (32,112))
    b[m] = dict(velocity_high_over_low_db=level(y[:,q],x[:,q]),repeat_exact=np.array_equal(x,y),
                steady_level_relative_to_corresponding_reference_db=level(x[:,q],ref[32][:,q]),
                steady_residual_dbr=level((x-ref[32])[:,q],ref[32][:,q]),
                compact_hash32=h(x),compact_hash112=h(y))
assert b[3]['repeat_exact'] and not b[6]['repeat_exact']
original_taper = level(nr[False][112][:,q],nr[False][32][:,q])
assert np.max(np.abs(np.array(original_taper)-b[0]['velocity_high_over_low_db'])) < 1e-5
def harmonics(sound):
    x = sound[0,q].astype("float64")
    spec = abs(np.fft.rfft(x*np.hanning(len(x)),2**17))
    f = np.fft.rfftfreq(2**17,1/44100)
    band = np.flatnonzero((f>90)&(f<170))
    k = band[np.argmax(spec[band])]
    f0 = f[k]
    return dict(raw_peak_hz=f0, relative_h2_through_h9_db=[
        float(20*np.log10(np.max(spec[np.abs(f-f0*n)<3])/spec[k])) for n in range(2,10)])
print(json.dumps(dict(held_spectrum=dict(compact_fixed=harmonics(compact(3,32)), native=harmonics(r)),
                      native_binary_sha=binary,native_init_uid=uid,native_fixed_hash=h(r),
                      rate=44100, frames=132300, note=48, note_on=0, note_off=22050,
                      original_variable_velocity_high_over_low_db=original_taper,
                      steady_window=[q.start,q.stop],compact=b),indent=2))
