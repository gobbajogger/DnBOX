"""Replay the authorized current compiled standalone Jupiter-Sine generated leaf.
Private AU/PCM dependencies are separately held; they are not embedded here.
Run from workspace root with recovered oracle and PYTHONPATH tools as shown README.
"""
from pathlib import Path
import tarfile,ctypes as C,math
from io import BytesIO
SOURCE=Path('sj6-modern-owner/probe.py')
exec(compile(SOURCE.read_text().split('def trial(')[0],str(SOURCE),'exec'))
assert pin(p)=='c5cffc17f3e0d5a61ad53727c8c55243fef348151dc8d9263b7e8cb5ee30fad4'
A=Path('jupiter-gain/native/SJ6-Direct-AUv3-Jupiter-Sine-Gain.tgz')
assert pin(A)=='b0580a3f13c9070cb34f589015d345c022d32e2557e3af541bb60a5526289cb0'
S=Path('recovered-sound/main-route/suffix/settled_suffix.so')
assert pin(S)=='4d1d9eb81432a082398acced46fbab1a89fe767b6d1fdc0c57bd0a1750b16c38'
leaf=C.CDLL(str(S.resolve())).sj6_settled_postcrusher
leaf.argtypes=[C.c_void_p,C.c_void_p,C.c_uint,C.POINTER(C.c_uint),C.c_uint,C.c_float]
leaf.restype=C.c_int
def fixture(name):
 with tarfile.open(A) as t:
  d=json.load(t.extractfile(name+'/capture.json'))
  x=np.load(BytesIO(t.extractfile(name+'/output.npy').read()),allow_pickle=False)
 assert d['rate']==44100 and d['preset_name']==d['preset_name_after']=='Jupiter Sine'
 assert d['parameters']==d['parameter_readback_after']
 assert sum(d['buffers'])==x.shape[1]==148684 and x.shape[0]==2 and x.dtype==np.float32 and np.isfinite(x).all()
 assert hashlib.sha256(x.tobytes()).hexdigest()==d['pcm_sha256']
 assert [(a['frame'],a['velocity'],a['note']) for a in d['schedule']]==[(16384,87,69),(60484,0,69)]
 controls={x['address']:x['value'] for x in d['parameters']}
 assert controls['0']==d['actual_master'] and all(controls[str(i)]==0 for i in (107,108,109))
 return d,x
def raw(j,blocks):
 e=Emu(str(BIN)); d=SamplerDSP(e,44100)
 d.load_placeholder()
 for id,w in cfg['sampler_control_writes']['oscSampler'+str(j)][:57]:d.set_param(id,w,True)
 for id,w in [(0,.808),(1,0),(2,0),(3,0),(8,0),(9,0),(10,.002),(12,.002),(13,1),(15,3.517968233344483),(20,0),(25,0),(28,1),(29,0),(32,0),(36,20000),(37,440),(38,j+1),(39,1),(40,0),(41,0),(42,0),(43,1),(44,0)]:
  d.set_param(id,w,True)
 e.call(0x1003898c4,d.dsp,4);e.call(0x1003898a4,d.dsp,0)
 at=0;out=[]
 for n in blocks:
  if at==16384:e.call(0x100389c84,d.dsp,69,87,floats=(0.,))
  if at==60484:e.call(0x100389c8c,d.dsp,69,0)
  assert not any(at<x<at+n for x in (16384,60484))
  out.append(d.render(n));at+=n
 assert not e.unknown
 return np.concatenate(out,axis=1)
def hp(x):
 # Audio-unit HiPass model on same signed stimulus, double RBJ Q=1.
 import scipy.signal
 omega=2*math.pi*10/44100;cos=math.cos(omega);al=math.sin(omega)/2
 b=[(1+cos)/2,-(1+cos),(1+cos)/2];a=[1+al,-2*cos,1-al]
 return scipy.signal.lfilter(b,a,x,axis=1).astype('f4')
def suffix(x,blocks,raw_master):
 y=np.empty_like(x); sizes=(C.c_uint*len(blocks))(*blocks)
 assert leaf(x.ctypes.data,y.ctypes.data,x.shape[1],sizes,len(blocks),raw_master)
 return y
def rms(x):return np.sqrt(np.mean(np.square(x[:,25000:55000],dtype='f8'),axis=1))
def stage(x,d,ref):
 y=suffix(x,d['buffers'],d['actual_master'])
 return dict(rms=rms(y).tolist(),relative_db=(20*np.log10(rms(y)/rms(ref))).tolist(),
  same_cue_signed_null=(20*np.log10(rms(y-ref)/rms(ref))).tolist())
out={}
for j in (1,2):
 n=f'native-jupiter-sine-gain-authored-osc{j}-only';rn=f'native-jupiter-sine-gain-half-osc{j}-only'
 d,x=fixture(n);h,y=fixture(rn);r,u=fixture(n+'-repeat')
 assert d['installed_au_sha256']==h['installed_au_sha256']==r['installed_au_sha256']
 assert d['buffers']==h['buffers']==r['buffers'] and d['schedule']==h['schedule']==r['schedule']
 pcm=raw(j,d['buffers'])
 q=hp(pcm*np.float32(5/6))
 import scipy.signal
 c=scipy.signal.lfilter([1.,-1.],[1.,-.995],q,axis=1).astype('f4')
 assert np.allclose(rms(pcm),.19568721543143547,rtol=2e-6),rms(pcm)
 out[str(j)]=dict(original_stem_rms=rms(pcm).tolist(),original_native_full_rms=rms(x).tolist(),original_native_repeat_rms=rms(u).tolist(),original_native_half_rms=rms(y).tolist(),
    diagnostic_bypass={'authored':stage(q,d,x),'half':stage(q,h,y)},diagnostic_active_zero={'authored':stage(c,d,x),'half':stage(c,h,y)})
print(json.dumps(dict(executable_sha256=pin(BIN),capture_archive_sha256=pin(A),
  cutoff_configuration_sha256=pin(p),suffix_sha256=pin(S),cases=out,scope='True current Mac ARM64 isolated generated Sine core. Same signed cue, conditional 5/6→HP10 Q1→optional connected zero Crusher (no external Booster)→source-inspired settled suffix; not a full native internal render. No new fitted gain/time/pan.'),indent=2))
