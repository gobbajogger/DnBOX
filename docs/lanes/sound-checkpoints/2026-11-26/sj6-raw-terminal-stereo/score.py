"""Immutable same-cue dry/chorus pair decomposition, no fit, no new renders."""
from pathlib import Path
import hashlib,tarfile,io,json,numpy as np
b=Path('sj6-cross-preset')
a=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
def sha(x):return hashlib.sha256(x).hexdigest()
assert sha(a.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
frozen=json.load(open(b/'sub-range.private-output/manifests.json'))
R={};C={};P={};pins={}
with tarfile.open(a) as t:
 for k in ('dry','authored','authored-repeat'):
  p=f'./program-2-{k}/'
  q=json.load(t.extractfile(p+'capture.json'))
  x=np.load(io.BytesIO(t.extractfile(p+'output.npy').read()),allow_pickle=False)
  assert x.shape==(2,148684) and q['active']==q['after'] and sha(x.tobytes())==q['pcm_sha256']
  assert q['warm_frames']==16384 and q['rate']==44100
  R[k]=x.astype('f8');P[k]=q
  pins[k]=[q['pcm_sha256']]
  if k!='authored-repeat':
   c=(b/'sub-range.private-output'/f'2-{k}.private.f32').read_bytes()
   m=next(z for z in frozen if z['uid']==q['wanted']['program_uid'] and z['arm']==k)
   assert m['compact_sha']==sha(c) and m['reference_pcm_sha']==q['pcm_sha256']
   C[k]=np.frombuffer(c,'<f4').reshape(-1,2).T.astype('f8')
   pins[k].append(sha(c))
assert np.array_equal(R['authored'],R['authored-repeat'])
assert all(P[k]['schedule']==P['dry']['schedule'] and P[k]['cue_sha256']==P['dry']['cue_sha256'] for k in P)
qa,qb=({x['address']:x['value'] for x in P[k]['active']} for k in ('authored','dry'))
assert {k for k in qa if qa[k]!=qb[k]}=={'40'} and (qa['40'],qb['40'],qa['208'])==(1,0,1)
def basis(a):return (a[0]+a[1])*.5,(a[0]-a[1])*.5
def db(x):return float(20*np.log10(x)) if x>0 else None
def norm(x):return float(np.linalg.norm(x))
win=[('attack',16384,20794),('first_hold',20794,38434),('release',38434,49459),
     ('chord_hold',53869,82534),('final_tail',82534,104584),('whole',16384,148684)]
result={}
for key,lo,hi in win:
    vals={}
    for name,Q in [('native',R),('compact',C)]:
      dry=Q['dry'][:,lo:hi];auth=Q['authored'][:,lo:hi];wet=auth-dry
      vals[name]={k:basis(z) for k,z in [('dry',dry),('authored',auth),('delta',wet)]}
    row={}
    for k in ('dry','authored','delta'):
      x=vals['compact'][k]; y=vals['native'][k]
      row[k]=dict(relative_to_own_dry_mid_db={n:[db(norm(w)/norm(vals[n]['dry'][0])) for w in d] for n,d in [('native',y),('compact',x)]}, side_to_mid_dB={n:db(norm(d[1])/norm(d[0])) for n,d in [('native',y),('compact',x)]}, 
                  compact_to_native_level_db=[db(norm(a)/norm(b))for a,b in zip(x,y)],
                  compact_signed_error_dbr=[db(norm(a-b)/norm(b))for a,b in zip(x,y)],
                  rms_dbfs={n:[db(norm(w)/(len(w)**.5))for w in d] for n,d in [('native',y),('compact',x)]})
    result[key]=row
print(json.dumps(dict(pins=pins,scope='Raw current captured version with physical public Chorus-I switch as sole change, repeated authored tight baseline. Compact exact previous Range-only. Orthogonal stereo split never changes waveform, delta contains conditioned switching / private topology and is not asserted to be a pure return.',measured=result),indent=2,allow_nan=False))
