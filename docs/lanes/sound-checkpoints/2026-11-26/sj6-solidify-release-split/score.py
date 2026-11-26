"""Compare fixed time signed source tail and repeat floor; no time/phase fits."""
from pathlib import Path
import hashlib,tarfile,io,sys,json,numpy as np
o,b=map(Path,sys.argv[1:])
a=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
def sha(x):return hashlib.sha256(x).hexdigest()
assert sha(a.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
all={}
pins={}
with tarfile.open(a) as t:
 for arm in ('authored','dry','authored-repeat'):
  p=f'./program-1-{arm}'
  d=json.load(t.extractfile(p+'/capture.json'))
  x=np.load(io.BytesIO(t.extractfile(p+'/output.npy').read()),allow_pickle=False)
  assert d['active']==d['after'] and x.shape==(2,148684) and sha(x.tobytes())==d['pcm_sha256']
  assert d['warm_frames']==16384 and d['rate']==44100
  if arm=='authored':sched=d['schedule']; cue=d['cue_sha256']
  else:assert d['schedule']==sched and d['cue_sha256']==cue
  pins[arm]=d['pcm_sha256']
  all[arm]=x.T.copy()
for phase in (0,2):
 for arm in ('authored','dry'):
  k=f'1-{phase}-1-{arm}';x=(o/f'{k}.f32').read_bytes()
  all[f'{phase}-{arm}']=np.frombuffer(x,'<f4').reshape(-1,2)
  frozen=json.load(open('sj6-solidify-sine-phase/result.json'))[f'1-{arm}']['variants']['1:2']
  assert sha(x)==frozen['base_pcm_sha' if phase==0 else 'candidate_pcm_sha']
  pins[f'{phase}-{arm}']=sha(x)
def rms(x): return float(np.sqrt(np.mean(x*x,dtype='f8')))
def dba(x):
 z=rms(x)
 return float(20*np.log10(z)) if z>0 else None
def compare(x,y):
    x,y=(a.astype('f8') for a in (x,y))
    xx,yy=rms(x),rms(y)
    return dict(levels_dbfs=[dba(x),dba(y)], signed_residual_dbfs=dba(x-y),
        signed_residual_dbr=float(20*np.log10(rms(x-y)/yy)) if yy and rms(x-y) else None,
        signed_corr=float(np.vdot(x,y)/(np.linalg.norm(x)*np.linalg.norm(y))) if xx and yy else None)
out={}
for release,at in [('single',38434),('final',82534)]:
 out[release]={}
 for lo,hi in [(-16,0),(0,2),(2,4),(4,8),(8,12),(12,16),(16,25),(25,50),(50,100),(100,200),(0,16),(16,50)]:
   i,j=[at+round(k*44.1) for k in (lo,hi)]
   v={k:a[i:j] for k,a in all.items()}
   results={f'{x}_vs_{y}':compare(v[x],v[y]) for x,y in [('authored-repeat','authored'),('0-authored','authored'),('2-authored','authored'),('2-authored','authored-repeat'),('0-dry','dry'),('2-dry','dry'),('2-dry','0-dry')]}
   out[release][f'{lo}:{hi}']=results
stage={}
for mode in (0,2):
 for arm in ('authored','dry'):
  stem=o/f'1-{mode}-1-{arm}'
  x,y=(np.fromfile(f'{stem}-stage{i}.f32','<f4').reshape(-1,2) for i in (0,1))
  end=all[f'{mode}-{arm}']
  meter=(str(stem)+'.txt')
  gain=np.float32(float.fromhex(Path(meter).read_text().splitlines()[-1].split()[1]))
  assert x.shape==y.shape==end.shape==(148684,2)
  assert np.array_equal(y*gain,end)
  assert np.array_equal(x,y)==(arm=='dry') 
  r={}
  for off in (38434,82534):
   live=np.flatnonzero(np.any(x[off:off+1500]!=0,axis=1))
   assert len(live)>0
   r[str(off)]={'last_actual_pre_fx_input_delta_frames':int(live[-1]),'after16ms_terminal_exact_zero':bool(not end[off+706:off+1500].any())}
  stage[f'{mode}-{arm}']={'dry_path_is_master_only_bit_exact':bool(np.array_equal(x,y)), 'releases':r}
# A fit to known poles is diagnostic only. Train modal coordinates on the same
# fixed interval, never align or tune the pole(s), score disjoint actual output.
rel=np.arange(8115)
a=float.fromhex('0x1.10427p-19'); b=float.fromhex('0x1.ff4564p-1')
z=next(x for x in np.roots([1,-(1+b-a),b]) if x.imag>0)
basis=np.column_stack([(z**rel).real,(z**rel).imag,float.fromhex('0x1.fd70a4p-1')**rel,float(np.float32(.999))**rel])
end_train,off=1499,705
modal={}
for arm in ('dry','authored','authored-repeat'):
 modal[arm]={}
 for event in (38434,82534):
   ys=all[arm][event+off:event+off+len(rel)].astype('f8')
   coeff=np.linalg.lstsq(basis[:end_train],ys[:end_train],rcond=None)[0]
   ys_hat=basis@coeff
   modal[arm][str(event)]={'actual_unforced_modes_holdout':compare(ys_hat[end_train:],ys[end_train:])}
# Nonqualifying sensitivity only: Solidify exact zero-Crusher guard is
# false (#108 isn't zero). Ask a physical question using cloned state
# at gate-off, not a proposed dry profile admission or zeroing state.
from scipy.signal import lfilter
a0=float.fromhex('0x1.ffa2ap-1')
def hypothesis(x):
    hp=lfilter([a0,-2*a0,a0],[1,-(1+b-a),b],x,axis=0)
    cr=lfilter([1,-1],[1,-float.fromhex('0x1.fd70a4p-1')],hp,axis=0)
    return lfilter([1,-1],[1,-float(np.float32(.999))],cr,axis=0)
sensitivity={}
for mode in (0,2):
 x=all[f'{mode}-dry'].astype('f8')
 for name,at in (('single',38434),('final',82534)):
    output=hypothesis(x)
    zero=x.copy();zero[at:]=0
    inherited=hypothesis(zero) # clone with identical previous input/history
    nohist=x.copy();nohist[:at]=0
    driven=hypothesis(nohist)
    for lo,hi in ((0,16),(16,50)):
        i,j=(at+round(ms*44.1) for ms in (lo,hi))
        assert rms((output-inherited-driven)[i:j])<1e-11
        y=all['dry'][i:j]
        sensitivity[f'{mode}:{name}:{lo}-{hi}']={n:compare(q[i:j],y) for n,q in (
            ('combined',output),('inherited',inherited),('driven',driven))}
print(json.dumps(dict(inputs=pins,cues=sched,windows=out,stage=stage,modal=modal,nonadmitting_counterfactual=sensitivity),indent=2,allow_nan=False))
