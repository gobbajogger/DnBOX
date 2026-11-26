"""Validate a finite difference of an implied source, not an ill-conditioned inverse."""
import hashlib,json,sys
from pathlib import Path
import numpy as np
from scipy.signal import lfilter
v,c=map(Path,sys.argv[1:]);manifest=json.loads((c/'manifest.json').read_text())
mo=json.loads((v/'capture.json').read_text()); orig=np.load(v/'output.npy',allow_pickle=False)
assert hashlib.sha256(orig.tobytes()).hexdigest()==mo['pcm_sha256']
assert mo['active']==mo['after'] and mo['rate']==44100
a,b,t=[manifest[k] for k in ('abs_begin_frame','abs_end_excluded_frame','noteoff_abs_frame')]
assert t==mo['schedule'][1]['frame']==38434
def pcm(n):
 x=(c/n).read_bytes()
 assert hashlib.sha256(x).hexdigest()==manifest['PCM_SHA256'][n]
 return np.frombuffer(x,'<f4').reshape(-1,2).T.astype('f8')
x=pcm('center-only.f32'); y=pcm('center-plus-hp-dc.f32')
orig=orig[:,a:b].astype('f8')
hp0,hp1,hp2=map(float.fromhex,['0x1.ffa2ap-1','0x1.10427p-19','0x1.ff4564p-1'])
crush=float.fromhex('0x1.fd70a4p-1')
den=np.convolve([1.,-1-hp2+hp1,hp2],[1.,-crush])
den=np.convolve(den,[1.,-.999])
def score(x,y):
 q=np.linalg.norm(y)
 return [float(20*np.log10(np.linalg.norm(x-y)/q)), float(20*np.log10(np.linalg.norm(x)/q))]
rows={}
for stride in [1,4,16,32,64,128]:
 n=np.zeros(stride*4+1)
 n[::stride]=np.array([1.,-4.,6.,-4.,1.])
 e=lfilter(n,[1.],x,axis=1)
 def project(z):
  z=lfilter(den,[1.],z,axis=1)
  for _ in range(4):
   z=lfilter(np.ones(stride),[1.],z,axis=1)
  return z/hp0
 f=project(y);u=project(orig)
 lo=t-a
 windows={'hold':(lo-2048,lo),'early_release':(lo,lo+560),
           'late_release':(lo+560,lo+2048),'whole':(4*stride+5,len(x[0]))}
 rows[stride]={name:{'validate_same_input_compact':score(f[:,l:h],e[:,l:h]),
                     'compact_vs_implied_original':score(e[:,l:h],u[:,l:h])}
                for name,(l,h) in windows.items() if l>=4*stride+5}
print(json.dumps({'scope':'original dry fixed linear suffix conjecture must first pass matched compact check; no phase/gain fit', 'columns':['signed_dbr','level_db'],'results':rows},indent=2))
