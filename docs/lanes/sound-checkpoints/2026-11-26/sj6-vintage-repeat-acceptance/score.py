"""Analyze only immutable signed captures and existing scalar host receipts."""
from pathlib import Path
import hashlib,tarfile,io,json,numpy as np
def sha(x):return hashlib.sha256(x).hexdigest()
a=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
assert sha(a.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
r={}
def load(t,k):
 p='./program-'+k
 m=json.load(t.extractfile(p+'/capture.json'))
 x=np.load(io.BytesIO(t.extractfile(p+'/output.npy').read()),allow_pickle=False)
 assert m['active']==m['after'] and sha(x.tobytes())==m['pcm_sha256'] and x.shape==(2,148684)
 assert m['warm_frames']==16384 and m['rate']==44100
 return m,x.T.astype('f8')
windows=[('first_0_16',38434,39140),('first_16_50',39140,40639),('last_0_16',82534,83240),('last_16_50',83240,84739),('last_50_100',84739,86944)]
rms=lambda y:float(np.sqrt(np.mean(y*y)))
db=lambda x:float(20*np.log10(x)) if x else None
with tarfile.open(a) as t:
 for i,n in [(1,'Solidify'),(2,'Raw neutral')]:
  m,h=load(t,f'{i}-authored'); mr,k=load(t,f'{i}-authored-repeat')
  assert (m['active'],m['schedule'],m['cue_sha256'])==(mr['active'],mr['schedule'],mr['cue_sha256'])
  mm,d=load(t,f'{i}-dry')
  receipt={}
  assert m['schedule']==mm['schedule']
  archive=Path(f'sj6-cross-preset/sub-range.private-output/{i}-authored.private.f32')
  v=np.fromfile(archive,'<f4').reshape(-1,2)
  assert v.shape==(148684,2)
  for w,l,u in windows:
   x,y,z,c=(p[l:u] for p in (h,k,d,v))
   receipt[w]={'authored_dbfs':db(rms(x)),'repeat_dbfs':db(rms(y)),
      'authored_repeat_error_dbfs':db(rms(x-y)), 'authored_repeat_dbr':db(rms(x-y)/rms(x)) if rms(x) else None,
      'dry_single_dbfs':db(rms(z)), 'range_saved_compact_against_both_dbfs':[db(rms(c-p)) for p in (x,y)]}
  r[n]={'authored_repeat_bit_identical':np.array_equal(h,k),'windows':receipt}
d=json.load(open('sj6-native-vintage-owner/seeds.json'))
native=json.load(open('sj6-native-vintage-owner/runtime.json'))
prior=json.load(open('sj6-solidify-tail-slope/result.json'))
rows={}
for label,z in [('first','first_release'),('last','final_early')]:
  vals=[]
  for k,w in d.items():
   if k.startswith('same_frame:') and k.endswith(':authored'):
    x=w['rows'][z]
    vals.append(x['after_vs_native']-x['before_vs_native'])
  assert len(vals)==4
  rows[label]={'native_dry_draws':1,'num_host_seeds':4,'change_dbr_per_seed':vals,'median_change_dbr':float(np.median(vals)),
              'improved_seed_count':sum(x<0 for x in vals)}
# Authored default-seed retained and redraw receipt was scored 82534:83239
# vs first genuine authored only. Retain against both exists in prior authored
# phase slope archive for 82534:83240 (different by one sample). Avoid
# mixing windows: compute rigorous bounds in the exact Vintage receipt window.
l,u=82534,83239
with tarfile.open(a) as t:
 _,v0=load(t,'1-authored');_,v1=load(t,'1-authored-repeat')
va,vb=v0[l:u],v1[l:u]
dd=rms(va-vb)
d0=native['1-authored']['rows']['final_early']
triangle={}
for option,col in [('retain','before_vs_saved'),('redraw','after_vs_saved')]:
    d1=rms(va)*10**(d0[col]/20)
    triangle[option]={'against_authored_a_dbfs':db(d1),
         'against_authored_b_bounds_dbfs':[db(abs(d1-dd)),db(d1+dd)]}
p=prior['windows']['final']['0:16']; retained_exact_vary={
  w:p[w]['signed_residual_dbfs'] for w in ('2-authored_vs_authored','2-authored_vs_authored-repeat')}
print(json.dumps(dict(native_repeats=r,dry_seed_summary=rows,
                     authored_one_seed_against_repeat_bounds=triangle,
                     existing_exact_retained_vs_both_0_16ms=retained_exact_vary,
                     scope='No host rerender, no new native captures, no inferred matching random state; bounds only if candidate PCM missing'),indent=2,allow_nan=False))
