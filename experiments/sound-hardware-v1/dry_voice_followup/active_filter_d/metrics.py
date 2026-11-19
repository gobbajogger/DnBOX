from pathlib import Path
import numpy as np,json,sys,hashlib
f=Path(sys.argv[1]);a=np.fromfile(f,'<f4').reshape(3,88192,2).astype(float)
def rms(x): return float(np.sqrt(np.mean(x*x)))
def sp(x):
 w=np.lib.stride_tricks.sliding_window_view(x,2048)[::512]
 return np.sqrt(np.mean(abs(np.fft.rfft(w*np.hanning(2048),axis=-1))**2,axis=0))
g=float(sys.argv[2]) if len(sys.argv)>2 else rms(a[1,:,0])/rms(a[1,:,1]);rows=[]
for i,n in enumerate([36,60,84]):
 x=a[i,:,0];y=a[i,:,1]*g;s,t=sp(x),sp(y);floor=max(s.max()*1e-4,1e-12);mask=(s>=floor)|(t>=floor);d=20*np.log10(np.maximum(s[mask],floor)/np.maximum(t[mask],floor));rows.append(dict(note=n,rms_db=abs(20*np.log10(rms(y)/rms(x))),spectral_db=float(np.sqrt(np.mean(d*d)))))
r=dict(scope='dry controlled three-pitch screen, not full engine gate',gain=g,rows=rows,nonfinite=int((~np.isfinite(a)).sum()),pcm_sha256=hashlib.sha256(f.read_bytes()).hexdigest());f.with_suffix('.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r,indent=2))
