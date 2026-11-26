import itertools,json,ctypes as c,numpy as np
from scipy.signal import sosfilt,butter
from refs import run,early,N,sr,p,f
m=c.CDLL('./micro.so');m.micro_new.restype=p;m.micro_size.restype=c.c_size_t;m.micro_drop.argtypes=[p];m.micro_run.argtypes=[p,f,f,f,c.c_uint];m.micro_config.argtypes=[c.c_float]*4
windows=[(int(sr*a),int(sr*b)) for a,b in ((.095,.19),(.19,.33),(.33,.55),(.55,.85),(.85,1.4))]
lows=butter(2,(160,900),btype='bandpass',fs=sr,output='sos');highs=butter(2,(1000,4000),btype='bandpass',fs=sr,output='sos')
def bands(y):
 return np.array([[np.sum(d[a:b]**2) for a,b in windows] for d in (sosfilt(lows,y,axis=0),sosfilt(highs,y,axis=0))])
def sim(v,fb,ch=0,at=0):
 m.micro_config(*v);h=m.micro_new();x=np.zeros((N,2),'f4');x[at,ch]=.25;y=np.empty_like(x)
 m.micro_run(h,x,y,np.full(N,fb,'f4'),N);m.micro_drop(h);return y
def db(x):return 10*np.log10(np.maximum(x,1e-20))
refs={fb:bands(run(fb)) for fb in (.6,.8,.96)}
scores=[]
for ex,sh,g in itertools.product((.7,1.1,1.6),(.4,.75,.98),(.15,.30,.6)):
 params=(ex,sh,g,1.)
 err=[db(bands(sim(params,fb)))-db(refs[fb]) for fb in refs]
 x=np.stack(err); mask=np.stack([r>max(r.flat)*1e-7 for r in refs.values()])
 scores.append((float(np.sqrt(np.mean(x[mask]**2))),params))
scores.sort();print(json.dumps(dict(size=m.micro_size(),best=scores[:5])))
open('chosen.json','w').write(json.dumps(scores[0][1]))
v=scores[0][1]
hold=[]
for fb in (.47,.72,.89,.985):
 for ch,at in ((0,0),(1,0),(0,4567),(1,7888)):
  a=run(fb,ch,at);b=sim(v,fb,ch,at)
  def metric(lo,hi):
   x,y=a[lo:hi],b[lo:hi];pa=np.mean(x*x,axis=0);pb=np.mean(y*y,axis=0)
   return dict(residual_dbr=db(np.mean((x-y)**2,axis=0))-db(pa),ref_rms_dbfs=db(pa),err_dbfs=db(np.mean((x-y)**2,axis=0)),level_delta=db(pb)-db(pa))
  rr=dict(fb=fb,ch=ch,at=at,exact_first=np.argmax((abs(a)>.0001).any(axis=1)).item(),candidate_first=np.argmax((abs(b)>.0001).any(axis=1)).item(),late=metric(at+int(.095*sr),at+int(.6*sr)),late2=metric(at+int(.6*sr),at+int(1.4*sr)))
  hold.append(rr)
def encode(a):
 if isinstance(a,np.ndarray):return a.tolist()
 raise TypeError
open('impulse-holdouts.json','w').write(json.dumps(hold,default=encode,indent=2))
