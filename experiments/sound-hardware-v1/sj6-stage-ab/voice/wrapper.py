"""Current actual processEvents closure -> real DSP stereo sampler output before Apple mixer."""
import hashlib,json,struct
from pathlib import Path
import numpy as np
import paths,macho,sampler,emu
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_CPU_ARM64_MAX
from emu import Uc,Emu
def uc(*a,**k):
 u=Uc(*a,**k);u.ctl_set_cpu_model(UC_CPU_ARM64_MAX);return u
emu.Uc=uc
BIN=Path('sj6-modern-private-recovery/arm64.private')
macho.EXPECTED_SHA='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
paths.BIN=str(BIN)
import objcmeta as o
base=dict(o.classes['AKAudioUnitBase'][1]);assert base['processEventsBlock:']==0x1003cce44
for k,v in dict(CREATE=0x100389848,INIT=0x100389e20,SETPARAM=0x100389e5c,PROCESS=0x10038a55c,LOADDATA=0x10038988c,BUILDKEYMAP=0x100389c54).items():setattr(sampler,k,v)
cfg=Path('recovered-sound/checkpoint/dnbox-sound-bench/sj6/brainwave-normal-sub/four-sources.json')
p=json.loads(cfg.read_text())['sampler_control_writes']
e=Emu(str(BIN))
def sampler_owner(name):
 d=sampler.SamplerDSP(e,44100);d.load_placeholder()
 for addr,value in p[name]:d.set_param(addr,value,True)
 e.call(0x1003898c4,d.dsp,4);e.call(0x1003898a4,d.dsp,2)
 return d
def wrapped(d):
 e.w64(d.dsp+0x18,0)
 closure,byref,t=e.calloc(64),e.calloc(64),e.calloc(64)
 e.w64(closure+32,byref);e.w64(byref+8,byref);e.w64(byref+24,d.dsp)
 vtable=e.r64(d.dsp)
 assert e.r64(vtable+16)==sampler.PROCESS
 def render(n):
  e.wr(d.bufL,bytes(4*n));e.wr(d.bufR,bytes(4*n))
  e.w32(d.abl+12,4*n);e.w32(d.abl+28,4*n)
  e.wd(t,float(d.now));e.call(0x1003ccecc,closure,0,d.abl,t,n,0)
  d.now+=n
  return np.array([np.frombuffer(e.rd(addr,n*4),'<f4') for addr in (d.bufL,d.bufR)])
 return render
def run(name,key):
 a,b=sampler_owner(name),sampler_owner(name)
 w=wrapped(b);x=[];y=[]
 seen=[]
 def hit(*_):
  if e.x(0)==b.dsp:seen.append([e.x(1),e.x(2),e.r64(b.dsp+0x20)])
 h=e.uc.hook_add(UC_HOOK_CODE,hit,begin=sampler.PROCESS,end=sampler.PROCESS)
 for n in (128,)*16:
  assert np.array_equal(a.render(n),w(n))
 for d in (a,b):e.call(0x100389c84,d.dsp,key,86,doubles=(0.,))
 for n in (17,31,63,128,23,128,7,128)*4:x.append(a.render(n));y.append(w(n))
 for d in (a,b):e.call(0x100389c8c,d.dsp,key,0)
 for n in (17,31,63,128,23,128,7,128)*4:x.append(a.render(n));y.append(w(n))
 e.uc.hook_del(h)
 x,y=[np.concatenate(z,axis=1) for z in (x,y)]
 assert np.array_equal(x.view('u4'),y.view('u4')) and np.max(abs(y))>.00001
 assert all(off==0 and ptr==b.abl for _,off,ptr in seen)
 return {'role':name,'key':key,'samples_per_channel':x.shape[1],'process_calls':len(seen),'n_set':sorted(set(z[0] for z in seen)),'peak':float(np.max(abs(y))),'sha':hashlib.sha256(y.tobytes()).hexdigest(),'audiobuffer_mismatches':0}
res=[run(n,k) for n,k in [('oscSampler1',36),('oscSampler2',36)]]
assert not e.unknown,e.unknown
print(json.dumps(dict(scope='Real current AKAudioUnitBase processEvents closure and virtual sampler; constructed closure storage, public AudioBufferList ABI; no Apple mixer, imported Mixer frames, or full preset scene claim.',source=macho.EXPECTED_SHA,cases=res),indent=2))
