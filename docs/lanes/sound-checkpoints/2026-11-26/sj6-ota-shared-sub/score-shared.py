from pathlib import Path
import sys,numpy as np,tarfile,io,hashlib,json
gd,loc=map(Path,sys.argv[1:]); s=(gd/'score.py').read_text();exec(s[:s.rindex('print(json.dumps')])
with tarfile.open('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz') as tf:
 refs={}
 for q in ('dry','authored','authored-repeat'):
  p='program-2-'+q
  x=np.load(io.BytesIO(tf.extractfile('./'+p+'/output.npy').read()),allow_pickle=False)
  m=json.load(tf.extractfile('./'+p+'/capture.json'))
  assert hashlib.sha256(x.tobytes()).hexdigest()==m['pcm_sha256'] and m['active']==m['after']
  refs[q]=x
def X(k,p,st):return np.fromfile(loc/f's{k}-{p}.{st}','<f4').reshape(148684,2).T.astype(float)
def n(a,b):return float(db(np.linalg.norm(a-b)/np.linalg.norm(b)))
r={}
for k in (0,1):
 o={}
 for st in ('pre','final'):
  o[st]={}
  for w,(i,j) in W.items():
   if w in ('late_tail','whole','chord_attack'):continue
   a,b,c,d,e,f=(X(k,z,st)[:,i:j] for z in ['2-dry','a-full','sub-full','shared-a-half','a-half','sub-half'])
   o[st][w]=dict(all_independent_sum_dbr=n(a,b+c),half_main_independent_sum_dbr=n(d,e+c),sub_27_53_dbr=n(f,c*(27/53)))
 for q in ('dry','authored','authored-repeat'):
  o[q]=score(refs[q],X(k,'2-'+('authored' if q.startswith('authored') else q),'final'))
 r[k]=o
for g in ('0-dry','1-dry','a-zero'):
 for st in ('pre','final'):assert np.array_equal(X(0,g,st),X(1,g,st))
print(json.dumps(r,indent=2,allow_nan=False))
