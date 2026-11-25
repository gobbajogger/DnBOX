import numpy as np
from pathlib import Path
d=Path(__file__).parent
def rms(x):return np.sqrt(np.mean(x*x))
def db(x):return 20*np.log10(max(x,1e-30))
def corr(a,b):
 return np.mean(a*b)/max(rms(a)*rms(b),1e-30)
for i in (62,113,128,278,435):
 def data(s,m):return np.fromfile(d/f'{i}-{s}-{m}.f32','<f4').reshape(-1,2).astype('d')
 out={'case':i}
 for m in ('wet','reverb'):
  for w,x in [('early',slice(198450,220500)),('late',slice(220500,None))]:
   arr={(s,a):data(s,a)[x] for s in ('reference','old','new') for a in ('dry','delay',m)}
   wet={s:arr[s,m]-arr[s,'dry'] for s in ('reference','old','new')}
   if m=='wet':
    delay=arr['old','delay']-arr['old','dry']
    delta=wet['new']-wet['old']
    iso=(data('new','reverb')[x]-data('old','reverb')[x])
    if rms(delta):
     out[w+'_isolated_predicts_full_change_corr']=round(corr(delta,iso),3)
     out[w+'_change_corr_with_baseline']=round(corr(wet['old'],delta),3)
     if rms(delay):out[w+'_change_corr_with_delay']=round(corr(delay,delta),3)
   out[w+'_'+m+'_new_minus_old_db']=round(db(rms(wet['new'])/rms(wet['old'])),3)
 print(out)
