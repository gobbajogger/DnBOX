from pathlib import Path
import numpy as np,json,sys
t=Path(sys.argv[1]); out={'pins':(t/'pins').read_text().splitlines(),'cases':{}}
def rms(x):return np.sqrt(np.mean(x*x,axis=0))
def db(x):return 20*np.log10(np.maximum(x,1e-30))
def rel(a,b):
    v=db(np.divide(a,b,out=np.zeros_like(a),where=b>1e-24));return np.where(b>1e-24,v,None).tolist()
for k in ('pad','organ','live'):
    r,y=[np.fromfile(t/f'{k}-{a}.raw','<f4').reshape(-1,2).astype('float64') for a in ('full','age')]
    assert r.shape==y.shape==(264728,2) and np.isfinite(y).all()
    row=out['cases'][k]={'allocation':json.loads((t/f'{k}-alloc.json').read_text()),'ranges':{}}
    for w,a,b in [('first-echo',1800,6500),('held',22050,180000),('release',198578,220628),('last-second',220628,264728)]:
        rr,yy=r[a:b],y[a:b]
        f=np.fft.rfftfreq(b-a,1/44100)
        def eng(x):
            p=abs(np.fft.rfft(x*np.hanning(b-a)[:,None],axis=0))**2
            return np.stack([p[(f>=l)&(f<h)].sum(0) for l,h in [(80,340),(340,960),(960,3200),(3200,9000)]])
        row['ranges'][w]={'wet_level_dB':rel(rms(yy),rms(rr)),'wet_residual_dBr':rel(rms(yy-rr),rms(rr)),
          '80_340_960_3200_9000Hz_band_delta_dB':rel(np.sqrt(eng(yy)),np.sqrt(eng(rr)))}
print(json.dumps(out,indent=2,allow_nan=False))
