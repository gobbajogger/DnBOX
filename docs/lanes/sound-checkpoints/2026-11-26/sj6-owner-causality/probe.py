import json,hashlib,struct
from pathlib import Path
import numpy as np
from emu import Emu
from sampler import SamplerDSP
from unicorn import UC_HOOK_CODE
import paths
pin=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
assert pin(paths.BIN)=='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
p=Path('recovered-sound/checkpoint/dnbox-sound-bench/sj6/brainwave-normal-sub/four-sources.json'); cfg=json.loads(p.read_text())
assert cfg['gain_param_zero']['sampler1']==.5873944163322449
def trial(who,lead,variant='default'):
 e=Emu(); d=SamplerDSP(e,44100)
 e.w32(d.dsp+0x58,4)
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
 for addr,cb in ((0x1003a5664,prep),(0x1003a5b30,commit),(0x1003a5b70,swapped)):
  e.uc.hook_add(UC_HOOK_CODE,cb,begin=addr,end=addr)
 def go(end):
  nonlocal at,off
  while at<end:
   n=min(end-at,128);off=0;d.render(n);at+=n
 e.call(0x10037d148,d.dsp,36,86)
 go(33075-lead)
 if lead>0:e.call(0x10037d150,d.dsp,36,0)
 go(33075)
 for k in (48,52,55):e.call(0x10037d148,d.dsp,k,86)
 go(34100);assert not e.unknown
 return dict(who=who,lead=lead,variant=variant,concept='same clock and fixed low-key velocity; marker Sub media independent; source-side state only',commit=hit,initialized_at_commit=swap,columns=['relative_frame','amp_stage','current_velocity','pending_velocity','requested_freq_4cc','held_freq_4d0','phase','amp_sample','sample_cursor','active_zone_equals_pending','key_4c8','saw_increment_530','sample_rate_ratio_18'],prep=rows)
cases=[trial(w,lead) for w in ('oscSampler1','sampler1') for lead in (0,11025,22050)]
for main,sub in zip(cases[:3],cases[3:]):
 assert main["commit"]==sub["commit"]
 assert main["commit"][0][0]==33491
 assert [r[:6]+r[7:8] for r in main["prep"]]==[r[:6]+r[7:8] for r in sub["prep"]]
 assert sub["initialized_at_commit"]==[[33491,0.0,True,0.0]]
 pending=[r for r in sub["prep"] if 33075<=r[0]<=33491]
 assert len(pending)>10 and not any(r[9] for r in pending)
 assert len({r[8] for r in pending})>10
 assert all(r[10]==55 for r in pending)
 assert len({r[12] for r in pending})==1
 assert pending[-1][12]>next(r[12] for r in sub["prep"] if r[0]==33507)
 pp=[r for r in main["prep"] if 33091<=r[0]<=33491]
 assert 0<pp[0][11]<pp[-1][11]<pp[0][4]/44100
 assert abs((pp[1][6]-pp[0][6])/16-pp[1][11])<1e-7
 assert next(r for r in sub["prep"] if r[0]==33507)[9] is True
print(json.dumps(dict(controls_sha256=pin(p),cases=cases),indent=2))
