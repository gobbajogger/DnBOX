from pathlib import Path
import hashlib,tarfile,json,struct
from emu import Emu,Uc
import emu,macho,sampler
from swiftrt import SwiftWorld
from unicorn.arm64_const import UC_CPU_ARM64_MAX
BIN=Path(Path('/tmp/dnbox-newest-sj6-oracle-location').read_text().strip())/'arm64.private'
macho.EXPECTED_SHA='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
sha=lambda a:hashlib.sha256(a.read_bytes()).hexdigest()
assert sha(BIN)==macho.EXPECTED_SHA
def mk(*a,**k):
 u=Uc(*a,**k);u.ctl_set_cpu_model(UC_CPU_ARM64_MAX);return u
emu.Uc=mk
for k,v in dict(CREATE=0x100389848,INIT=0x100389e20,SETPARAM=0x100389e5c,PROCESS=0x10038a55c).items():setattr(sampler,k,v)
e=Emu(str(BIN));w=SwiftWorld(e);e.deferred=[]
f32=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
path=Path('sj6-direct-stage/SJ6-Brainwave-Direct-AUv3-Master.tgz')
assert sha(path)=='82d0348669ebecb49245ee83fcb44c2a8576b9a2a5599e2f4591e671646bd5b8'
with tarfile.open(path) as t:
 d=json.load(t.extractfile('native-warm-brainwave-master-authored-main-only/capture.json'))
 assert d['parameters']==d['parameter_readback_after']
 # Restore the real authored Sub level; initial native readback is supplied.
 raw={x['address']:x['value'] for x in d['pre_mute_parameters']}
cond=e.calloc(0x700);inst=e.calloc(0x700);core=e.calloc(0x700)
e.w64(0x10082ea30,0x100);e.w64(cond+0x100,inst);e.w64(inst+0x20,core)
e.w64(0x1008145a0,0x300);e.w64(0x1008145b0,0x320);e.w64(0x10080cab0,0x380)
defs=[('OSC1','1',0x1000ee56c,0x1007e3ba8,.808,1.),('OSC2','3',0x1000ee590,0x1007e3bb0,.808,1.),('Sub','202',0x1000ee548,0x1007e3b98,.808,1.),('Noise','201',0x1000ee524,0x1007e3bb8,.707,2.5)]
who={};dsp={}
for i,(label,_id,_,off,_max,_) in enumerate(defs):
 o=w.new_obj('_TtC8AudioKit9AKSampler',size=0x500);o.native_imps={'setMasterVolume:':0x100556a34}
 a=w.new_obj('_TtC8AudioKit18AKSamplerAudioUnit',size=0x500)
 pos=0x400+i*0x18;e.w64(off,pos);e.w64(core+pos,o.addr)
 e.w64(o.addr+0x300,a.addr);e.wd(o.addr+0x320,float('nan'));dsp[a.addr]=(label,sampler.SamplerDSP(e,44100))
calls=[]
def endpoint(w,o,e):
 label,d=dsp[o.addr];addr=e.x(2);actual=e.s(0)
 assert addr==0
 calls.append((label,addr,actual))
 e.deferred.append(lambda:d.set_param(addr,actual,True))
 return True
w.sel_handlers['setParameterWithAddress:value:']=endpoint
cases={}
for label,pid,pc,off,top,power in defs:
 rows=[]
 for val in dict.fromkeys([.125,.25,.5,.75,.875,1.,0.,f32(raw[pid])]):
  calls.clear()
  e.call(pc,floats=(val,),regs={20:cond})
  expected=f32(top*pow(f32(val),power))
  assert calls==[(label,0,expected)],(label,val,calls,expected,w.msglog[-4:])
  rows.append([val,calls[0][2]])
 cases[label]={'authoring_parameter':pid,'authoring_readback':raw[pid],'expected_transfer_top':top,'expected_transfer_power':power,'native_sender':hex(pc),'core_ivar_offset_global':hex(off),'rows':rows}
assert not e.unknown
print(json.dumps(dict(newest_executable_sha=sha(BIN),source_capture_sha=sha(path),scope='Actual current Swift normalized closure -> actual current AKSampler masterVolume property -> instrument AU physical address0 message -> actual DSP setParameter, using minimal host layout; no AU render or inference.',cases=cases),indent=2))
