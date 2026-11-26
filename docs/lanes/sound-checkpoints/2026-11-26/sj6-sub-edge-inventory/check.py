"""Find whether the signed stock-oscillator shape residual is localized at edges."""
from pathlib import Path
import sys,json,math
import numpy as np
from scipy.signal import butter,sosfiltfilt,hilbert
sys.path.insert(0,sys.argv[1])
from source_probe import read

def run(a,b):
 out={}
 for root,rate,raw,lo,hi in read(a,b):
  x=raw.astype('f8')/32768;f0=220*2**((root-69)/12)
  band=butter(4,[.7*f0,1.3*f0],fs=rate,btype='band',output='sos')
  h=hilbert(sosfiltfilt(band,x)); ph=(np.angle(h)/(2*np.pi))%1.;amp=np.abs(h)
  begin=lo+(hi-lo)//3;end=lo+(hi-lo)//2
  phase_idx=(ph*256).astype('i4')
  mass=np.bincount(phase_idx[begin:end],minlength=256,weights=amp[begin:end]**2)
  sig=np.bincount(phase_idx[begin:end],minlength=256,weights=(amp*x)[begin:end])
  assert min(mass)>0
  bins=sig/mass
  y=np.interp(ph*256,np.arange(257),np.r_[bins,bins[0]])*amp
  # Traditional oscillator-edge correction can move only a fixed high-
  # derivative arc; use the independent template, never residual-based masks.
  slope=np.abs(np.roll(bins,-1)-np.roll(bins,1))
  edges=slope>=np.quantile(slope,.85)
  at_edge=edges[phase_idx];rows={}
  for name,start,stop in [('strike',0,rate//20),('body',rate//10,rate//2),('pre_wrap',hi-256,hi),('post_wrap',lo,lo+256)]:
   mask=at_edge[start:stop]; ref=x[start:stop]; e=(y-x)[start:stop]; norm=np.linalg.norm(ref)
   # Even perfect correction restricted to edges cannot remove this floor.
   rows[name]={'scope_signed_dbr':float(20*np.log10(np.linalg.norm(e)/norm)),
      'residual_off_edge_pct':float(100*np.sum(e[~mask]**2)/np.sum(e**2)),
      'off_edge_signed_floor':float(20*np.log10(np.linalg.norm(e[~mask])/norm)) if np.any(~mask) else None,
      'edge_fraction':float(np.mean(mask))}
  out[root]=rows
 return out
if __name__=='__main__':print(json.dumps(run(*map(Path,sys.argv[2:])),indent=2,allow_nan=False))
