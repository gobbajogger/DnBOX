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
def trial(who,factor):
 e=Emu(str(BIN)); d=SamplerDSP(e,44100)
 e.call(0x1003898bc,d.dsp,1)
 if who=='sampler1':
  y=np.tile(np.array([-.3,-.2,-.1,0,.1,.2,.3,.2,.1],dtype=np.float32),200)
  d.load_sample(dict(note=36,freq=440*2**((36-69)/12),lokey=0,hikey=127,loop=True,loop_start=0.,loop_end=1799.,rate=44100.),y)
  d.build_keymap()
 else:d.load_placeholder()
 for k,v in cfg['sampler_control_writes'][who]:d.set_param(k,v,True)
 authored=cfg['gain_param_zero'][who]; expected_fader=np.float32(authored*factor)
 d.set_param(0,expected_fader,True)
 measured=set(); injected=[]
 for pc,vel in ((0x1003b4d7c,0x5d0),(0x1003b4dc0,0x528)):
  def tap(*_,pc=pc,vel=vel):
   v=e.x(19); amp=e.s(15); velocity=e.rf(v+vel);out=e.s(0)
   assert np.float32(amp*velocity)==out
   assert amp==expected_fader
   measured.add((hex(pc),amp,velocity,out));injected.append(out)
  e.uc.hook_add(UC_HOOK_CODE,tap,begin=pc,end=pc)
 def note(k,v):e.call(0x100389c84,d.dsp,k,v)
 note(36,86);clips=[]
 for b in range(40):
  if b==32:
   for k,v in ((48,78),(52,82),(55,96)):note(k,v)
  clips.append(d.render(128))
 assert not e.unknown and len(injected)>100
 assert any(row[0]=='0x1003b4d7c' for row in measured)
 return np.concatenate(clips,axis=1),sorted(measured),len(injected)
def score(w):
 runs={s:trial(w,v) for s,v in (('authored',1.),('half',.5),('quarter',.25))}
 out={}
 for key,n in [('half',2),('quarter',4)]:
  x,g,c=runs[key];y,base,bc=runs['authored']
  assert x.shape==y.shape==(2,5120) and c==bc
  assert (x*np.float32(n)).tobytes()==y.tobytes()
  out[key]={'bit_exact_pcm_samples_against_authored':x.size,'actual_amp_prep_rows':g,'validated_prep_word_count':c}
 out['authored_actual_amp_prep_rows']=runs['authored'][1]
 return out
print(json.dumps(dict(scope='Latest true original compiled address0 sender to effective per-voice Gain. Controlled independent physical addresses/faders & pending velocities; marker audio. Not actual Swift panel-to-receiver curve.',newest_macho_sha256=pin(BIN),operators={w:score(w) for w in ('oscSampler1','oscSampler2','sampler1')}),indent=2))
