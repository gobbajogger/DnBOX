"""Whole-chain signed compare. A real compact receipt needs all authored readbacks."""
from pathlib import Path
import argparse,hashlib,io,json,tarfile,re,subprocess
import numpy as np
D=Path(__file__).resolve().parent; P=D/'private'
manifest=json.loads((P/'reference-manifest.json').read_text())
ap=argparse.ArgumentParser();ap.add_argument('--pilot',action='store_true');ap.add_argument('--arm',nargs=3,action='append',metavar=('ROLE','PCM','RECEIPT'));args=ap.parse_args()
hashbytes=lambda b:hashlib.sha256(b).hexdigest()
roles=('authored','verb-off')
FS=manifest['rate'];N=manifest['frames']; cue=manifest['schedule'];expected=[(e['frame'],e['note'],e['velocity'],int(e['velocity']!=0)) for e in cue]
def admit(r, pcm, role):
 for k in ('rate','frames','preset_uid','schedule','buffers','cue_sha256'):
  if r.get(k)!=manifest[k]:raise ValueError('candidate '+role+' missing/mismatched '+k)
 if r.get('controls_before')!=manifest['controls'][role] or r.get('controls_after')!=manifest['controls'][role]:
  raise ValueError('candidate '+role+' missing/mismatched 170 before/after Float32 words')
 if r.get('gate_receipts')!=[list(x) for x in expected]:raise ValueError('candidate gate receipts differ')
 if r.get('source_class') not in ('actual_combined_host','actual_native_firmware'):raise ValueError('not a whole-path producer')
 if not re.fullmatch(r'[0-9a-f]{64}',r.get('build_sha256','')):raise ValueError('candidate unbound build')
 if r.get('pcm_sha256')!=hashbytes(pcm):raise ValueError('candidate PCM not bound')
if args.pilot and args.arm:ap.error('do not use diagnostic with admitted mode')
ref={}
with tarfile.open(P/'warm-brainwave-native.private.tgz') as t:
 for r in (*roles,'authored-repeat','verb-off-repeat'):
  x=np.load(io.BytesIO(t.extractfile('native-warm-brainwave-'+r+'/output.npy').read()),allow_pickle=False)
  assert x.shape==(2,N) and x.dtype==np.dtype('<f4') and np.isfinite(x).all()
  assert manifest['pcm_sha256'][r]==hashbytes(x.tobytes())
  ref[r]=x.astype('f8')
out={};receipts={};input_hashes={}
def readpcm(b):
 assert len(b)==N*8
 x=np.frombuffer(b,dtype='<f4').reshape(2,N).astype('f8')
 assert np.isfinite(x).all()
 return x
if args.pilot:
 for n,r in zip(('1','0'),roles):
  f=subprocess.run([str(D/'render'),n],capture_output=True,check=True)
  log=f.stderr.decode(); parser=r'^event (\d+) (\d+) (\d+) (\d+)$'
  gate_events=[tuple(map(int,x)) for x in re.findall(parser,log,re.M)]; assert gate_events==expected
  assert 'pass runtime INIT=0' in log
  out[r]=readpcm(f.stdout)
  assert np.max(np.abs(out[r]))>0 and not out[r][:,:16384].any()
  (P/f'baseline-{r}.f32').write_bytes(f.stdout)
  (P/f'baseline-{r}.events').write_text(log)
  receipts[r]=dict(frames=N,rate=FS,gate_receipts=gate_events,build_sha256=hashbytes((D/'render').read_bytes()),
                   preset='compact SuperJ6 INIT slot0; original Brainwave UID NOT installed',
                   controls_transplant='room only; 7-bit detents; no inferred author identity',pcm_sha256=hashbytes(f.stdout))
 # Negative tests on strict admission; diagnostic mode can never set a pass.
 try:admit(receipts['authored'],(P/'baseline-authored.f32').read_bytes(),'authored')
 except ValueError as e:refusal=str(e)
 else:raise AssertionError('partial preset adoption was incorrectly admitted')
else:
 if not args.arm or sorted(x[0] for x in args.arm)!=sorted(roles):ap.error('need exactly authored and verb-off with signed complete receipts')
 for r,p,s in args.arm:
  try:
   b=Path(p).read_bytes(); c=json.loads(Path(s).read_text());admit(c,b,r)
  except ValueError as e:ap.error(str(e))
  out[r]=readpcm(b); receipts[r]=c
 if receipts['authored']['build_sha256']!=receipts['verb-off']['build_sha256']:ap.error('arms differ in build')
def powr(x):return np.mean(x*x,axis=1)
def db(p,q):return (10*np.log10(np.maximum(p,1e-80)/np.maximum(q,1e-80))).tolist()
def metric(a,b):
 pa=powr(a);pb=powr(b); err=powr(a-b)
 dot=np.sum(a*b,axis=1)
 den=np.sqrt(np.sum(a*a,axis=1)*np.sum(b*b,axis=1))
 return dict(ref_dbfs=db(pa,1),trial_dbfs=db(pb,1),level_delta_db=db(pb,pa),signed_residual_dbr=db(err,pa),absolute_error_dbfs=db(err,1),
    correlation=[None if not z else float(np.clip(n/z,-1,1)) for n,z in zip(dot,den)])
slices={'leading_silence':(0,16384),'single':(16384,38434),'chord':(49459,82534),'release':(82534,N),'last_second':(N-FS,N),'whole':(16384,N)}
def allm(a,b):return {k:metric(a[:,i:j],b[:,i:j]) for k,(i,j) in slices.items()}
w=ref['authored']-ref['verb-off']; g=ref['authored-repeat']-ref['verb-off-repeat']; c=out['authored']-out['verb-off']
assert np.max(np.abs(w))>0 and np.max(np.abs(c))>0
result=dict(scope='same cue diagnostic; different preset' if args.pilot else 'exact-bound signed matched preset A/B',
    comparison_admitted=not args.pilot, binding_refusal=refusal if args.pilot else None,
    room_off_is_not_all_fx_off=True,installed_au_sha256=manifest['installed_au_sha256'],cue_sha256=manifest['cue_sha256'],
    ref_pcm_shas=manifest['pcm_sha256'], candidate_receipts=receipts,
    authored=allm(ref['authored'],out['authored']),verb_off=allm(ref['verb-off'],out['verb-off']),
    signed_room_delta=allm(w,c),source_delta_repeat_floor=allm(w,g),
    original_wet_repeat=allm(ref['authored'],ref['authored-repeat']),
    original_off_repeat=allm(ref['verb-off'],ref['verb-off-repeat']))
print(json.dumps(result,indent=2,allow_nan=False))
