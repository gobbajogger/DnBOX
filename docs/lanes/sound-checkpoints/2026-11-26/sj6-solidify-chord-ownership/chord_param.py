from pathlib import Path
import math, json, sys, hashlib
from unicorn import UC_HOOK_CODE
exec(compile(Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0],'source setup','exec'))
F=Path('sj6-cross-preset/normal-static.private-output/1-dry.private.txt')
text=F.read_text().splitlines(); num=int(text[1]); raw={int(k):float(v) for k,v in [w.split() for w in text[2:2+num]]}
schedule=[]
frames=0
for line in text[3+num:]:
 x=list(map(int,line.split())); assert x[1]*2+2==len(x)
 ev=list(zip(x[2::2],x[3::2])); schedule.append((frames,x[0],[(key-12, 87 if v else 0) for key,v in ev])); frames+=x[0]
 assert raw[86]==0 and raw[205]==1
assert frames==148684
def run(lane,legato,long_release,sequence,dial,target):
 e=Emu(str(BIN));d=SamplerDSP(e,44100);d.load_placeholder()
 for p,v in cfg['sampler_control_writes']['oscSampler'+str(lane)]:d.set_param(p,v,True)
 # Deliberately controlled receiver profile: transport, owner modes and exact
 # timings from the saved cue. Envelope reset vs live long release is swept;
 # no claim original normalized ADSR setters have been projected here.
 for p,v in [(0,.808),(28,1),(29,legato),(25,0),(51,0),(36,20000.),(7,0),(8,0),(40,0),(1,0),(2,-12*(lane-1)),(3,0),(37,440.),(15,1.9057256 if long_release else .02),(41,1),(target,dial)]:
  d.set_param(p,v,True)
 e.call(0x1003898c4,d.dsp,4);e.call(0x1003898a4,d.dsp,0)
 slot=[None]; rel=[]; rows=[]; commits=[]; audio=[];at=0;within=[0]
 def state(v):return [e.r32(v+0x4c8),e.rf(v+0x4cc),e.rf(v+0x4d0),e.rf(v+0x530),e.rf(v+0x52c),e.r32(v+0x340),e.rf(v+0x5e4),e.rf(v+0x528),e.rf(v+0x5d0)]
 def prep(*_):
  v=e.x(0)
  if not e.r32(v+0x340):return
  slot[0]=v; f=at+within[0]; within[0]+=e.x(1)
  if any(k-128<=f<k+800 for k in [16384,38434,49459,82534]):rel.append([f,*state(v)])
 def commit(*_):
  commits.append([at,*state(e.x(19))])
 for loc,cb in [(0x1003b4c54,prep),(0x1003b5120,commit)]:
  e.uc.hook_add(UC_HOOK_CODE,cb,begin=loc,end=loc)
 for at,n,ev in schedule:
  if at>52000:break
  if at==49459:ev=list(zip(sequence,[87]*len(sequence)))
  for key,v in ev:
   before=state(slot[0]) if slot[0] else None
   if v:e.call(0x100389c84,d.dsp,key,v,doubles=(0.,))
   else:e.call(0x100389c8c,d.dsp,key,0)
   after=state(slot[0]) if slot[0] else None
   rows.append([at,key,v,before,after])
  within[0]=0;d.render(n)
 assert not e.unknown
 return dict(lane=lane,legato=legato,long_release=long_release,sequence=sequence,dial=dial,target=target,events=rows,ticks=rel,commits=commits)
res=[run(1,1,0,seq,dial,target) for seq in [(48,52,55)] for target in [26,34,35,40,41,42] for dial in [0.,1.]]
print(json.dumps(dict(binary_sha256=pin(BIN),input_sha=pin(F),stage_cols=['key','requested','current','increment','phase','amp_stage','amp','vel','pending_vel'],presses=res),indent=2))
