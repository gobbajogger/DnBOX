from pathlib import Path
import hashlib,json,sys
import numpy as np
r=Path(sys.argv[1]); start,stop=25000,55000
t=2*np.pi*440*(np.arange(start,stop)-16384)/44100.
M=np.stack([g(k*t) for g in (np.sin,np.cos) for k in (1,2,3)]+[np.ones(len(t))],axis=1)
out=[]
for p in sorted(r.glob('*-response-v*')):
 m=json.loads((p/'capture.json').read_text())
 a=np.load(p/'output.npy',allow_pickle=False)
 assert hashlib.sha256(a.tobytes()).hexdigest()==m['pcm_sha256']
 assert m['forced_controls']['205']==0 and m['forced_controls']['3']==1
 y=a[:,start:stop].T.astype(float);c=np.linalg.lstsq(M,y,rcond=None)[0];u=c[3:6]-1j*c[:3]
 gain=20*np.log10(np.abs(u/u[:1]));phase=np.angle(u*np.exp(-1j*np.arange(1,4)[:,None]*np.angle(u[:1])),deg=True)
 out.append({'midi':m['command_velocity'],'repeat':m['repeat'],'rms':np.sqrt(np.mean(y*y,axis=0)).tolist(),'harmonic_db':gain.tolist(),'relative_phase':phase.tolist(),'sha':m['pcm_sha256']})
print(json.dumps(out,indent=2))
