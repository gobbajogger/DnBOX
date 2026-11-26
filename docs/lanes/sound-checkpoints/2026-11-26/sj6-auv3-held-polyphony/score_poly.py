"""Score steady-state held-note gain with same-clock AUv3 solo and poly arms."""
import hashlib,json,sys
from pathlib import Path
import numpy as np

root=Path(sys.argv[1]); result=json.loads((root/'result.json').read_text())
assert result['state']=='pass'
arms=('held','additions','all','held-repeat','all-repeat')
y={};meta={}
for arm in arms:
 p=root/f'native-jupiter-sine-gain-authored-poly-{arm}'
 m=json.loads((p/'capture.json').read_text())
 a=np.load(p/'output.npy',allow_pickle=False)
 assert a.dtype==np.dtype('<f4') and a.shape==(2,m['frames']) and np.isfinite(a).all()
 assert hashlib.sha256(a.tobytes()).hexdigest()==m['pcm_sha256']
 assert m['component']=='aumu/j6ct/AuPr' and m['component_version']==67072 and m['rate']==44100
 assert m['forced_controls']['32']==m['forced_controls']['33']==0
 assert m['calibration_cue']=='jupiter-poly-held-v1' and len(m['schedule'])==12
 a4=[e for e in m['schedule'] if e['note']==57]
 assert len(a4)==2 and all(e['send']==(arm!='additions') for e in a4)
 y[arm]=a.astype(float);meta[arm]=m
m=meta['held']
for mm in meta.values():
 assert [tuple(e[k] for k in ('frame','note','action','velocity')) for e in mm['schedule']]==[tuple(e[k] for k in ('frame','note','action','velocity')) for e in m['schedule']]
 assert mm['buffers']==m['buffers'] and mm['parameters']==m['parameters']
keys=(57,45,55,60,73,82)
rate=m['rate']; frequencies=440*2**((np.array(keys)-69)/12)
windows=[('one_in',22528,28160),('three_in',35328,40448),('six_in',48384,55808),
         ('three_out',62976,68608),('one_out',75776,81408)]
def db(x):
 return float(20*np.log10(max(1e-15,x)))
def norm(x): return np.linalg.norm(x)
rows={}
for name,lo,hi in windows:
 t=2*np.pi*np.arange(lo,hi)/rate; w=np.hanning(hi-lo)
 theta=t[:,None]*frequencies
 cols=np.concatenate([np.sin(theta),np.cos(theta),np.ones((len(t),1))],axis=1)
 coeff={k:np.linalg.lstsq(cols*w[:,None],a[:,lo:hi].T*w[:,None],rcond=None)[0] for k,a in y.items()}
 level={k:np.hypot(v[:len(keys)],v[len(keys):2*len(keys)]) for k,v in coeff.items()}
 rows[name]={
   'held_amplitude':{k:val[0].tolist() for k,val in level.items()},
   'held_amplitude_db_poly_vs_solo':[db(a/b) for a,b in zip(level['all'][0],level['held'][0])],
   'held_repeat_amplitude_db_poly_vs_solo':[db(a/b) for a,b in zip(level['all-repeat'][0],level['held-repeat'][0])],
   'tones_amplitude':{k:val.tolist() for k,val in level.items()},
   'sample_peak':{k:np.max(np.abs(a[:,lo:hi]),axis=1).tolist() for k,a in y.items()},
   'signed_sum_residual_dBr':[db(norm(a[:,lo:hi]-y['held'][:,lo:hi]-y['additions'][:,lo:hi])/norm(a[:,lo:hi])) for a in (y['all'],y['all-repeat'])],
   'signed_repeat_residual_dBr':{k:db(norm(y[k][:,lo:hi]-y[k+'-repeat'][:,lo:hi])/norm(y[k][:,lo:hi])) for k in ('held','all')}
 }
print(json.dumps({'source_sha256':m['installed_au_sha256'],'keys':keys,'sample_rate':rate,
       'window_edges_frames':windows,'all_cue_sample_peak':{k:np.max(np.abs(a),axis=1).tolist() for k,a in y.items()},'windows':rows},indent=2))
