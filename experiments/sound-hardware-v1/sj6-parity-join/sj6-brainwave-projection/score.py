import hashlib,io,json,pathlib,re,tarfile
import numpy as np
p=pathlib.Path(__file__).resolve().parent
s=p.parent/'sj6-parity-suite'/'private'
m=json.loads((s/'reference-manifest.json').read_text())
N=m['frames']
h=lambda b:hashlib.sha256(b).hexdigest()
expected=[(e['frame'],e['note'],e['velocity'],int(e['velocity']!=0)) for e in m['schedule']]
ref={}
with tarfile.open(s/'warm-brainwave-native.private.tgz') as t:
 for role in ('authored','verb-off'):
  a=np.load(io.BytesIO(t.extractfile('native-warm-brainwave-'+role+'/output.npy').read()),allow_pickle=False)
  assert a.shape==(2,N) and a.dtype==np.dtype('<f4') and h(a.tobytes())==m['pcm_sha256'][role]
  ref[role]=a.astype('f8')
candidate={}
for proj in (0,1,2,3):
 for wet,role in [(0,'verb-off'),(1,'authored')]:
  log=(p/'private'/f'{wet}-{proj}.log').read_text()
  assert [tuple(map(int,x)) for x in re.findall(r'^event (\d+) (\d+) (\d+) (\d+)$',log,re.M)]==expected
  assert ('projection verified; not full preset' in log)==bool(proj)
  assert 'pass runtime INIT=0' in log
  assert ('authored Unison receiver active' in log)==(proj==2)
  assert ('explicit diagnostic unity master' in log)==(proj==3)
  a=np.fromfile(p/'private'/f'{wet}-{proj}.f32',dtype='<f4').reshape(2,N)
  assert not a[:,:16384].any() and np.isfinite(a).all() and np.abs(a).max()>0
  candidate[proj,role]=a.astype('f8')
assert all(not np.array_equal(candidate[0,r],candidate[1,r]) for r in ref)
ref['signed-room-delta']=ref['authored']-ref['verb-off']
for proj in (0,1,2,3):candidate[proj,'signed-room-delta']=candidate[proj,'authored']-candidate[proj,'verb-off']
def score(a,b):
 power=lambda x:np.maximum(np.mean(x*x,axis=1),1.e-70)
 return {'level_error_db':(10*np.log10(power(b)/power(a))).tolist(),'signed_residual_dbr':(10*np.log10(power(b-a)/power(a))).tolist()}
segments={'held':slice(16384,38434),'chord':slice(49459,82534),'release':slice(82534,N),'whole':slice(16384,N)}
out={}
for role,a in ref.items():
 out[role]={str(proj):{k:score(a[:,v],candidate[proj,role][:,v]) for k,v in segments.items()} for proj in (0,1,2,3)}
ratios={}
for role in ('authored','verb-off'):
 a,b=candidate[1,role],candidate[3,role]
 mask=abs(b)>1.e-8
 ratio=a[mask]/b[mask]
 ratios[role]={'samples':len(ratio),'median':float(np.median(ratio)),'span':float(np.ptp(ratio)),'db':float(20*np.log10(np.median(ratio)))}
 assert np.ptp(ratio)<1.e-6
print(json.dumps({'master_ablation':ratios,'admitted_whole_preset_match':False,'scope':'Same cue, partial source-backed authored controls projected through real runtime. Unmapped controls still use Init. No alignment, fitted gain, or copied full receiver receipts.','native_au_hash':m['installed_au_sha256'],'actual_raw_word_checks':16,'projection':out},indent=2))
