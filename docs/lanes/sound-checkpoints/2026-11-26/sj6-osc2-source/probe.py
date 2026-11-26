"""Current original oscillator2 profile path vs direct dry native stem."""
from pathlib import Path
from io import BytesIO
import struct,tarfile,ctypes as C,hashlib,json
import numpy as np
from unicorn import UC_HOOK_CODE
exec(compile(Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0],'verified modern owner prefix','exec'))
ARCH=Path('sj6-direct-stage/SJ6-Brainwave-Direct-AUv3-Fanin.tgz')
assert pin(ARCH)=='d3821515187f4c086cde7ecbff57bafbae216fbd86becd1ff356fb1fe4a5df9f'
with tarfile.open(ARCH) as t:
 base='native-warm-brainwave-fanin-authored-osc2-only/'
 r=json.load(t.extractfile(base+'capture.json'))
 x=np.load(BytesIO(t.extractfile(base+'output.npy').read()),allow_pickle=False)
 p0={q['address']:q['value'] for q in r['parameters']}
 assert r['parameters']==r['parameter_readback_after'] and p0['1']==p0['202']==p0['201']==p0['108']==0 and p0['3']==1
 assert r['installed_au_sha256']=='1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
 assert hashlib.sha256(x.tobytes()).hexdigest()==r['pcm_sha256']
 assert r['rate']==44100 and x.shape==(2,148684)
def render(profile,factor,cutoff,force_dc=False):
 e=Emu(str(BIN));d=SamplerDSP(e,44100);d.load_placeholder()
 for k,v in cfg['sampler_control_writes']['oscSampler2']:d.set_param(k,v,True)
 e.call(0x1003898c4,d.dsp,factor)
 if profile:
  word=e.rd(0x1007e6588,4)
  assert struct.unpack('<f',word)[0]==1.5309427976608276
  data=e.calloc(52);e.wr(data,word+b'\0'*48)
  for key in range(128): e.call(0x10038989c,d.dsp,2,key,data)
 e.call(0x1003898a4,d.dsp,2)
 d.set_param(36,cutoff,True)
 done=[];n=0
 taps={k:[] for k in ('generated_profile','pre_decimation','hb1_even','hb1_odd','hb2')}
 if force_dc:
  def force(*_):
   if n>=r['warm_frames']:e.sets(0,.25)
  e.uc.hook_add(UC_HOOK_CODE,force,begin=0x1003b6bb4,end=0x1003b6bb4)
 if factor==4:
  for name,(pc,reg) in dict(generated_profile=(0x1003b69a8,0),pre_decimation=(0x1003b6bb4,0),hb1_even=(0x1003b6c4c,17),hb1_odd=(0x1003b6c98,0),hb2=(0x1003b6d00,0)).items():
   def track(*_,name=name,reg=reg):
    if 24576<=n<37600:taps[name].append(e.s(reg))
   e.uc.hook_add(UC_HOOK_CODE,track,begin=pc,end=pc)
 events={d['frame']:(d['action'],m['oscSampler2']) for b,d,m in cfg['events'][:2]}
 for nframes in r['buffers']:
  frame=n-r['warm_frames']
  if frame in events:
   event,w=events[frame]
   if event=='on':e.call(0x100389c84,d.dsp,*w)
   else:e.call(0x100389c8c,d.dsp,w,0)
  done.append(d.render(nframes));n+=nframes
  if n>=r['warm_frames']+22050:break
 assert not e.unknown
 return np.concatenate(done,axis=1),taps
def rms(x):return np.linalg.norm(x.astype(float))/np.sqrt(x.size)
def db(a,b):return 20*np.log10(max(rms(a),1e-30)/max(rms(b),1e-30))
def suffix(z):
 # Conditional no-fit real-source terminal path: public centered fan-in trim,
 # external Apple HP10 evaluated separately below, no active Crusher, room, Unison.
 raw=p0['0'];gain=np.float32(1-.49881276637272776*float(p0['130'])**1.5)
 z=np.ascontiguousarray(z*gain,dtype=np.float32)
 so=Path('recovered-sound/main-route/suffix/settled_suffix.so')
 lib=C.CDLL(str(so.resolve()));f=lib.sj6_settled_postcrusher
 pt=np.ctypeslib.ndpointer(dtype=np.float32,ndim=2,flags='C_CONTIGUOUS')
 f.argtypes=[pt,pt,C.c_uint,C.POINTER(C.c_uint),C.c_uint,C.c_float]
 blocks=[];s=0
 for b in r['buffers']:
  blocks.append(b);s+=b
  if s==z.shape[1]:break
 assert s==z.shape[1]
 words=(C.c_uint*len(blocks))(*blocks)
 y=np.empty_like(z);assert f(z,y,s,words,len(words),raw)==1
 return y
def hp10(z):
 # Independent transparent candidate only (not authenticated Apple node).
 from math import tan,pi,sqrt
 k=tan(pi*10/44100)
 c=1/(1+sqrt(2)*k+k*k);a1=2*(k*k-1)*c;a2=(1-sqrt(2)*k+k*k)*c
 out=np.zeros_like(z)
 for i,s in enumerate(z):
  x1=x2=y1=y2=0.
  for j,x0 in enumerate(s):
   y=c*(float(x0)-2*x1+x2)-a1*y1-a2*y2
   x1,x2,y1,y2=float(x0),x1,y,y1;out[i,j]=y
 return out
res={}
for label,prof,factor,filt in [('actual_4x_profile',True,4,300.),('profile_not_loaded',False,4,300.),('one_x_rate_control',True,1,300.),('two_x_rate_control',True,2,300.),('held_lowcut_probe',True,4,211.95989990234375)]:
 leaf,taps=render(prof,factor,filt);y=suffix(leaf); yh=suffix(hp10(leaf))
 windows={}
 for name,i,j in [('attack',16384,18432),('held',24576,37634)]:
  target=x[:,i:j];q=y[:,i:j];qh=yh[:,i:j];f=leaf[:,i:j]
  rf=lambda a,b:float(np.sum(a.astype(float)*b.astype(float))/(rms(a)*rms(b)*a.size))
  windows[name]=dict(candidate_leaf_rms=rms(f),real_auv3_terminal_rms=rms(target),suffix_terminal_level_db=db(q,target),suffix_terminal_null_dbr=db(q-target,target),suffix_signed_corr=rf(q,target),external_model_hpf_terminal_level_db=db(qh,target),external_model_hpf_terminal_null_dbr=db(qh-target,target))
 res[label]={'profile_loaded':prof,'factor':factor,'receiver_cutoff_stimulus_hz':filt,'leaf_pcm_sha':hashlib.sha256(leaf.tobytes()).hexdigest(),'windows':windows,'taps':{k:{'count':len(a),'mean':float(np.mean(a)),'rms':rms(np.asarray(a))} for k,a in taps.items() if a}}
_,unity=render(True,4,300.,force_dc=True)
assert unity['generated_profile'], 'authenticated loaded profile renderer must execute'
unity_stats={k:{'count':len(a),'min':min(a),'max':max(a),'mean':float(np.mean(a))} for k,a in unity.items() if a}
assert all(max(abs(w['min']-.25),abs(w['max']-.25))<1e-6 for k,w in unity_stats.items() if k!='generated_profile')
print(json.dumps(dict(hb_actual_constant_input=unity_stats,scope='actual modern OSC2 leaf and authentic native output, distinct boundaries, guarded source-only downstream incl optional model HP, no fitted parameters or assumed whole match',original_sha=pin(BIN),captured_reference_sha=r['pcm_sha256'],case=res),indent=2))
