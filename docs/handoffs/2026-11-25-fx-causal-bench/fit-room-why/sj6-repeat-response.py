import sys,numpy as np
sys.path.insert(0,'sj6-fx-bench')
import bench
from pathlib import Path
SR=44100
n=44100; N=2048
for f in (220,1000,3000,6000):
 t=np.arange(N); envelope=np.sin(np.pi*t/N)**2
 x=np.zeros((2,n),np.float32)
 x[0,:N]=.12*envelope*np.sin(2*np.pi*f*t/SR)
 y={}
 for nm,fn in [('reference',lambda e:bench.audited_delay_reference(x,e,Path('sj6-multirate').resolve())),('fitted',lambda e:bench.compact(Path('fit-s1-ab/sj6.so'),'delay',x,e))]:
  y[nm]=fn(True)-fn(False)
 def rms(z):return np.sqrt(np.mean(z*z))
 for k in (1,2):
  start=int(k*SR/4)
  a=y['reference'][0,start-32:start+N+32]
  b=y['fitted'][0,start-32:start+N+32]
  rr=rms(b)/rms(a)
  print(f,k,round(float(20*np.log10(rr)),3),round(float(np.dot(a,b)/np.linalg.norm(a)/np.linalg.norm(b)),4),bool(np.all(y['fitted'][1]==0)))
