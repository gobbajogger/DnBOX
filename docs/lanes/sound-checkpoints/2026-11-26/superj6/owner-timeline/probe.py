from io import BytesIO
import ctypes as C
from pathlib import Path
import tarfile,json,hashlib
import numpy as np
D=Path('recovered-sound')
s=lambda b:hashlib.sha256(b).hexdigest()
ref=D/'SJ6-Brainwave-Warm-Main-Sub.native.private.tgz'
afx=D/'SJ6-Brainwave-Warm-Main-Sub-Crusher-native.private.tgz'
hp=D/'SJ6-Brainwave-OTA-order-native-Chain.private.tgz'
prefix=D/'SJ6-Brainwave-Main-Scale-Order-Apple-Prefix.private.tgz'
opens=[tarfile.open(a) for a in (ref,afx,hp)]
assert s(afx.read_bytes())=='0ce56bbedd2b7627edec53809a6887f53ef98a02529854829c730d58b5ecd8cc'
assert s(ref.read_bytes())=='9db19abc7c56183e745e1f3de150559b319dabda4817ebd30af9a9db356c4b53'
assert s(hp.read_bytes())=='7457978e9374d93763144e467579981dd9269a487fbb151b47a6e3c2c4d5f136'
suffix=D/'main-route/suffix/settled_suffix.so'
lib=C.CDLL(str(suffix.resolve()));f=lib.sj6_settled_postcrusher
ptr=np.ctypeslib.ndpointer(dtype=np.float32,ndim=2,flags='C_CONTIGUOUS')
f.argtypes=[ptr,ptr,C.c_uint,C.POINTER(C.c_uint),C.c_uint,C.c_float]
get=lambda t,n:t.extractfile(n).read()
pcm=lambda a:np.frombuffer(a,dtype='<f4').reshape(2,148684)
def report(key):
 r,leaf,new=opens
 pre='native-warm-brainwave-'+key+'-only/'
 rc=json.loads(get(r,pre+'capture.json')); real=np.load(BytesIO(get(r,pre+'output.npy')),allow_pickle=False)
 assert rc['installed_au_sha256']=='1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
 assert s(real.tobytes())==rc['pcm_sha256'] and rc['parameters']==rc['parameter_readback_after']
 cr=leaf if key=='sub' else new
 path='native-sub' if key=='sub' else 'candidate-crusher'
 cj=json.loads(get(cr,path+'/capture.json'))
 assert cj['native_before']==cj['native_after']
 assert cj['ordered_block_sizes']==rc['buffers'] and cj['scheduled_events']==rc['schedule']
 assert cj['source_pcm_sha256']==cj['input_file_sha256']==cj['bypass_pcm_sha256']
 assert cj['forced_parameter_values']=={'3':0.44387006759643555,'4':0,'5':0,'15':0}
 out=get(cr,path+'/crusher-out.f32');assert s(out)==cj['output_pcm_sha256'];after=pcm(out)
 if key=='sub':
  src=np.load(BytesIO(get(cr,path+'/input.npy')),allow_pickle=False)
  if src.shape==(148684,2): src=src.T.copy()
  assert s(src.tobytes())==cj['input_pcm_sha256'];before=None
 else:
  hrec=json.loads(get(new,'candidate-hpf/capture.json'))
  src=pcm(get(new,'candidate-hpf/hpf-out.f32'));assert s(src.tobytes())==cj['input_pcm_sha256']==hrec['output_pcm_sha256']; assert hrec['native_before']==hrec['native_after']=={'0':10,'1':0}
  before=pcm(get(tarfile.open(prefix),'candidate.f32'));assert s(before.tobytes())==hrec['input_pcm_sha256']
 z=np.empty_like(after);b=(C.c_uint*len(rc['buffers']))(*rc['buffers'])
 raw=next(a['value'] for a in rc['parameters'] if str(a['address'])=='0')
 assert f(after,z,rc['frames'],b,len(b),raw)==1
 def db(n,d):return float(20*np.log10(max(n,1e-30)/max(d,1e-30)))
 # Ratios are metrics only, not calibration of candidate PCM.
 def window(i,j):
  q=real[:,i:j].astype(float);y=z[:,i:j].astype(float)
  per=[]
  for a,b in zip(y,q):
   na,nb=np.linalg.norm(a),np.linalg.norm(b)
   if min(na,nb)<1e-10:per.append(None);continue
   corr=float(np.dot(a,b)/(na*nb))
   ccos=np.clip(corr,-1.,1.)
   per.append({'level_db':db(na,nb),'raw_null_db':db(np.linalg.norm(a-b),nb),
                'signed_correlation':corr,
                'positive_scalar_invariant_shape_error_db':20*np.log10(max(1e-15,np.sqrt(max(0.,1-ccos**2)))) if corr>0 else 0.})
  return per
 evs=rc['schedule']
 assert [i['frame'] for i in evs]==[16384,38434,49459,49459,49459,82534,82534,82534]
 short=[]
 for n,ca in [('initial_key',16384),('first_release',38434),('chord_key_ons',49459),('older_owner_commit_marker',49875),('last_key_off',82534)]:
  offsets=(-128,0,128,256,384,512,768,1024,2048)
  short.append(dict(event=n, center_frame=ca, windows=[dict(start=i,stop=j,channels=window(ca+i,ca+j)) for i,j in zip(offsets,offsets[1:])]))
 return dict(native_pcm=rc['pcm_sha256'],candidate_waveform_hash=s(z.tobytes()),candidate_source=cj['source_pcm_sha256'],operator=key,
  same_event_state=evs, windows=short,
  long_controls={n:window(i,j) for n,i,j in [('held_one_key',24576,37634),('held_chord',53971,80329),('final_release',82534,115302)]})
print(json.dumps(dict(scope='complete official same cue Mac A/B with source/leaf replay; gain invariant shape trace, no fitted candidate',main=report('main'),sub=report('sub')),indent=2))
