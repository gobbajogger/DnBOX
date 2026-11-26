from pathlib import Path
import json,struct,collections
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_SP
exec(compile(Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0],'source setup','exec'))
def run(mode,lane,fader,raw_sign):
 e=Emu(str(BIN));d=SamplerDSP(e,44100)
 e.call(0x1003898bc,d.dsp,1)
 n=65536; key=36; hz=440*2**((key-69)/12)
 wave=(raw_sign*.5*np.sin(np.arange(n)*np.pi*hz/44100)).astype('f4')
 d.load_sample(dict(note=key,freq=hz,lokey=0,hikey=127,loop=False,loop_start=0.,loop_end=n-1.,start=0.,end=n-1.,rate=44100.),wave)
 d.build_keymap()
 for i,v in cfg['sampler_control_writes']['sampler1']:d.set_param(i,v,True)
 for i,v in [(0,fader),(7,0),(8,0),(9,0),(14,0),(15,.012),(35,0),(36,20000.),(39,0),(40,0),(41,1),(51,int(mode=='OTA'))]:
  d.set_param(i,v,True)
 e.call(0x100389c84,d.dsp,36,86,doubles=(0.,))
 phase=0; taps=collections.defaultdict(list); c=collections.Counter()
 def tap(key,pc,r):
  def record(*_):
   if phase==2:taps[key].append(e.s(r))
  e.uc.hook_add(UC_HOOK_CODE,record,begin=pc,end=pc)
 addrs=dict(generated=(0x1003b69a8,0),post_hb=(0x1003b6d00,0),voice_input=(0x1003b7734,0),voice_gain=(0x1003b7734,12),voice_scaled=(0x1003b7738,0),normal_left=(0x1003b7c14,1),right_out=(0x1003b7d40,0))
 for k,(pc,r) in addrs.items():tap(k,pc,r)
 def left(*_):
  if phase==2:taps['left_out'].append(e.rf(e.x(22)))
 e.uc.hook_add(UC_HOOK_CODE,left,begin=0x1003b7d44,end=0x1003b7d44)
 for name,pc,offset in [('ota_l_in',0x1003b7870,0x68),('ota_r_in',0x1003b7a80,0x64)]:
  def record(*_,name=name,offset=offset):
   if phase==2:taps[name].append(e.rf(e.uc.reg_read(UC_ARM64_REG_SP)+offset))
  e.uc.hook_add(UC_HOOK_CODE,record,begin=pc,end=pc)
 tap('normal_in',0x1003b78e0,0)
 tap('ota_l_filtered',0x1003b7af0,6)
 tap('ota_l_in',0x1003b77e0,0);tap('ota_r_in',0x1003b7ccc,0);tap('ota_l_filtered',0x1003b77e4,0);tap('ota_r_filtered',0x1003b7cd0,0);tap('ota_l_fader',0x1003b77e4,15);tap('ota_r_fader',0x1003b7ce4,8)
 def flip(*_):
  if raw_sign!=1 and phase:e.sets(0,raw_sign*e.s(0))
 e.uc.hook_add(UC_HOOK_CODE,flip,begin=0x1003b6bb4,end=0x1003b6bb4)
 for _ in range(12):d.render(256)
 phase=1
 for _ in range(8):d.render(256)
 phase=2; out=np.concatenate([d.render(128) for _ in range(4)],1)
 assert not e.unknown
 return {k:np.asarray(v,dtype=np.float32) for k,v in taps.items()},out
rows=[];sets={}
for mode in ['Normal','OTA']:
 for lane,fader in [(1,.808),(3,.404)]:
  for sign in [1,-1]:
   sets[mode,lane,sign]=run(mode,1 if lane==3 else lane,fader,sign)
def describe(x):
 y=x.astype(float);return {'n':int(x.size),'mean':float(np.mean(y)),'rms':float(np.sqrt(np.mean(y*y))),'peak':float(np.max(abs(y)))}
def compare(a,b):
 return float(np.max(abs(a-b))) if a.shape==b.shape and a.size else None
for (m,l,s),(t,o) in sets.items():
 u=sets[m,l,-s]
 rows.append(dict(mode=m,lane=l,invert=s,taps={k:describe(v) for k,v in t.items()},output=[describe(z) for z in o],
  right_out_exact=bool(np.array_equal(t['right_out'],o[1])),
  left_out_exact=bool(np.array_equal(t['left_out'],o[0])), half_amplitude_linear=(compare(o,np.float32(.5)*sets[m,1,s][1]) if l==3 else None),
  scaling_exact=(bool(np.array_equal(t['voice_scaled'],np.float32(t['voice_input']*t['voice_gain']))) if 'voice_scaled' in t else None),
  associated_inputs_at_half={k:compare(v,sets[m,1,s][0][k]) for k,v in t.items() if (k.endswith('_in') or k=='ota_l_filtered')} if l==3 else None,
  max_sign_error_out=compare(o,-u[1]),max_sign_error_before=compare(t.get('voice_input',o[:,:0]),-u[0].get('voice_input',o[:,:0]))))
print(json.dumps(dict(source=pin(BIN),observations=rows),indent=2))
