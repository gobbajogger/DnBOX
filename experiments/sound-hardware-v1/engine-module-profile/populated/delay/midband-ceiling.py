from pathlib import Path
import hashlib, json, os, subprocess
import numpy as np
D=Path(__file__).resolve().parent
B=D/'oracle-build'; F=D.parent/'s1-fx-parity/reference-build'
sr=44100
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def power(x):return np.mean(x*x,axis=0)
def db(a,b):return (10*np.log10(np.maximum(a,1.e-80)/np.maximum(b,1.e-80))).tolist()
bands={'mid_1_to_4k':(1000,4000),'body_dc_to_4k':(0,4000),'upper_4k_to_nyquist':(4000,22051),'mid_0p5_to_6k':(500,6000),'all_exact_guard':(0,22051)}
def record(o,c):
    assert o.shape==c.shape and np.isfinite(o).all() and np.isfinite(c).all()
    # Full record has identical original timestamps. Pad the error before
    # offline complex projection to avoid a circular wrap from last to first.
    err=c-o; n=len(o); padded=np.pad(err,((sr,sr),(0,0)))
    z=np.fft.rfft(padded,axis=0)
    f=np.fft.rfftfreq(len(padded),1/sr)
    arms={'bounded':c}
    for name,(lo,hi) in bands.items():
        pr=np.fft.irfft(z*((f>=lo)&(f<hi))[:,None],n=len(padded),axis=0)[sr:sr+n]
        arms[name]=c-pr
    assert np.max(abs(arms['all_exact_guard']-o))<2e-14
    windows={'whole':slice(None),'body':slice(0,-int(1.5*sr)),'release':slice(-int(1.5*sr),None),'last_second':slice(-sr,None)}
    return {k:{w:dict(signed_dBr=db(power((x-o)[sl]),power(o[sl])),error_dBFS=db(power((x-o)[sl]),1),level_delta_dB=db(power(x[sl]),power(o[sl]))) for w,sl in windows.items()} for k,x in arms.items()}
out={}
for id in (62,151):
    arms={}; logs={}
    for mode in ('reference','bounded'):
        tap=B/f'{id}-{mode}.trace'; pcm=B/f'{id}-{mode}.pcm'
        env={k:v for k,v in os.environ.items() if not k.startswith(('S1_','AB_','ARP_'))}
        env.update(S1_DELAY_SHAPE='1024',S1_DELAY_TAP_TRACE=str(tap))
        proc=subprocess.run([B/mode,F/'factory-inputs/public-morphs.f32',F/'commands/preset-commands.bin',str(id),str(pcm),'wet'],env=env,capture_output=True,text=True,check=True)
        t=np.fromfile(tap,'<f4').reshape(-1,6)
        p=np.fromfile(pcm,'<f4').reshape(-1,2)
        assert p.shape==(264600,2) and t.shape[0] in (264600,264728)
        arms[mode]=(t[-len(p):],p)
        logs[mode]=dict(tap_sha256=sha(tap),pcm_sha256=sha(pcm),result=proc.stdout.strip(),mode=proc.stderr.strip())
    a,ap=arms['reference']; b,bp=arms['bounded']
    assert a[:,:4].tobytes()==b[:,:4].tobytes()
    out[id]=dict(cue=dict(first_four_tap_input_columns_equal=True,reference_target=float(a[-1,0]),return_frames=len(a)),
                 sources=logs,delay_return=record(a[:,4:].astype('f8'),b[:,4:].astype('f8')),
                 final_mix_projection_for_orientation_only=record(ap.astype('f8'),bp.astype('f8')))
print(json.dumps(dict(scope='Cheating noncausal complex error cancellation at observed same-clock taps. No gain/time fitting; not a causal DSP or wet nonlinear rerender.',results=out),indent=2,allow_nan=False))
