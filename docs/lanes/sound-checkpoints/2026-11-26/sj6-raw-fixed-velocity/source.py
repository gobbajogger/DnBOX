"""Actual current default velocity precision, physical unit and prefilter taps."""
from pathlib import Path
exec(compile(Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0],'current sampler setup','exec'))
def taper():
 e=Emu(str(BIN)); cond=e.calloc(0x900)
 from unicorn.arm64_const import UC_ARM64_REG_X26
 field=0x10082f0f8; slot=0x700
 e.w64(field,slot)
 e.uc.hook_add(UC_HOOK_CODE,lambda *_:e.uc.emu_stop(),begin=0x1002683b4,end=0x1002683b4)
 e.call(0x1002683a4,regs={24:cond})
 assert e.rd(cond+slot,8)==struct.pack('<d',1.)
 out=[]; velocity=[]
 def stop(*_):velocity.append(e.x(26)); e.uc.emu_stop()
 e.uc.hook_add(UC_HOOK_CODE,stop,begin=0x1002f8bdc,end=0x1002f8bdc)
 for nominal in (1,31,55,80,86,87,88,96,100,127):
    e.call(0x1002f8b68,regs={19:cond,26:nominal})
    inferred=int(float(np.float32(nominal)/np.float32(127))*127.)
    assert velocity[-1]==inferred
    out.append(dict(input=nominal,actual_output=velocity[-1]))
 assert velocity[-5]==86
 assert e.call(0x1002b9ef0,36,86)==86
 assert [e.call(0x1002b9ef0,k,86) for k in (48,52,55)]==[86]*3
 assert not e.unknown
 return out
def run(vel):
 e=Emu(str(BIN)); d=SamplerDSP(e,44100);d.load_placeholder()
 # Scoped real current receiver projection for Raw OSC1 and velocity
 # ablation. Whole per-layer source bank/terminal Apple graph not asserted.
 for p,v in cfg['sampler_control_writes']['oscSampler1'][:57]:d.set_param(p,v,True)
 for p,v in ((0,.808),(1,0),(2,0),(3,0),(8,0),(9,0),(10,.002),(12,.002),(13,1),(15,.01272),(20,1),(25,0),(28,0),(29,0),(32,1),(36,20000),(37,440),(38,1),(39,0),(40,0),(41,1),(42,0),(43,0),(44,0),(51,1)):
    d.set_param(p,v,True)
 e.call(0x1003898c4,d.dsp,4); e.call(0x1003898a4,d.dsp,2)
 x=[]; now=0; pre=[]
 def stage(*_):
    if 16384<=now<38434:pre.append(e.s(0))
 e.uc.hook_add(UC_HOOK_CODE,stage,begin=0x1003b77e0,end=0x1003b77e0)
 for end,k,on in ((16384,36,1),(38434,36,0),(39200,0,-1)):
   while now<end:
      n=min(128,end-now);x.append(d.render(n)[0]);now+=n
   if on>=0:
      if on:e.call(0x100389c84,d.dsp,k,vel,floats=(0.,))
      else:e.call(0x100389c8c,d.dsp,k,0)
 assert len(pre)==22050 and not e.unknown,(len(pre),e.unknown)
 return np.array(pre,dtype='f4'),np.concatenate(x)
table=taper(); small,b=run(86); big,a=run(87)
y={}
def value(v):
 return float(np.linalg.norm(v.astype('f8')))
for k,x,z in [('prefilter_attack',small[:4410],big[:4410]),('prefilter_hold',small[4410:],big[4410:]),('post_voice_hold',b[20794:38434],a[20794:38434]),('post_voice_release',b[38434:],a[38434:])]:
   left=value(x);right=value(z);assert left>0 and right>0
   rr=float(20*np.log10(left/right))
   q=float(20*np.log10(value(x-z*np.float32(86./87))/left))
   y[k]={'velocity_86_vs_87_db':rr,'signed_after_exact_velocity_ratio_dbr':q}
print(json.dumps(dict(current_executable=pin(BIN),default_conductor_taper=1.,nominal_to_effective=table,physical_raw_ranged_keys=[36,48,52,55],source=y,
    scope='True executable .2683a4 constructor default and .2f8b68 note taper, real generated current saw owner physical setting projection, not full saved native input/event conductor'),indent=2))
