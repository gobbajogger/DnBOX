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
def trial(who,lead,variant='default'):
 e=Emu(str(BIN)); d=SamplerDSP(e,44100)
 e.call(0x1003898bc,d.dsp,1)
 if who=='sampler1':
  # Valid deterministic continuous-loop marker; deliberately not original PCM.
  wave=np.tile(np.array([-.3,-.2,-.1,0,.1,.2,.3,.2,.1],dtype=np.float32),200)
  for lo,hi,nt in [(0,39,36),(40,49,48),(50,53,52),(54,127,55)]:
   d.load_sample(dict(note=nt,freq=440*2**((nt-69)/12),lokey=lo,hikey=hi,loop=True,loop_start=0.,loop_end=1799.,rate=44100.),wave)
  d.build_keymap()
 else:d.load_placeholder()
 for k,v in cfg['sampler_control_writes'][who]:d.set_param(k,v,True)
 if variant=='poly':d.set_param(41,0,True)
 at=0; off=0; relative=None; rows=[]; hit=[]; swap=[]
 def cursor(v):return struct.unpack("<d",e.rd(v+0x10,8))[0]
 def zone(v,ofs):return e.r64(v+ofs)
 def prep(*_):
  nonlocal off,relative
  v=e.x(0)
  if not e.r32(v+0x340): return
  relative=at+off; off+=e.x(1)
  if relative>=33075-16:
   rows.append([relative,e.r32(v+0x340),e.rf(v+0x528),e.rf(v+0x5d0),e.rf(v+0x4cc),e.rf(v+0x4d0),e.rf(v+0x52c),e.rf(v+0x5e4),cursor(v),zone(v,0x28)==zone(v,0x5d8),e.r32(v+0x4c8),e.rf(v+0x530),struct.unpack('<d',e.rd(v+0x18,8))[0]])
 def commit(*_):
  v=e.x(19)
  if relative is not None and relative>=33075:hit.append([relative,e.r32(v+0x340),e.rf(v+0x528),e.rf(v+0x5d0),e.rf(v+0x4cc),e.rf(v+0x4d0)])
 def swapped(*_):
  v=e.x(19)
  if relative is not None and relative>=33075:swap.append([relative,cursor(v),zone(v,0x28)==zone(v,0x5d8),e.rf(v+0x52c)])
 for addr,cb in ((0x1003b4c54,prep),(0x1003b5120,commit),(0x1003b5160,swapped)):
  e.uc.hook_add(UC_HOOK_CODE,cb,begin=addr,end=addr)
 def go(end):
  nonlocal at,off
  while at<end:
   n=min(end-at,128);off=0;d.render(n);at+=n
 e.call(0x100389c84,d.dsp,36,86)
 go(33075-lead)
 if lead>0:e.call(0x100389c8c,d.dsp,36,0)
 go(33075)
 for k in (48,52,55):e.call(0x100389c84,d.dsp,k,86)
 go(34100);assert not e.unknown
 return dict(who=who,lead=lead,variant=variant,concept='same clock and fixed low-key velocity; marker Sub media independent; source-side state only',commit=hit,initialized_at_commit=swap,columns=['relative_frame','amp_stage','current_velocity','pending_velocity','requested_freq_4cc','held_freq_4d0','phase','amp_sample','sample_cursor','active_zone_equals_pending','key_4c8','saw_increment_530','sample_rate_ratio_18'],prep=rows)
cases=[trial(w,lead) for w in ('oscSampler1','sampler1') for lead in (0,11025,22050)]
old_result=Path("sj6-owner-causality/result.json")
old_cases=json.loads(old_result.read_text())["cases"]
assert len(cases)==6 and all(len(c["prep"])==66 for c in cases)
assert cases==old_cases, "Controlled original owner state should match"
print(json.dumps(dict(executable_sha256=pin(BIN),controls_sha256=pin(p),older_receipt_sha256=pin(old_result),old_new_prep_rows_exact=396,old_new_commit_rows_exact=6,old_new_initialization_rows_exact=6,scope="current Mac standalone compiled original receiver, older proven receiver controls projected exactly, marker audio/sample state; no app callback/Apple FX or rendered waveform asserted",cases=cases),indent=2))
