from pathlib import Path
import sys,hashlib,json
import numpy as np
assert len(sys.argv)==2
d=Path(sys.argv[1]);out={}
pins=['1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257','17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943']
target={'all':{},'osc1-only':{'3':0,'202':0},'osc2-only':{'1':0,'202':0},'sub-only':{'1':0,'3':0}}
d0={}
for repeat in (False,True):
 for arm,muted in target.items():
  root=d/('native-warm-brainwave-fanin-authored-'+arm+('-repeat' if repeat else ''))
  m=json.loads((root/'capture.json').read_text());x=np.load(root/'output.npy',allow_pickle=False)
  vals={a['address']:a['value'] for a in m['parameters']}
  assert [m['installed_au_sha256'],m['s4_input_sha256']]==pins
  assert m['rate']==44100 and m['warm_frames']==16384 and m['frames']==148684 and m['repeat'] is repeat
  assert m['ablation_arm']==arm and m['master_mode']=='authored'
  assert x.shape==(2,148684) and x.dtype==np.float32 and np.isfinite(x).all() and not x[:,:16384].any()
  assert hashlib.sha256(x.tobytes()).hexdigest()==m['pcm_sha256']
  assert m['parameters']==m['parameter_readback_after']
  assert all(vals[str(k)]==0 for k in (11,16,17,22,23,24,28,40,41,59,103,107,108,109,201))
  if not d0:d0=dict(vals=vals,schedule=m['schedule'],buffers=m['buffers'])
  assert m['schedule']==d0['schedule'] and m['buffers']==d0['buffers']
  assert vals==d0['vals']|muted
  out[(arm,repeat)]=x
def norm(a):return np.linalg.norm(a.astype(np.float64))
def pair(x,r):
 db=lambda a,b:20*np.log10(max(norm(a),1e-30)/max(norm(b),1e-30))
 return {'level_db':db(x,r),'null_dbr':db(x-r,r)}
r=out['all',False];x=sum(out[a,False].astype(np.float64) for a in target if a!='all')
res={}
for n,i,j in [('held',24576,37634),('pending',49459,49875),('commit',49875,50003),('chord',53971,81434),('release',82534,115302)]:
 res[n]={'unweighted_sum_vs_all':pair(x[:,i:j],r[:,i:j]),'native_repeats':{a:pair(out[a,True][:,i:j],out[a,False][:,i:j]) for a in target}}
print(json.dumps(dict(scope='Terminal warm only, dry control closures. Summability cannot establish absolute gain, failure needs state/input-dependent diagnostic before changing module.',windows=res),indent=2))
