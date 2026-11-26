import hashlib,json,math,sys
from pathlib import Path
import numpy as np
R=Path(sys.argv[1]); J=lambda p:json.loads(p.read_text())
assert J(R/'result.json')['state']=='pass'
def db(x):
 return 20*np.log10(np.maximum(1e-15,x))
def load(p,digest):
 y=np.load(p,allow_pickle=False)
 assert y.shape==(2,148684) and y.dtype==np.dtype('<f4') and np.isfinite(y).all()
 assert hashlib.sha256(y.tobytes()).hexdigest()==digest
 return y.astype(float)
rate=44100; on=16384; off=60484
windows=((on+11025,on+33075),(on+1323,on+5733),(on+22050,on+35280))
def score(y,w):
 a,b=w; v=y[:,a:b]; t=2*np.pi*440*np.arange(a,b)/rate
 g=np.stack([np.sin(k*t) for k in (1,2,3,4)]+[np.cos(k*t) for k in (1,2,3,4)]+[np.ones(len(t))],axis=1)
 e=np.linalg.lstsq(g,v.T,rcond=None)[0]; h=np.hypot(e[:4],e[4:8])
 return {'rms':np.sqrt(np.mean(v*v,axis=1)), 'h1':h[0], 'harmonic_db':db(np.max(h[1:],axis=0)/h[0]),'peak':np.max(np.abs(v),axis=1)}
cal=J(R/'bare.json'); assert cal['component']=='aufx/dcmp/appl' and cal['on']==on and cal['off']==off
bare=[]; check={}
for a in cal['arms']:
 assert len(a['params'])==7
 x=load(R/f"bare-{a['id']}.npy",a['pcm_sha256'])
 bare.append((a,x))
 keys=tuple(sorted(a['params'].items()))
 assert not check or keys==check['apple']
 check['apple']=keys
job={}
for p in R.glob('native-jupiter-sine-gain-authored-compressor-*'):
 m=J(p/'capture.json'); assert m['calibration_cue']=='jupiter-compressor-A4-v1' and m['component_version']==67072
 state='on' if m['ablation_arm'].endswith('-on') else 'off'
 assert m['forced_controls']['123']==0 and m['forced_controls']['17']==(float(np.float32(.02)) if state=='on' else 0)
 assert m['authored_master']==m['actual_master'] and m['rate']==rate
 assert [(a['frame'],a['note']) for a in m['schedule']]==[(on,69),(off,69)]
 assert m['buffers']==bare[0][0]['buffers']
 y=load(p/'output.npy',m['pcm_sha256'])
 f=float(m['osc2_fader']); k=(f,state,bool(m['repeat']))
 assert k not in job
 job[k]=[score(y,w) for w in windows]
faders=[.0625,.125,.25,.375,.5,.75,1.]
training=[1,2,4,6];holdout=[0,3,5]
expected=np.zeros((2,148684)); t=np.arange(off-on)/rate
expected[:,on:off]=np.float32(np.minimum(1,t/.002)*np.sin(2*np.pi*440*t))
for a,x in bare:
 if a['bypass']:
  y=expected*a['peak_in']; check['bare_bypass_max_abs_error']=float(np.max(np.abs(x-y)))
  assert check['bare_bypass_max_abs_error']<4e-7
seed=np.geomspace(.008,.90,8192); report={'source':check,'faders':faders,'analysis':{}}
passing=np.ones(len(seed),bool)
for key in ('rms','h1'):
 for ch in (0,1):
  for wi,w in enumerate(windows):
   levels={}
   for a,x in bare:
    if a['bypass']:continue
    peak=a['peak_in']
    val=score(x,w)[key][ch]
    ref=score(expected*peak,w)[key][ch]
    levels.setdefault(peak,[]).append(float(db(val/ref)))
   points=np.array(sorted(levels)); curve=np.array([np.median(levels[f]) for f in points])
   bRepeat=float(max(np.ptp(v) for v in levels.values()))
   offLevel=np.array([job[(f,'off',False)][wi][key][ch] for f in faders])
   native=np.array([float(db(job[(f,'on',False)][wi][key][ch]/job[(f,'off',False)][wi][key][ch])) for f in faders])
   rel=offLevel/offLevel[-1]
   nRepeat=max(abs(native[faders.index(f)]-float(db(job[(f,'on',True)][wi][key][ch]/job[(f,'off',True)][wi][key][ch]))) for f in (.25,.5))
   ep=max(.35,bRepeat+nRepeat+.10)
   xx=seed[:,None]*rel
   valid=(xx>=points[0])&(xx<=points[-1])
   pred=np.interp(np.log(xx),np.log(points),curve)
   residual=np.abs(pred-native)
   good=valid.all(1)&(residual[:,training]<=ep).all(1)&(residual[:,holdout]<=ep).all(1)
   low=float(np.median(curve[points<=.04]))
   discriminative=bool(((low-native)>=1.5).sum()>=2)
   hypo=np.interp(np.log(np.array([.15,.6])[:,None]*rel),np.log(points),curve)
   discriminative=discriminative and bool((np.abs(np.diff(hypo,axis=0))[0]>=2*ep).sum()>=2)
   if wi==0:
    passing &= good & discriminative & (bRepeat<=.10)
   if wi==2:
    passing &= good
   listg=seed[good].tolist()
   report['analysis'][f'{key}_{ch}_{wi}']={'bare_amp':points.tolist(),'bare_response_db':curve.tolist(),'bare_repeat_db':bRepeat,
       'instrument_off':offLevel.tolist(),'instrument_response_db':native.tolist(),'repeat_db':float(nRepeat),'epsilon_db':float(ep),
       'inlet_interval':([listg[0],listg[-1]] if listg else []),'best_max_resid_db':float(np.min(np.max(np.where(valid,residual,100),axis=1))),
       'discriminative_knee':discriminative,'off_harmonic_db_at_full':float(job[(1.,'off',False)][wi]['harmonic_db'][ch])}
ok=seed[passing];interval=[] if not len(ok) else [float(ok[0]),float(ok[-1])]
report['admissible_interval']=interval
report['identifiable']=bool(len(ok) and db(ok[-1]/ok[0])<=3)
print(json.dumps(report,indent=2))
