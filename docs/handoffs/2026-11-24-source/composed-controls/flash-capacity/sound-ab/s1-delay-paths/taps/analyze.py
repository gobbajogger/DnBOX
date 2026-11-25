from pathlib import Path
import numpy as np
d=Path(__file__).parent; sr=44100
def data(m,v,c):
 return np.fromfile(d/f'{m}-{v}.f32le',dtype='<f4').reshape(-1,2)[-sr:,c].astype(np.float64)
def db(a,b):return 20*np.log10(max(a,1e-16)/max(b,1e-16))
for name,m,c in [('left 2T',0,0),('right direct T',1,1),('right cascade 2T+T',2,1),('right combined',0,1)]:
 a=data(m,'full',c);b=data(m,'compact',c)
 spec=lambda a: abs(np.fft.rfft(a*np.hanning(sr)))
 af=spec(a);bf=spec(b)
 print(name,'full RMS',round(np.std(a),7),'RMS error',round(db(np.linalg.norm(b),np.linalg.norm(a)),2),'null',round(db(np.linalg.norm(a-b),np.linalg.norm(a)),2), 'bands',[round(float(db(np.linalg.norm(bf[l:h]),np.linalg.norm(af[l:h]))),2) for l,h in [(20,500),(500,1000),(1000,2000),(2000,4000),(4000,16000)]])
