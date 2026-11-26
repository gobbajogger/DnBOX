import json,hashlib,struct
from pathlib import Path
import numpy as np
from emu import Emu
from sampler import SamplerDSP
from unicorn import UC_HOOK_CODE
import paths,macho,sampler
from emu import Uc
from unicorn.arm64_const import UC_CPU_ARM64_MAX
import emu
def make_uc(*args,**kwargs):
 u=Uc(*args,**kwargs);u.ctl_set_cpu_model(UC_CPU_ARM64_MAX);return u
emu.Uc=make_uc
pin=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
BIN=Path(Path('/tmp/dnbox-newest-sj6-oracle-location').read_text().strip())/'arm64.private'
macho.EXPECTED_SHA='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
assert pin(BIN)==macho.EXPECTED_SHA
for k,v in dict(CREATE=0x100389848,INIT=0x100389e20,SETPARAM=0x100389e5c,PROCESS=0x10038a55c,LOADDATA=0x10038988c,BUILDKEYMAP=0x100389c54).items():setattr(sampler,k,v)

p=Path('recovered-sound/checkpoint/dnbox-sound-bench/sj6/brainwave-normal-sub/four-sources.json'); cfg=json.loads(p.read_text())
assert cfg['gain_param_zero']['sampler1']==.5873944163322449
def trial(who,lead,binary,variant='default'):
 e=Emu(str(binary));d=SamplerDSP(e,44100)
 e.call(0x1003898bc,d.dsp,1) if binary==BIN else e.w32(d.dsp+0x58,4)
 if who=='sampler1':
  # Valid deterministic continuous-loop marker; deliberately not original PCM.
  wave=np.tile(np.array([-.3,-.2,-.1,0,.1,.2,.3,.2,.1],dtype=np.float32),200)
  for lo,hi,nt in [(0,39,36),(40,49,48),(50,53,52),(54,127,55)]:
   d.load_sample(dict(note=nt,freq=440*2**((nt-69)/12),lokey=lo,hikey=hi,loop=True,loop_start=0.,loop_end=1799.,rate=44100.),wave)
  d.build_keymap()
 else:d.load_placeholder()
 for k,v in cfg['sampler_control_writes'][who]:d.set_param(k,v,True)
 if variant=='poly':d.set_param(41,0,True)
 clips=[];at=0; active=False; taps={k:[] for k in ('preL','preR','voiceL','voiceR','accL','accR','sumL','sumR')}
 addresses={'preL':(0x1003b78e0,0),'preR':(0x1003b7944,0),'voiceL':(0x1003b7934,1),'voiceR':(0x1003b7d3c,1),'accL':(0x1003b7934,0),'accR':(0x1003b7d3c,0),'sumL':(0x1003b7938,0),'sumR':(0x1003b7d40,0)}
 for k,(pc,r) in addresses.items():
  def tap(*_,k=k,r=r):
   if active:taps[k].append(e.s(r))
  e.uc.hook_add(UC_HOOK_CODE,tap,begin=pc,end=pc)
 def go(end):
  nonlocal at
  while at<end:
   n=min(end-at,128)
   if active:clips.append(d.render(n))
   else:d.render(n)
   at+=n
 play=0x100389c84 if binary==BIN else 0x10037d148
 stop=0x100389c8c if binary==BIN else 0x10037d150
 e.call(play,d.dsp,36,86)
 go(8192);active=True;go(33075-lead)
 if lead>0:e.call(stop,d.dsp,36,0)
 go(33075)
 for k in (48,52,55):e.call(play,d.dsp,k,86)
 go(34100);assert not e.unknown

 return np.concatenate(clips,axis=1),taps

def score(w):
 x,t=trial(w,11025,BIN)
 assert all(len(v)==x.shape[1] for v in t.values()),{k:len(v) for k,v in t.items()}
 def f(a):return np.asarray(a,dtype=np.float32)
 for i,ch in enumerate('LR'):
  acc,voice,total=(f(t[k+ch]) for k in ('acc','voice','sum'))
  assert not np.any(acc)
  assert (acc+voice).tobytes()==total.tobytes()==x[i].tobytes()
 windows={}
 for name,b,e in [('held',8192,21250),('pending',33075,33491),('after_commit',33491,34100)]:
  rms=lambda a:float(np.sqrt(np.mean(np.square(np.asarray(a[b-8192:e-8192],dtype=np.float64)))))
  windows[name]={k:rms(v) for k,v in t.items()}
 return {'frames':x.shape[1],'output_sha256':hashlib.sha256(x.tobytes()).hexdigest(),'rms':windows}
print(json.dumps(dict(scope='Exact compiled newest Mac AKSamplerDSP single-voice direct Normal accumulator and buffer, pre/post per-operator. Matched old projected controls, no loaded current Swift waveform profile or actual app mix.',newest_macho_sha256=pin(BIN),controls_sha256=pin(p),operators={w:score(w) for w in ('oscSampler1','oscSampler2','sampler1')}),indent=2))
