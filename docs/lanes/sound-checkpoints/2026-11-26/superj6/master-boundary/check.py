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
 def met(i,j):
  ref=real[:,i:j].astype(float);current=z[:,i:j].astype(float)
  norm=lambda d:np.linalg.norm(d.astype(float))
  return {'terminal_level_db':db(norm(current),norm(ref)),'terminal_null_dbr':db(norm(current-ref),norm(ref)),'native_leaf_hpf_db':None if before is None else db(norm(src[:,i:j]),norm(before[:,i:j])),'native_leaf_crusher_db':db(norm(after[:,i:j]),norm(src[:,i:j])),'source_suffix_db':db(norm(current),norm(after[:,i:j])),'signed_corr':float(np.vdot(ref,current)/(norm(ref)*norm(current)))}
 return {'source_digest':cj['source_pcm_sha256'],'native_reference_digest':rc['pcm_sha256'],'external_leaf_receipt': 'host HP then real Apple AUDistortion' if key=='sub' else 'real Apple HP10 and AUDistortion', 'end_boundary':'source-backed guarded host suffix; not the loaded instrument','windows':{n:met(i,j) for n,i,j in [('attack',16384,18432),('held',24576,37634),('pending',49459,49875),('chord',53971,81434),('release',82534,115302)]}}
print(json.dumps(dict(source_hashes={a.name:s(a.read_bytes()) for a in (ref,afx,hp,prefix,suffix)},main=report('main'),sub=report('sub')),indent=2))
