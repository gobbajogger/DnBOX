from pathlib import Path
import ctypes as c, hashlib, io, json, math, re, sys, tarfile, zipfile,struct
import numpy as np
o,b=map(Path,sys.argv[1:])
# Reuse authorized matching sample capsule decoder, no compiled app import.
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('ipa=')[1].split('nat=Path(')[0]
exec(compile('ipa='+s,'existing private SFZ container decoder','exec'))
sha=lambda z:hashlib.sha256(z).hexdigest()
assert sha(data)=='ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740'
assert sha(sfz)=='3668ef83c102b4952a19bc6fd5adc585eb69d0eff4dd4743be9488b0771fc75d'
arc=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert sha(arc.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(arc) as t:
 def take(a):
  p=f'./program-2-{a}/'
  m=json.load(t.extractfile(p+'capture.json'))
  r=np.load(io.BytesIO(t.extractfile(p+'output.npy').read()),allow_pickle=False).copy()
  assert sha(r.tobytes())==m['pcm_sha256'] and m['active']==m['after'] and m['frames']==r.shape[1]==148684
  return r,m
 refs=dict((a,take(a)) for a in ('authored','dry','authored-repeat'))
assert np.array_equal(refs['authored'][0],refs['authored-repeat'][0])
from dose import collect
m=refs['dry'][1];raw={int(v['address']):v['value'] for v in m['active']}
def run(pack):
 a,pin=collect(m,raw,regions,pack,sub_only=True)
 x,_,events=a['Sub']
 assert {str(k):v for k,v in events.items()}==json.load(open('sj6-warm-raw-postvoice/result-final.json'))['source_roles']['Sub']['events']
 return x,pin
old,pin=run(pack)
assert sha(old.tobytes())==json.load(open('sj6-warm-raw-postvoice/result-final.json'))['source_roles']['Sub']['output_sha']
def score(x,y):
 x,y=np.asarray(x).astype('f8'),np.asarray(y).astype('f8');n=np.linalg.norm
 return [float(20*np.log10(n(x-y)/n(y))),float(20*np.log10(n(x)/n(y))),float(np.vdot(x,y)/n(x)/n(y))]
def make(cycles):
 p={};met={}
 for g in regions:
  name=g['sample'].replace('\\','/').split('/')[-1]
  sr,med=pack[name]
  lo,hi,root=[int(g[k]) for k in ('loop_start','loop_end','pitch_keycenter')]
  period0=sr/(220*2**((root-69)/12))
  center=lo+int(.65*(hi-lo));guard=round(10*period0)
  a,b=center-4*guard,center-2*guard
  u=med[a:b].astype('f8')
  def er(k):return float(np.mean((u-med[a+k:b+k])**2))
  lag=min(range(max(1,int(period0*.97)),int(period0*1.03)+2),key=er)
  ym,yy,yp=(er(k) for k in (lag-1,lag,lag+1))
  assert 2*yy<ym+yp
  period=lag+.5*(ym-yp)/(ym-2*yy+yp)
  L=cycles*period;n=int(np.ceil(L));samples=np.arange(len(med))
  carrier=np.interp(center+np.arange(n)*L/n,samples,med)
  assert center+L<hi and center-4*guard>lo
  t=(samples-center)%L*n/L
  tiled=np.clip(np.rint(np.interp(t,np.arange(n+1),np.append(carrier,carrier[0]))),-32768,32767).astype('i2')
  p[name]=(sr,tiled)
  w={'strike-0-50ms':(0,sr//20),'early-body-100-500ms':(sr//10,sr//2),
       'blind-loop-head':(lo+guard,lo+2*guard),'blind-loop-tail':(hi-2*guard,hi-guard),
       'actual-loop-boundary-before':(hi-256,hi),'actual-loop-boundary-after':(lo,lo+256)}
  met[root]=dict(period_frames=period,nominal_period_frames=period0,raw_table_points=n, blind={k:score(tiled[i:j],med[i:j]) for k,(i,j) in w.items()})
 return p,met
rep={}
for cycles in (1,4):
 synthetic,metrics=make(cycles)
 new,p2=run(synthetic);assert p2==pin
 windows={'onset':(16384,18589),'first_hold':(20794,38434),'first_release':(38434,39094),'chord':(53869,82534),'final_release':(82534,83194),'full':(16384,148684)}
 rep[cycles]=dict(raw_source_zone_metrics=metrics,diagnostic_PCM16_B=2*sum(z['raw_table_points'] for z in metrics.values()),
 actual_current_sub_source_replay={k:score(new[:,i:j],old[:,i:j]) for k,(i,j) in windows.items()})
print(json.dumps(dict(source_pin=pin,media_in_memory_only=True,columns=['signed_dbr','level_db','signed_correlation'],variations=rep,
 scope='real current Sub isolated receiver exact Raw MIDI, cutoff, AMP and sample owner untouched, late blind source-loop slices; private reference table not deployed or exported'),indent=2,allow_nan=False))
