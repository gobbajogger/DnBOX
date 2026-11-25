"""Measure backed-up reference WAVs against rerendered compact headless arms."""
import argparse
import hashlib
import json
from pathlib import Path
import numpy as np
import wave

SR=44100
WINDOW=2048
GAIN_DB={1:0,62:-4.57,113:0,128:-1.10,278:0,435:-2.09}
def rms(x,axis=None):return np.sqrt(np.mean(x*x,axis=axis))
def db(a,b):return 20*np.log10(np.maximum(a,1e-16)/np.maximum(b,1e-16))
def wav(path):
    with wave.open(str(path))as w:
        assert w.getparams()[:4]==(2,2,44100,264600)
        raw=w.readframes(264600)
    return np.frombuffer(raw,dtype='<i2').reshape(-1,2).astype(float)/32768
def floats(f):
    q=np.fromfile(f,dtype='<f4').reshape(-1,2).astype(float)
    assert q.shape==(264600,2) and np.isfinite(q).all() and rms(q)>1e-6
    return q
def spec(a):
    a=np.lib.stride_tricks.sliding_window_view(a,WINDOW,axis=0)[::512]
    return np.sqrt(np.mean(np.abs(np.fft.rfft(a*np.hanning(WINDOW),axis=2))**2,axis=0))
def spec_dist(a,b):
    keep=np.fft.rfftfreq(WINDOW,1/SR)
    a,b=spec(a),spec(b)
    z=(keep>=20)&(keep<=16000)
    aa,bb=a[:,z],b[:,z]
    floor=np.max(aa,axis=1)[:,None]*1e-4
    return rms(db(np.maximum(bb,floor),np.maximum(aa,floor)),axis=1)
def margin(a,b):
    return dict(whole_dB=db(rms(b,axis=0),rms(a,axis=0)).round(3).tolist(),
                note_body_dB=round(float(db(rms(b[SR:198450]),rms(a[SR:198450]))),3),
                release_1p5s_dB=round(float(db(rms(b[198450:]),rms(a[198450:]))),3),
                last_1s_dB=round(float(db(rms(b[-SR:]),rms(a[-SR:]))),3),
                spectral_distance_db=spec_dist(a,b).round(3).tolist(),
                raw_residual_dB_rel_reference=round(float(db(rms(a-b),rms(a))),3))
def main():
    p=argparse.ArgumentParser()
    p.add_argument('references',type=Path)
    p.add_argument('fixed_calibration',type=Path)
    a=p.parse_args()
    base=Path(__file__).resolve().parent
    data=[]
    profile={row['program']:row['preview_gain'] for row in json.loads(a.fixed_calibration.read_text())}
    for i,g in GAIN_DB.items():
        original=wav(a.references/f'{i:03d}-original.wav')
        archived=wav(a.references/f'{i:03d}-historical-compact.wav')
        assert abs(20*np.log10(profile[i])-g)<=0.005
        g=profile[i]
        b=floats(base/'baseline'/f'{i}-wet.f32le')*g
        c=floats(base/'recovered'/f'{i}-wet.f32le')*g
        dry_a=floats(base/'baseline'/f'{i}-dry.f32le')
        dry_b=floats(base/'recovered'/f'{i}-dry.f32le')
        witness=db(rms(b-archived),rms(archived))
        assert witness<-70,(i,witness)
        data.append(dict(id=i,baseline_compact_repro_residual_db=round(float(witness),3),
                        original_clipped_words=int(np.sum(np.abs(original)>=32767/32768)),
                        reference_SHA256=hashlib.sha256((a.references/f'{i:03d}-original.wav').read_bytes()).hexdigest(),
                        older_vs_original=margin(original,b),
                        recovered_vs_original=margin(original,c),
                        changes_recovered_vs_baseline=dict(wet_residual_db=round(float(db(rms(c-b),rms(b))),3),
                            dry_residual_db=round(float(db(rms(dry_b-dry_a),rms(dry_a))),3))))
    print(json.dumps({'scope':'aligned 16-bit corrected oracle preview vs headless private diagnostic, not full firmware, dry reference unavailable and spectral window is diagnostic',
                    'pairs':data},indent=2))
if __name__=='__main__':main()
