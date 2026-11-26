"""Actual original current sampler machine-code path; generated waveform only."""
from pathlib import Path
import struct,json
import numpy as np
exec(compile(Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0],'verified modern owner setup','exec'))
def capture(who,change):
 e=Emu(str(BIN));d=SamplerDSP(e,44100);d.load_placeholder()
 for k,v in cfg['sampler_control_writes']['oscSampler'+str(who)]:d.set_param(k,v,True)
 for k,v in [(0,.808),(1,0),(2,0),(3,0),(7,0),(8,0),(9,0),(10,.002),(11,0),(12,.002),(13,1),(14,0),(15,.002),(20,1),(25,0),(28,0),(29,0),(32,1),(36,20000.001953125),(37,440),(38,who+1),(39,0),(40,0),(41,0),(42,0),(43,1),(44,0),(51,0)]:
  d.set_param(k,v,True)
 e.call(0x1003898c4,d.dsp,4)
 # Admit actual generated descriptor ownership; synthetic scales distinguish
 # navigation, not commercial waveform data.
 for mode,scale in enumerate((.72,1.11,1.37,1.66)):
  q=e.calloc(52);e.wr(q,struct.pack("<f",scale)+bytes(48))
  for note in (65,71):e.call(0x10038989c,d.dsp,mode,note,q)
 pos=0; rows={}; pcm={}; nodes=set(); observed=[]; voice=None
 def tap(*_):
  nonlocal voice
  voice=e.x(24); nodes.add(voice)
  observed.append([e.s(0),e.rf(voice+0x52c),e.r32(voice+0x4c8)])
 e.uc.hook_add(UC_HOOK_CODE,tap,begin=0x1003b78e0,end=0x1003b78e0)
 def go(n):
  nonlocal pos
  observed.clear()
  for _ in range(n//128):d.render(128)
  pos+=n
  return np.array(observed)
 go(16384)
 e.call(0x1003898a4,d.dsp,0)
 e.call(0x100389c84,d.dsp,65,87)
 go(8192)
 for mode in (1,3,2,0):
  old=e.rf(voice+0x52c)
  if change:e.call(0x1003898a4,d.dsp,mode)
  written=e.rf(voice+0x52c)
  z=go(512)
  assert z.shape==(512,3) and np.all(z[:,2]==65)
  pcm[mode]=z[:,0].astype("<f4")
  rows[str(mode)]=dict(pre_phase=old,after_set_phase=written,first_after_prep=float(z[0,1]),last_after_render=e.rf(voice+0x52c),rms=float(np.sqrt(np.mean(z[:,0]**2))),held_digest=hashlib.sha256(z[:,0].astype("<f4").tobytes()).hexdigest())
 assert not e.unknown and len(nodes)==1
 # Apply one different mode, then force a genuinely new key while preserving
 # the single original held note. Check which owner hears that mode.
 if change:e.call(0x1003898a4,d.dsp,3)
 e.call(0x100389c84,d.dsp,71,87)
 z=go(2048)
 for note in (65,71):
  k=z[z[:,2]==note,0].astype('<f4')
  assert len(k)==2048,(who,note,len(k))
  pcm[note]=k
 return rows,pcm
out=[]
for w in [1,2]:
 c,b=capture(w,True),capture(w,False)
 def diff(x,y):
  return dict(equal=bool(np.array_equal(x,y)),relative_rms=float(np.linalg.norm(x-y)/np.linalg.norm(y)))
 out.append(dict(owner=w,phase=c[0],held={m:diff(c[1][m],b[1][m]) for m in (1,3,2,0)}, simultaneous={k:diff(c[1][k],b[1][k]) for k in (65,71)}))
print(json.dumps(dict(source=pin(BIN),scope='current actual compiled original generated sampler; real mode setter, fixed oscillator controls, no full app or downloaded table authority',owners=out),indent=2))
