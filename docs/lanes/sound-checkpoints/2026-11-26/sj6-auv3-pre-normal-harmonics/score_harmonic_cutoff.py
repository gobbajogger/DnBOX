"""Compare installed A4 harmonic changes to original Normal filter transfer."""
from pathlib import Path
import ctypes,hashlib,json,sys
import numpy as np
root=Path(sys.argv[1]); dyl=ctypes.CDLL(sys.argv[2]); dyl.coeff.argtypes=[ctypes.c_double,ctypes.c_double,ctypes.POINTER(ctypes.c_double)]
J=lambda p:json.loads(p.read_text());assert J(root/'result.json')['state']=='pass'
points={'open':(1,20000),'c450':(.45,424),'c500':(.5,599),'c625':(.625,1967),'c750':(.75,4743)}
freq=np.arange(1,7)*440.
db=lambda x:20*np.log10(np.maximum(1e-15,np.abs(x)))
def prediction(hz):
 out=np.zeros(5);dyl.coeff(44100.,float(hz),out.ctypes.data_as(ctypes.POINTER(ctypes.c_double)))
 b0,b1,b2,a1,a2=out; z=np.exp(-2j*np.pi*freq/44100.)
 return (b0+b1*z+b2*z*z)/(1+a1*z+a2*z*z)
def relative_phase(c):
 return np.angle(c*np.exp(-1j*np.arange(1,7)[:,None]*np.angle(c[:1])),deg=True)
report={}
for shape in ('sine','tri'):
 prefix=f'native-jupiter-sine-gain-authored-harmonic-{shape}-'
 a={}
 for p in root.glob(prefix+'*'):
  m=J(p/'capture.json'); cut=m['ablation_arm'].split('-')[-1]; rep=bool(m['repeat'])
  assert m['forced_controls']['83']==float(np.float32(1/3 if shape=='tri' else 0))
  assert m['forced_controls']['7']==float(np.float32(points[cut][0])) and m['forced_controls']['131']==0
  assert m['forced_controls']['17']==0 and m['forced_controls']['123']==0
  v=np.load(p/'output.npy',allow_pickle=False)
  assert hashlib.sha256(v.tobytes()).hexdigest()==m['pcm_sha256']
  start,stop=25000,55000; t=2*np.pi*np.outer((np.arange(start,stop)-16384)/44100.,freq)
  matrix=np.concatenate([np.sin(t),np.cos(t),np.ones((len(t),1))],axis=1)
  coeff=np.linalg.lstsq(matrix,v[:,start:stop].T.astype(float),rcond=None)[0]
  a[cut,rep]=coeff[6:12]-1j*coeff[:6]
 assert len(a)==7
 own=a['open',False]; own_rel=db(own/own[:1]); pred0=prediction(20000)
 rows={}
 for cut in points:
  y=a[cut,False]; obs_delta=db(y/own); rel_delta=obs_delta-obs_delta[:1]
  r=prediction(points[cut][1])/pred0
  pr=db(r); pr_rel=pr-pr[0]
  phase_wrapped=lambda x:(x+180)%360-180
  obs_phase=phase_wrapped(relative_phase(y)-relative_phase(own))
  pr_phase=relative_phase(r[:,None])
  rep_error=None
  if (cut,True) in a:
   y2=a[cut,True]; rep_error={'max_relative_h1_to_h3_db':float(np.max(np.abs(db(y/y[:1])[:3]-db(y2/y2[:1])[:3]))),'fundamental_repeat_db':db(y2[0]/y[0]).tolist()}
  rows[cut]={'hz':points[cut][1],'fundamental_delta_db':obs_delta[0].tolist(),'predicted_h1_db':float(pr[0]),'relative_delta_db':rel_delta.tolist(),
             'predicted_relative_db':pr_rel.tolist(),'relative_error_db':(rel_delta-pr_rel[:,None]).tolist(),
             'relative_phase_change_deg':obs_phase.tolist(),'predicted_relative_phase_change_deg':pr_phase[:,0].tolist(),
             'relative_phase_error_deg':phase_wrapped(obs_phase-pr_phase).tolist(),'repeat':rep_error}
 report[shape]={'baseline_relative_db':own_rel.tolist(),'cuts':rows}
print(json.dumps(report,indent=2))
