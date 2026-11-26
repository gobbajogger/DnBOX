"""Signed same-UID diagnostic. No fitted level, clock, offset, trimming, or phase."""
from pathlib import Path
import hashlib,io,json,sys,tarfile
import numpy as np
au,root,scorer=map(Path,sys.argv[1:4])
sys.path.insert(0,str(scorer.resolve()))
from compare import channel_score
digest=lambda b:hashlib.sha256(b).hexdigest()
assert digest(au.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
def calc(reference,candidate):
 a=reference.astype('f8');b=candidate.astype('f8')
 peak=lambda x:float(np.max(abs(x)))
 n=np.linalg.norm(a);dn=np.linalg.norm(b-a)
 log=lambda v:float(20*np.log10(v)) if v>0 else None
 return dict(ref_rms=float(np.sqrt(np.mean(a*a))),cand_rms=float(np.sqrt(np.mean(b*b))),
     ref_peak=peak(a),cand_peak=peak(b),signed_dbr=log(dn/n) if n else None,
     level_delta_db=log(np.linalg.norm(b)/n) if n else None,
     log_spectral_rms_db=channel_score(a,b,44100,(20,16000))[1] if len(a)>=2048 else None)
windows={'whole':(0,132300),'attack':(0,4410),'single_hold':(4410,22050),
 'single_release':(22050,33075),'chord_attack':(33075,37485),
 'chord_hold':(37485,66150),'early_tail':(66150,88200),'late_tail':(88200,132300)}
def analyze(ref,can):
 return {k:[calc(a[lo:hi],b[lo:hi]) for a,b in zip(ref,can)] for k,(lo,hi) in windows.items()}
def npy(t,p):
 f=t.extractfile(p)
 return np.load(io.BytesIO(f.read()),allow_pickle=False)
results=[]
m=json.loads((root/'manifests.json').read_text())
with tarfile.open(au) as t:
 for i in range(3):
  native={};compact={}
  for arm in ('authored','dry','authored-repeat'):
   p=f'./program-{i}-{arm}'
   ref=npy(t,p+'/output.npy')
   cap=json.load(t.extractfile(p+'/capture.json'))
   assert ref.shape==(2,148684) and ref.dtype==np.dtype('<f4') and np.isfinite(ref).all()
   assert digest(ref.tobytes())==cap['pcm_sha256'] and cap['warm_frames']==16384
   native[arm]=ref[:,16384:]
   if arm=='authored-repeat':continue
   row=[e for e in m if e['uid']==cap['wanted']['program_uid'] and e['arm']==arm]
   assert len(row)==1 and row[0]['reference_pcm_sha']==cap['pcm_sha256']
   pcm=(root/f'{i}-{arm}.private.f32').read_bytes()
   assert digest(pcm)==row[0]['compact_sha']
   x=np.frombuffer(pcm,dtype='<f4').reshape((148684,2)).T
   assert np.isfinite(x).all() and len(x[0])==cap['frames'] and not np.any(x[:,:16384])
   compact[arm]=x[:,16384:]
  results.append(dict(program=cap['wanted']['program'],uid=cap['wanted']['program_uid'],
    authored=analyze(native['authored'],compact['authored']),dry=analyze(native['dry'],compact['dry']),
    native_repeat=analyze(native['authored'],native['authored-repeat']),
    fx_difference=analyze(native['authored']-native['dry'],compact['authored']-compact['dry']),
    interpretation="FX difference is return-side response to same events, not same-input isolated leaf proof"))
print(json.dumps(results,indent=2,allow_nan=False))
