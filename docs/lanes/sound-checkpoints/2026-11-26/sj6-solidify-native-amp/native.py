from pathlib import Path
import sys,os,hashlib,json, math,struct
from unicorn import UC_HOOK_CODE
exec(compile(Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0],'source setup','exec'))
f32=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
def trial(kind):
 source=Path('sj6-cross-preset/normal-static.private-output/'+kind+'-dry.private.txt')
 data=source.read_text().splitlines();n=int(data[1]); raw={int(a):f32(float(b)) for a,b in (s.split() for s in data[2:2+n])}
 start=[raw[k] for k in (36,37,38,39)]
 fn=lambda u,l,h,e:f32(l+(h-l)*u**e)
 phys=[fn(u,*z) for u,z in zip(start,[(.002,16,3.33),(.002,24,3.35),(0,1,2.5),(.002,24,3.35)])]
 e=Emu(str(BIN));d=SamplerDSP(e,44100);d.load_placeholder()
 for p,v in cfg['sampler_control_writes']['oscSampler1']:d.set_param(p,v,True)
 for p,v in [(0,.808),(28,1),(29,1),(35,0),(51,0),(36,20000.),(1,0),(2,0),(3,0),(7,0),(8,0),(40,0),(41,1),(11,0),(14,0),*zip((10,12,13,15),phys)]:
  d.set_param(p,v,True)
 e.call(0x1003898c4,d.dsp,4);e.call(0x1003898a4,d.dsp,0)
 tape=[]; rows=[]; at=0; offset=0
 def prep(*_):
  nonlocal offset
  v=e.x(0)
  st=e.r32(v+0x340);n=e.x(1)
  if not st:return
  rows.append([at+offset,n,st,e.rf(v+0x5e4)])
  offset+=n
 e.uc.hook_add(UC_HOOK_CODE,prep,begin=0x1003b4c54,end=0x1003b4c54)
 for s in data[3+n:]:
  p=list(map(int,s.split()));frames=p[0]
  for key,v in zip(p[2::2],p[3::2]):
   key+=[-12,0,12][int(raw[86]*2)]
   v=(87 if raw[205] else v) if v else 0
   tape.append([at,key,v])
   if v:e.call(0x100389c84,d.dsp,key,v,doubles=(0.,))
   else:e.call(0x100389c8c,d.dsp,key,0)
  offset=0; d.render(frames);at+=frames
 assert not e.unknown
 return dict(kind=kind,input=pin(source),raw=start,physical=phys,events=tape,prep=rows)
print(json.dumps({'binary_sha':pin(BIN),'cases':[trial(n) for n in ('1','0','2')]},indent=2))
