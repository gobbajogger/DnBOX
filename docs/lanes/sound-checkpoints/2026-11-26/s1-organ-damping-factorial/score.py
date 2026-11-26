from pathlib import Path
import numpy as np,json,hashlib,sys
d=Path(sys.argv[1]); repo=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(repo/'s1-controlled-ab'));from measure import margin
ids=('original-wet','compact-matched','compact-corrected')
ps=[d/(q+'.tap') for q in ids]
hashes=[hashlib.sha256(p.read_bytes()).hexdigest() for p in ps]
assert hashes[:2]==['09d3fcf1ed1c92bc441b3c1c37dcda796c99ec3b6b08b6e8769205cd1e079716',
                   '694c6c84cfbb3c881fd606efa05b88e000fc6cdff382ab3bc8e286cd19e0916a'],hashes
o,b,c=[np.memmap(p,dtype='<f4',mode='r',shape=(264728,10)) for p in ps]
assert all(p.stat().st_size==264728*40 for p in ps)
assert all(np.isfinite(x).all() for x in (o,b,c))
assert np.array_equal(o[:,2:4],b[:,2:4]) and np.array_equal(o[:,2:4],c[:,2:4])
assert np.array_equal(b[:,8:10],c[:,8:10])
def metric(r,y):
 r,y=[np.asarray(x[128:,4:6],dtype=np.float64) for x in (r,y)]
 def rms(x):return np.sqrt(np.mean(x**2,axis=0))
 windows={'onset_100ms':(0,4410),'held':(0,198450),'release_100ms':(198450,202860),
          'release':(198450,264600),'last_second':(220500,264600)}
 m=margin(r,y)
 m['windows']={k:{'level_db': (20*np.log10(np.maximum(rms(y[a:b]),1e-30)/np.maximum(rms(r[a:b]),1e-30))).tolist(),
                 'raw_residual_db':(20*np.log10(np.maximum(rms(r[a:b]-y[a:b]),1e-30)/np.maximum(rms(r[a:b]),1e-30))).tolist()}
    for k,(a,b) in windows.items()}
 return m
print(json.dumps(dict(fixture=113, scope='host only direct room-return columns 4/5; matched input, unchanged candidate controls',
  hashes=dict(zip(ids,hashes)),matched_source_send_all_frames=len(o),
  compact_feedback_mix_exact=True,
  compiled_baseline=metric(o,b),corrected_damping_only=metric(o,c)),indent=2))
