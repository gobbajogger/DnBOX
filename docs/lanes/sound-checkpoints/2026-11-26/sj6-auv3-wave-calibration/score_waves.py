import hashlib,json,sys
from pathlib import Path
import numpy as np
p=Path(sys.argv[1]); J=lambda x:json.loads(x.read_text()); assert J(p/'result.json')['state']=='pass'
rate=44100; on=16384
windows=[(on+11025,on+33075),(on+22050,on+35280)]
faders=np.array([.125,.25,.375,.5,.75,1.]); train=[0,1,3]; held_out=[2,4,5]
peak_grid=np.geomspace(.025,1.30 if len(sys.argv)>2 else .90,8192)
wave_controls={'tri':np.float32(1/3),'saw':np.float32(2/3),'pulse':np.float32(1)}
db=lambda a:20*np.log10(np.maximum(a,1e-15))
def load(p,d):
 y=np.load(p,allow_pickle=False)
 assert y.dtype==np.dtype('<f4') and y.shape==(2,148684) and np.isfinite(y).all()
 assert hashlib.sha256(y.tobytes()).hexdigest()==d
 return y
def measures(w,wnd):
 x,y=wnd; f=w[:,x:y].astype(float); t=2*np.pi*440*np.arange(x,y)/rate
 base=np.stack([g(k*t) for g in (np.sin,np.cos) for k in (1,2,3,4,5,6)]+[np.ones(len(t))],axis=1)
 coeff=np.linalg.lstsq(base,f.T,rcond=None)[0]; h=np.hypot(coeff[:6],coeff[6:12])
 return np.stack([np.sqrt(np.mean(f*f,axis=1)),*h])
out={}
for shape in wave_controls:
 pref='native-jupiter-sine-gain-authored-compressor-'+shape+'-'
 a={}; raw=None; meta=None
 for folder in p.glob(pref+'*'):
  m=J(folder/'capture.json'); mode=m['ablation_arm'].split('-')[-1]; assert mode in ('on','off')
  assert m['forced_controls']['83']==float(wave_controls[shape]) and m['forced_controls']['17']==(float(np.float32(.02)) if mode=='on' else 0)
  assert m['forced_controls']['123']==0 and m['component_version']==67072 and m['osc2_fader'] in faders
  assert [(e['frame'],e['note']) for e in m['schedule']]==[(on,69),(60484,69)]
  y=load(folder/'output.npy',m['pcm_sha256'])
  a[(m['osc2_fader'],mode,m['repeat'])]=[measures(y,w) for w in windows]
  if m['osc2_fader']==1 and mode=='off' and not m['repeat']:raw=y; meta=m
 assert len(a)==14 and raw is not None
 b=p/f'bare-{shape}'; sm=J(b/'shape.json'); cal=J(b/'bare.json')
 assert sm['off_pcm_sha256']==meta['pcm_sha256'] and cal['component']=='aufx/dcmp/appl'
 assert sm['off_peak']==float(np.max(np.abs(raw[:,windows[0][0]:windows[0][1]])))
 unit=(raw/np.float32(sm['off_peak'])).astype('<f4')
 u=np.array([measures(unit,w) for w in windows])
 table={}; bypass=[]
 data=[(b,r) for r in cal['arms']]
 if shape=='saw' and len(sys.argv)>2:
  ext=Path(sys.argv[2]); em=J(ext/'bare.json'); es=J(ext/'shape.json')
  assert J(ext/'result.json')['state']=='pass' and es['off_pcm_sha256']==sm['off_pcm_sha256'] and es['off_peak']==sm['off_peak']
  assert cal['component']==em['component'] and cal['version']==em['version']
  assert em['arms'][0]['params']==cal['arms'][0]['params']
  data.extend((ext,r) for r in em['arms'])
 for where,r in data:
  assert r['buffers']==meta['buffers']
  y=load(where/f"bare-{r['id']}.npy",r['pcm_sha256'])
  if r['bypass']:
   bypass.append(float(np.max(np.abs(y-(unit.astype(float)*r['peak_in']).astype('<f4')))))
   continue
  bas=np.array([measures(y,w) for w in windows])
  table.setdefault(r['peak_in'],[]).append(db(bas/(u*r['peak_in'])))
 assert max(bypass)<4e-7
 levels=sorted(table)
 ref=np.array([np.median(table[k],axis=0) for k in levels])
 max_repeat=np.maximum.reduce([np.ptp(table[k],axis=0) for k in levels])
 fit_train=np.ones(len(peak_grid),bool); fit_test=fit_train.copy(); summary={}; partials={}
 for w in range(len(windows)):
  for ch in (0,1):
   for channel,k in (('rms',0),('h1',1),('h3',3),('h5',5)):
    off=np.array([a[(i,'off',False)][w][k,ch] for i in faders]); onv=np.array([a[(i,'on',False)][w][k,ch] for i in faders])
    ratios=np.array([a[(i,'off',False)][w][0,ch] for i in faders]); ratios=ratios/ratios[-1]
    obs=db(onv/off); repeat=abs(float(obs[3]-db(a[(.5,'on',True)][w][k,ch]/a[(.5,'off',True)][w][k,ch])))
    epsilon=max(.35,repeat+float(max_repeat[w,k,ch])+.10)
    xx=peak_grid[:,None]*ratios; valid=(xx>=min(levels))&(xx<=max(levels))
    pred=np.interp(np.log(xx),np.log(levels),ref[:,w,k,ch]); err=np.abs(pred-obs)
    good_valid=(valid[:,train]&(err[:,train]<=epsilon)).all(1)
    good_held=(valid[:,held_out]&(err[:,held_out]<=epsilon)).all(1)
    app=channel in ('rms','h1')
    if app:
     fit_train &= good_valid
     fit_test &= good_held
    rname=f'{w}:{ch}:{channel}'
    passes=peak_grid[good_valid&good_held]
    summary[rname]={'native_response_db':obs.tolist(),'off_rms_or_partial':off.tolist(),'epsilon_db':epsilon,'max_repeat_db':repeat,'bare_repeat_db':float(max_repeat[w,k,ch]),
      'peak_range':([float(passes[0]),float(passes[-1])] if len(passes) else []),'best_error_db':float(np.min(np.max(np.where(valid,err,999),axis=1))), 'source_off_ratios':ratios.tolist()}
 def bounds(mask):
  a=peak_grid[mask]
  return [] if not len(a) else [float(a[0]),float(a[-1])]
 out[shape]={'raw':float(wave_controls[shape]),'off_digest':sm['off_pcm_sha256'],'off_peak':sm['off_peak'],'bare_component_version':cal['version'],'bare_bypass_max_error':max(bypass),
   'bare_max_calibrated_peak':max(levels),'training_range':bounds(fit_train),'heldout_range':bounds(fit_train&fit_test),'report':summary}
print(json.dumps(out,indent=2))
