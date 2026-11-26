from pathlib import Path
import json,struct,hashlib,itertools
import numpy as np
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_LR,UC_ARM64_REG_PC
from swiftrt import SwiftWorld
exec(compile(Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0],'source setup','exec'))
hz=44100
cue={16384:[(36,87)],38434:[(36,0)],49459:[(48,87),(52,87),(55,87)]}
def take(lane, last, ons=(87,87,87), hard=False):
 e=Emu(str(BIN));w=SwiftWorld(e);d=SamplerDSP(e,hz);d.load_placeholder()
 for id,val in cfg['sampler_control_writes']['oscSampler1']:d.set_param(id,val,True)
 gain=(.808,.36819931864738464)[lane]
 for id,val in [(0,gain),(1,-12*lane),(28,1),(29,1),(35,0),(51,0),(36,20000.001953125),(7,0),(8,0),(40,0),(41,1),(10,.01041649),(12,.002004788),(13,1),(15,.012719513),(11,0),(14,0)]:
  d.set_param(id,val,True)
 e.call(0x1003898c4,d.dsp,4);e.call(0x1003898a4,d.dsp,0)
 sampler=w.new_obj('_TtC8AudioKit9AKSampler',size=0x500)
 au=w.new_obj('_TtC8AudioKit18AKSamplerAudioUnit',size=0x500)
 e.w64(0x1008145a0,0x300);e.w64(sampler.addr+0x300,au.addr)
 # Only machine-level accessor of the fake AudioUnit is replaced. The actual
 # ordinary/hard wrapper, branch selectors, note ownership and DSP render execute.
 def handle(*_):
  assert e.x(0)==au.addr
  e.setx(0,d.dsp);e.uc.reg_write(UC_ARM64_REG_PC,e.uc.reg_read(UC_ARM64_REG_LR))
 e.uc.hook_add(UC_HOOK_CODE,handle,begin=0x1005fd3c0,end=0x1005fd3c0)
 offsets=[0x528,0x5d0,0x5e4]
 v=None; frame=0;trace=[];branch=[]
 def snap():
  return [e.r32(v+0x4c8),e.r32(v+0x340),*[e.rf(v+x) for x in offsets]] if v else None
 def prep(*_):
  nonlocal v
  g=e.x(0)
  if e.r32(g+0x340):v=g
 def hit(addr):
  def r(*_):
   if frame>=82534:branch.append([hex(addr),e.x(1),e.x(2)])
  e.uc.hook_add(UC_HOOK_CODE,r,begin=addr,end=addr)
 e.uc.hook_add(UC_HOOK_CODE,prep,begin=0x1003b4c54,end=0x1003b4c54)
 for pc in (0x100389c8c,0x10037a6cc,0x10037a800,0x10037a780,0x10037a890):hit(pc)
 note_on=cue|{49459:list(zip((48,52,55),ons)),82534:[(i,0) for i in last]}
 marks=sorted(note_on)+[83558];v=None;out=[]
 while frame<83558:
  for i,k in note_on.get(frame,[]):
   b=snap()
   if k:e.call(0x100389c84,d.dsp,i,k,doubles=(0.,))
   else:e.call(0x100560c2c if hard and frame==82534 else 0x100560c1c,i,regs={20:sampler.addr})
   if frame>=82534:trace.append([i,b,snap()])
  n=min(128,next(m for m in marks if m>frame)-frame)
  b=d.render(n)
  if frame>=82534:out.append(b)
  frame+=n
 assert not e.unknown
 x=np.concatenate(out,1)
 return x,dict(event_trace=trace,accepted_branch=branch,sha=pin_bytes(x.tobytes()),rms=float(np.sqrt(np.mean(x.astype(float)**2))))
def pin_bytes(x):return hashlib.sha256(x).hexdigest()
cases={}
for lane in (0,1):
 for label,order,vel,hard in [
  ('authored',(48,52,55),(87,87,87),False),
  ('swap_discards',(52,48,55),(87,87,87),False),
  ('owner_first_negative',(55,52,48),(87,87,87),False),
  ('only_owner_negative',(55,),(87,87,87),False),
  ('nonfixed_positive',(48,52,55),(96,80,88),False),
  ('hard_negative',(48,52,55),(87,87,87),True)]:
  a,r=take(lane,order,vel,hard)
  if label=='authored':ref=a
  r['same_output_as_authored']=bool(np.array_equal(a,ref))
  cases[str(lane)+'/'+label]=r
print(json.dumps(dict(source=pin(BIN),state=['key','stage','effective_velocity','pending_velocity','amp'],cases=cases),indent=2))
