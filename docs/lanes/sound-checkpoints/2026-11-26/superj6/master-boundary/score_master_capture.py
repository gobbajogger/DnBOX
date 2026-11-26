"""Score held known public master interventions, without fitting."""
import json,hashlib,sys
from pathlib import Path
import numpy as np
if len(sys.argv)!=2:
 raise SystemExit('score_master_capture.py /private/newly-captured-dir')
base=Path(sys.argv[1])
digest=lambda a:hashlib.sha256(a.tobytes()).hexdigest()
pinned='1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
cue='17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943'
windows=dict(attack=(16384,18432),held=(24576,37634),chord=(53971,81434),release=(82534,115302))
def capture(op,mode,rep):
 d=base/(f'native-warm-brainwave-master-{mode}-{op}-only'+('-repeat' if rep else ''))
 m=json.loads((d/'capture.json').read_text()); x=np.load(d/'output.npy',allow_pickle=False)
 assert m['installed_au_sha256']==pinned and m['s4_input_sha256']==cue
 assert x.dtype==np.float32 and x.shape==(2,148684) and np.isfinite(x).all() and digest(x)==m['pcm_sha256']
 assert m['rate']==44100 and m['parameters']==m['parameter_readback_after'] and not x[:,:16384].any()
 assert m['master_mode']==mode and m['ablation_arm']==op+'-only' and m['repeat'] is rep
 return x,m
def metric(x,z):
 ans={}
 for label,(i,j) in windows.items():
  a,b=[v[:,i:j].astype(np.float64) for v in (x,z)]
  norm=np.linalg.norm
  def db(n,d):return float(20*np.log10(max(norm(n),1e-30)/max(norm(d),1e-30)))
  ans[label]=dict(level_db=db(a,b),null_dbr=db(a-b,b))
 return ans
res={}
for op in ('main','sub'):
 ref,r=capture(op,'authored',False)
 vals=lambda m:{str(p['address']):p['value'] for p in m['parameters']}
 arms={}; origin=vals(r)
 for mode,f in [('authored',1.),('half',.5),('quarter',.25)]:
  x,m=capture(op,mode,False);y,n=capture(op,mode,True)
  assert all(v['schedule']==r['schedule'] and v['buffers']==r['buffers'] for v in (m,n))
  assert vals(m)==vals(n)
  assert {a for a in origin if origin[a]!=vals(m)[a]}==({'0'} if f!=1 else set())
  assert vals(m)['0']==float(np.float32(origin['0'])*np.float32(f))==m['actual_master']
  arms[mode]=dict(master=vals(m)['0'],capture=m['pcm_sha256'],repeat_capture=n['pcm_sha256'],repeat_fidelity=metric(y,x),known_ratio_only=metric(x,ref*np.float32(f)))
 res[op]=arms
print(json.dumps({'scope':'Public AU relative master law; this cannot identify absolute gain or tap before master.', 'operators':res},indent=2))
