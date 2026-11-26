from pathlib import Path
import numpy as np
from unicorn import UC_HOOK_CODE
def render(cue,raw):
 p=Path('sj6-modern-owner/probe.py')
 env={}
 exec(compile(p.read_text().split('def trial(')[0],'pinned current original source','exec'),env)
 Emu,SamplerDSP,BIN,cfg=[env[k] for k in ('Emu','SamplerDSP','BIN','cfg')]
 e=Emu(str(BIN));d=SamplerDSP(e,44100);d.load_placeholder()
 for k,v in cfg['sampler_control_writes']['oscSampler1'][:57]:d.set_param(k,v,True)
 f32=lambda x:float(np.float32(x))
 a=lambda p,l,h,b:f32(l+(h-l)*float(f32(raw[p]))**b)
 controls={0:f32(.808*raw[1]),1:0,2:0,3:0,7:0,8:0,9:0,
           10:a(36,.002,16,3.33),11:0,12:a(37,.002,24,3.35),13:a(38,0,1,2.5),14:0,15:a(39,.002,24,3.35),
           20:1,25:0,28:0,29:0,32:1,36:20000,37:440.,38:1,39:0,40:0,41:1,42:0,43:0,44:0,51:1}
 assert all(raw[k]==0 for k in (3,5,6,28,32,33,35,57,63,78,86,89,103,105,123,131,136,137,210,212))
 assert all(raw[k]==.5 for k in (72,74)) and np.float32(raw[82])==np.float32(2/3)
 assert np.float32(raw[206])==np.float32(.6) and raw[1]==raw[205]==raw[209]==1
 for k,v in controls.items():d.set_param(k,v,True)
 e.call(0x1003898c4,d.dsp,4);e.call(0x1003898a4,d.dsp,2)
 events={}
 for ev in cue['schedule']:
  events.setdefault(ev['frame'],[]).append((ev['note']-12,86 if ev['action']=='on' else 0))
 keys=(36,48,52,55); col={k:i for i,k in enumerate(keys)}
 frames=cue['frames']
 a=np.zeros((len(keys),frames),dtype='f4');g=a.copy();at_ota=a.copy()
 time=0; count=np.zeros((3,len(keys)),dtype='i8');valid=np.zeros_like(a,dtype='bool')
 dims=0
 def tap(stage):
  def f(*_):
   nonlocal dims
   k=e.r32(e.x(24)+0x4c8)
   assert k in col,(stage,k,time)
   i=col[k];x=count[stage,i];count[stage,i]+=1;at=time+x
   assert x<dims,(stage,i,x,dims)
   if stage==0:
    a[i,at]=e.s(0);g[i,at]=e.s(1)
   else:
    at_ota[i,at]=e.s(0);valid[i,at]=True
  return f
 for st,pc in enumerate((0x1003b6d94,0x1003b77e0)):
  e.uc.hook_add(UC_HOOK_CODE,tap(st),begin=pc,end=pc)
 for n in cue['blocks']:
  for k,v in events.get(time,[]):
   assert v in (0,86)
   e.call(0x100389c84 if v else 0x100389c8c,d.dsp,k,v,floats=(0.,))
  dims=n;count[:]=0;d.render(n)
  assert np.array_equal(count[0],count[1]), (time,n,count)
  time+=n
 assert time==frames and not e.unknown
 assert np.array_equal(np.float32(a*g),at_ota)
 assert np.all(g[valid]==np.float32(.3400000035762787)) # this is fixed wave scale, not AMP
 for i,k in enumerate(keys):
  positions=np.flatnonzero(valid[i])
  assert len(positions)>100 and np.all(np.diff(positions)==1),(k,len(positions))
 return a,g,at_ota,valid,keys,controls,env['pin'](BIN)
