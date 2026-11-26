import sys,json,hashlib,numpy as np
from pathlib import Path
A,T=map(Path,sys.argv[1:]);D=Path(__file__).resolve().parent
sys.path.insert(0,str(D.parents[1]/'s1-controlled-ab'));from measure import margin
expected=json.loads((D.parent/'fullrate-storage/result.json').read_text())['history_formats']['16']['original']
N=264728
data=np.fromfile(A/'original-wet.tap','<f4').reshape(N,10)
src=data[:,4:6]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def compare(x,y):
 rms=lambda a:np.sqrt(np.mean(a.astype('f8')**2,axis=0));db=lambda a:20*np.log10(np.maximum(a,1e-38))
 z={}
 for k,i,j in [('first_100ms',0,4410),('body',4410,198450),('release_front_100ms',198450,202860),('release_mid',202860,220500),('release_last_second',220500,264600)]:
  v,w=x[i+128:j+128],y[i+128:j+128]
  z[k]={'level_db':db(rms(w)/np.maximum(rms(v),1e-38)).tolist(),'rel_raw_db':db(rms(v-w)/np.maximum(rms(v),1e-38)).tolist()}
 return {'formal':margin(x[128:],y[128:]),'windows':z}
res={}
phase_hash=sha(A/'trajectory.raw')
for pair in ('64-64','16-16','16-12','16-8','12-8','8-8','h16-16','h16-12','h16-8','h16-6','14-4','13-4','12-5','11-6'):
 p=T/f'out-{pair}.raw';q=np.fromfile(p,'<f4').reshape(N,2)
 stats=json.loads((T/f'out-{pair}.stats').read_text())
 assert np.isfinite(q).all()
 if pair=='64-64':
  assert np.array_equal(src,q),(np.max(abs(src-q)),stats)
  assert not stats['double_roundtrip_mismatches']
 assert sha(T/f'phase-{pair}.raw')==phase_hash
 e=stats['band_energy'];stats['diff_vs_sum_energy_db']=float(10*np.log10(e[1]/e[0]))
 res[pair]={'stats':stats,'sha256':sha(p),'vs_source':compare(src,q)}
summary={'scope':'full-rate RevSC exact room input; reconstruct every tap independently; no alignment or side effects','frames_warm_included':N,'source_phase_sha256':phase_hash,'full_16_bit_stateless':{'history_bytes':expected['statistics']['history_bytes'],'metrics':expected['same_send_vs_full_float']},'pair_formats':res}
print(json.dumps(summary,indent=2,allow_nan=False))
