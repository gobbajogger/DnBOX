from pathlib import Path
import hashlib,json,struct,sys,collections
import numpy as np
from unicorn import UC_HOOK_CODE
exec(compile(Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0],'source setup','exec'))
base=Path(sys.argv[1]); sender=json.loads((base/'senders.json').read_text())
assert sender['source']==pin(BIN)
f32=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
seq={16384:[(36,87)],38434:[(36,0)],49459:[(48,87),(52,87),(55,87)],82534:[(48,0),(52,0),(55,0)]}
points=sorted(seq)+[83550]
def arm(layer,immediate):
 e=Emu(str(BIN));d=SamplerDSP(e,44100);d.load_placeholder()
 for p,k in cfg['sampler_control_writes']['oscSampler1']:d.set_param(p,k,True)
 for p,k in ((28,1),(29,1),(35,0),(51,0),(36,20000.001953125),(7,0),(8,0),(39,0),(40,0),(41,1),(37,440),(26,0),(2,0),(3,.75),(4,0),(5,.75),(10,.01041649),(12,.002004788),(13,1),(15,.012719513),(11,0),(14,0)):
  d.set_param(p,k,True)
 e.call(0x1003898c4,d.dsp,4);e.call(0x1003898a4,d.dsp,0)
 d.set_param(0,sender['fader_writes'][layer][2],immediate)
 d.set_param(1,0 if layer==0 else -12,immediate)
 idx=0;v=None;r=[];arm_words=[];now=0
 def ev(*_):
  nonlocal idx
  v=e.x(0); i=e.x(1)
  if not e.r32(v+0x340):return
  frame=now+idx; idx+=i
  if (17000<=frame<37500 or 50091<=frame<81000 or 82534<=frame<83095):
   # Request and oscillator-used increments; ramper fields {target, current, last} are
   # retained as observed binary words, not interpreted beyond static equality.
   r.append((frame,e.r32(v+0x4c8),e.rf(v+0x4cc),e.rf(v+0x530),
      e.r32(v+0x340),e.rd(d.dsp+0x198,16).hex(),e.rd(d.dsp+0x1c8,16).hex()))
 e.uc.hook_add(UC_HOOK_CODE,ev,begin=0x1003b4c54,end=0x1003b4c54)
 while now<83550:
  for n,k in seq.get(now,[]):
   if k:e.call(0x100389c84,d.dsp,n,k,doubles=(0.,))
   else:e.call(0x100389c8c,d.dsp,n,0)
  cap=next(f for f in points if f>now);n=min(cap-now,128)
  idx=0; pcm=d.render(n);arm_words.append(pcm)
  now+=n
 assert not e.unknown
 x=np.ascontiguousarray(np.concatenate(arm_words,1))
 windows={}
 for k,s,t in [('first',17000,37500),('chord',50091,81000),('release',82534,83095)]:
  v=[q for q in r if s<=q[0]<t]
  windows[k]=dict(rows=len(v),keys=sorted(set(q[1] for q in v)),request=sorted(set(q[2] for q in v)),
        inc_hz=sorted(set(44100*q[3] for q in v)),stages=sorted(set(q[4] for q in v)),
        fader_words=sorted(set(q[5] for q in v)),bend_words=sorted(set(q[6] for q in v)))
 return x,windows
o={}
for l in range(2):
 a,aw=arm(l,True);b,bw=arm(l,False)
 df=np.flatnonzero(np.any(a!=b,axis=0)); note={'same':len(df)==0,'first':int(df[0]) if len(df) else None,'last':int(df[-1]) if len(df) else None,'max_abs':float(np.max(abs(a-b)))}
 o[l]={'reached_steady':aw,'live_state':bw,'difference':note,'same_recv_PCM_SHA':hashlib.sha256(a.tobytes()).hexdigest(),
       'scope':'Receiver static current control words with captured fader/transpose sends; no authentic Vintage RNG, full current control callback or Apple effects.'}
print(json.dumps(o,indent=2))
