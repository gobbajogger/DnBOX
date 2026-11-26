from pathlib import Path
import sys,json,struct,hashlib
from unicorn import UC_HOOK_CODE
exec(compile(Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0],'source setup','exec'))
g=Path('/tmp/sj6-solidify-amp.Gue6N0')
m=json.loads((g/'native.json').read_text())['cases'][0]
phys=m['physical']; assert m['kind']=='1'
F=Path('sj6-cross-preset/normal-static.private-output/1-dry.private.txt');t=F.read_text().splitlines()
count=int(t[1]);raw={int(k):float(v) for k,v in (x.split() for x in t[2:2+count])}
rate=44100
def trial(layer):
 e=Emu(str(BIN));d=SamplerDSP(e,rate);d.load_placeholder()
 for i,x in cfg['sampler_control_writes']['oscSampler1']:d.set_param(i,x,True)
 for i,x in [(0,.808*(raw[1] if layer==0 else raw[3])),(7,0),(8,0),(9,0),(14,0),(15,phys[3]),(28,1),(29,1),(35,0),(36,20000.001953125),(39,0),(40,0),(41,1),(51,0),*zip((10,12,13),phys[:3])]:d.set_param(i,x,True)
 e.call(0x1003898c4,d.dsp,4)
 e.call(0x1003898a4,d.dsp,0)
 at=0
 state=None; event=[]; target=[]; buffers=[]
 v=None;idx=0
 def before(*_):
  nonlocal idx,v
  a=e.x(24); f=at+idx
  p=tuple(struct.unpack('<9d',e.rd(a+0x220,72)))
  y=e.s(0)
  if 82390<=f<83500: target.append([f,y,list(p)])
  v=a
 def right(*_):
  nonlocal idx
  a=e.x(24);f=at+idx
  q=tuple(struct.unpack('<9d',e.rd(a+0x290,72)))
  if 82390<=f<83500:
   assert len(target[-1])==3 and target[-1][0]==f
   target[-1]+= [e.s(0),list(q)]
  idx+=1
 def out(*_):
  f=at+idx-1
  if 82390<=f<83500:target[-1].append(e.s(0))
 for pc,fn in [(0x1003b78e0,before),(0x1003b7944,right),(0x1003b7d40,out)]:e.uc.hook_add(UC_HOOK_CODE,fn,begin=pc,end=pc)
 for x in t[3+count:]:
  b=list(map(int,x.split()));n=b[0]
  if at>83500:break
  for k,v0 in zip(b[2::2],b[3::2]):
   note=k-12-12*layer
   def hist():return e.rd(v+0x220,72).hex() if v else None
   h=hist()
   if v0:e.call(0x100389c84,d.dsp,note,87,doubles=(0.,))
   else:e.call(0x100389c8c,d.dsp,note,0)
   if at==82534:event.append([note,v0,h==hist(),hist()])
  idx=0; r=d.render(n)
  at+=n
 assert not e.unknown
 return {'layer':layer,'events':event,'receipts':target}
print(json.dumps({'binary':pin(BIN),'typed_input_sha':pin(F),'cases':[trial(k) for k in range(2)]}))
