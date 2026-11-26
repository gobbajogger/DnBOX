import math,json
import numpy as np
from pathlib import Path
phase={45:-.857453,57:-1.717044,69:-3.438979,81:-6.899373,93:-13.917443,105:-28.402045}
anchors={0:(.34223624,[-30.668,-29.871,-48.268,-60.646,-57.089,-54.886],[-105.843,176.485,-.383,-69.856,-88.765,-24.852]),
         1:(.26098111,[-22.328,-19.137,-32.552,-28.067,-37.649,-33.955],[147.889,-6.538,121.672,-11.831,101.943,-16.988])}
r={}
for source in sorted(Path('.').glob('test-*')):
 if not source.is_dir():continue
 cases=[]
 for w in range(2):
  for note in phase:
   pcm=np.fromfile(source/f'test-{note}-{w}-0.pcm',dtype='<f4').astype('f8')
   f=440*2**((note-69)/12)
   ix=np.arange(4410,22050)
   x=pcm[ix]; periods=np.exp(-2j*math.pi*f*ix/44100)
   a=np.array([2*np.mean(x*periods**i) for i in range(1,8)])
   role=dict(wave=w,note=note,rms=float(np.sqrt(np.mean(x*x))),
             h1_amp=float(abs(a[0])),h1_deg=float(np.angle(a[0],deg=True)),
             h1_deg_error=float(np.angle(a[0]*np.exp(-1j*np.deg2rad(phase[note])),deg=True)),
             harmonics_dBc=(20*np.log10(abs(a[1:]/a[0]))).tolist(),
             relative_degrees=[float(np.angle(a[h]/(a[0]/abs(a[0]))**(h+1),deg=True)) for h in range(1,7)])
   # Measured anchors at A4 only; use as validation, never to generate tables.
   if note==69:
    m,levels,theta=anchors[w]
    rr=np.array([m*10**(d/20)*np.exp(1j*np.deg2rad(angle+(i+2)*phase[note])) for i,(d,angle) in enumerate(zip(levels,theta))])
    b=np.r_[m*np.exp(1j*np.deg2rad(phase[note])),rr]
    role['h1to7_rounded_signed_projection_residual_dBr']=float(20*np.log10(np.linalg.norm(a-b)/np.linalg.norm(b)))
   cases.append(role)
 r[str(source)]=cases
print(json.dumps(r,indent=2))
