import hashlib,json,numpy as np
from pathlib import Path
root=Path(__file__).parent
r=44100
band=((20,500),(500,1000),(1000,2000),(2000,4000),(4000,16000))
def db(a,b):
 return float(20*np.log10(max(a,1e-12)/max(b,1e-12)))
def fft(p):
 a=np.fromfile(p,'<f4').reshape(-1,2)[-r:]
 return np.abs(np.fft.rfft(np.hanning(r)[:,None]*a,axis=0))
for h in ('stock','2','4'):
 rows=json.loads((root/f'{h}.json').read_text())['pairs'];print('WINDOW',h)
 for row in rows:
  k=row['id'];a=row['recovered_vs_original']
  p=lambda m,n: root/m/f'{k}-{n}.f32le'
  ident=lambda n:hashlib.sha256(p('stock',n).read_bytes()).digest()==hashlib.sha256(p(h,n).read_bytes()).digest()
  print(k,a['last_1s_dB'],a['spectral_distance_db'],a['raw_residual_dB_rel_reference'],'unchanged-dry',ident('dry'))
  if k==62:
   y=fft(p(h,'iso-wet')); x=fft(p('stock','iso-wet'))
   print('Pad delay-only band change R', [round(db(np.linalg.norm(y[l:u,1]),np.linalg.norm(x[l:u,1])),3) for l,u in band])
  if k in (62,128,278):
   c=np.fromfile(p(h,'iso-wet'),'<f4').reshape(-1,2)[-r:]
   d=np.fromfile(p('stock','iso-wet'),'<f4').reshape(-1,2)[-r:]
   print('Delay-only final-second L/R delta', [round(db(np.linalg.norm(c[:,j]),np.linalg.norm(d[:,j])),3) for j in range(2)])
