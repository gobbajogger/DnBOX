"""Original current authorized AKSampler live filter-state lifetime. No app/media shipped."""
import json
from pathlib import Path
SOURCE=Path('sj6-modern-owner/probe.py')
exec(compile(SOURCE.read_text().split('def trial(')[0],str(SOURCE),'exec'))

from collections import Counter
from unicorn.arm64_const import UC_ARM64_REG_S0,UC_ARM64_REG_S1
import struct
def one(note,owner,rate,events,parts):
 e=Emu(str(BIN)); d=SamplerDSP(e,rate); d.load_placeholder()
 for id,w in cfg['sampler_control_writes']['oscSampler'+str(owner)][:57]:d.set_param(id,w,True)
 for id,w in [(0,.808),(1,0),(2,0),(3,0),(8,0),(9,0),(10,.002),(12,.002),(13,1),(15,3.517968233344483),(20,1),(25,0),(28,1),(29,0),(32,1),(36,20000.001953125),(37,440),(38,owner+1),(39,0),(7,0),(40,0),(41,0),(42,0),(43,1),(44,0),(51,0)]:
  d.set_param(id,w,True)
 e.call(0x1003898c4,d.dsp,4);e.call(0x1003898a4,d.dsp,0)
 phase=None;audio=[]; switch=[]; gains=[]; notes=[];mark=0
 def cb_normal(*_):
  if phase is not None:
   v=e.x(24);audio.append((0,e.rd(v+0x248,32).hex()))
 def cb_ota(*_):
  if phase is not None:
   v=e.x(24);audio.append((1,e.rd(v+0x5f8,64).hex()))
 def cb_gain(*_):
  if phase is not None: gains.append(e.s(2))
 def cb_reset(*_):
  if phase is not None: switch.append([mark,e.x(1),e.x(0)])
 for addr,cb in [(0x1003b78e0,cb_normal),(0x1003b77e0,cb_ota),
                  (0x10037b3e0,cb_gain),(0x1003b41b0,cb_reset)]:
  e.uc.hook_add(UC_HOOK_CODE,cb,begin=addr,end=addr)
 for _ in range(32):d.render(512)
 e.call(0x100389c84,d.dsp,note,87,floats=(0.,))
 for _ in range(16):d.render(512)
 d.render(32)
 phase=True
 events=dict(events);mark=0;i=0
 while mark<1024:
  if mark in events:d.set_param(51,events[mark],True)
  pending=[t for t in events if t>mark]
  next_i=min(pending+[1024])
  count=min(parts[i%len(parts)],next_i-mark)
  d.render(count);mark+=count;i+=1
 assert len(audio)==1024 and not e.unknown
 edges=[(i,m,s) for i,(m,s) in enumerate(audio) if not i or m!=audio[i-1][0]]
 # 64 original voice reset calls per true commit, preserve first/last addresses only.
 reset_ranges=[]
 for x in switch:
  if reset_ranges and reset_ranges[-1][0:2]==x[:2]:reset_ranges[-1][2]+=1
  else:reset_ranges.append([x[0],x[1],1])
 return dict(rate=rate,events=events,parts=parts,edges=edges,fade_count=len(gains),first_gains=gains[:5],
             low_gains=sorted(gains)[:5],resets_by_callback=reset_ranges)
out=[one(57,1,rate,events,parts) for rate in (44100,48000) for events in
     [[(0,1)],[(0,1),(64,0)],[(0,1),(64,0),(96,1)],[(0,1),(640,0)]]
     for parts in ((16,),(13,37,5,91,29))]
print(json.dumps(dict(executable_sha256=pin(BIN),scope='same held source, DSP-only writes addr51 and no new note',trials=out),indent=2))
