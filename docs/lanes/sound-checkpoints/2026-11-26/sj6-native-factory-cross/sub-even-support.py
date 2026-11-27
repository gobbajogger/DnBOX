"""Measure true half-cycle residual with local interpolation, independent of phase bucket averaging."""
from pathlib import Path
import sys,json
import numpy as np
from scipy.interpolate import CubicSpline
sys.path.insert(0,str(Path(__file__).resolve().parent.parent/'sj6-sub-source-rate-distortion'))
from source_probe import read
a=Path(sys.argv[1])
N=256;out={}
for k,fs,raw,lo,hi in read(a/'Sounds/Osc/MySub.sfz',a/'J6P8R4N2S1'):
 if k not in (36,48,54,60,72,96):continue
 f=220*2**((k-69)/12)
 a0=int(.28*fs);a1=int(.72*fs)
 t=np.arange(a0,a1,dtype='f8')
 shift=.5*fs/f
 v=np.arange(a0-3,a1+int(shift)+5)
 # A deliberately sharp, but exactly symmetric, physically timed waveform.
 ctrl=sum(np.sin((2*np.pi*f*h/fs)*v)/h for h in range(1,34,2))
 y=raw[v].astype('f8')/32768
 phases=np.floor(np.mod(2*t*f/fs,1)*N).astype('int64')
 def shape(x):
  spl=CubicSpline(v,x); s=spl(t); d=spl(t+shift)
  o=.5*(s-d); e=.5*(s+d);e-=e.mean()
  h=np.bincount(phases,minlength=N)
  z=np.bincount(phases,weights=e*e,minlength=N)/h
  if not np.isfinite(z).all():raise ValueError("marker empty")
  n=max(1,round(N*.025))
  binsum=np.convolve(np.r_[z,z[:n-1]],np.ones(n),mode='valid')
  j=int(np.argmax(binsum))
  median_phase=float(np.median(np.mod(t[np.abs(e)>=np.quantile(np.abs(e),.999)]*2*f/fs,1)))
  return dict(relative_even_dbr=float(20*np.log10(np.linalg.norm(e)/np.linalg.norm(o))),
              best_2p5_fraction=float(binsum[j]/sum(z)),peak_interval_midpoint_phase=(j+.5*n)/N,
              median_high_energy_phase=median_phase)
 out[k]={'real':shape(y),'odd_control':shape(ctrl)}
print(json.dumps(out,indent=2,allow_nan=False))
