from pathlib import Path
from unicorn import UC_HOOK_CODE
p=Path('sj6-current-faders/probe.py')
exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'))
P=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
assert sha(P)=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
cue={}
with tarfile.open(P) as t:
 for arm in ('dry','authored','authored-repeat'):
  q=json.load(t.extractfile(f'./program-2-{arm}/capture.json'))
  assert q['active']==q['after']
  z={int(x['address']):f32(x['value']) for x in q['active']}
  cue[arm]=z[17]
assert all(x==0 for x in cue.values())
rage=w.new_obj('_TtC8AudioKit20AKDynaRageCompressor',size=0x500)
e.w64(inst+0xc0,rage.addr)
e.w64(0x1007d4de8,0x338)
e.w64(0x10080c110,0x300); e.w64(0x10080c118,0x308);e.w64(0x10080c120,0x310);e.w64(0x10080c128,0x318);e.w64(0x10080c138,0x328)
# Real class vtable slots, checked in original translated metadata; bind these to our minimal allocation.
assert [e.r64(v) for v in (0x1007b81e0,0x1007b81e8)]==[0x1003e25d4,0x1003e2674]
class_base=0x1007b8008
assert [e.r64(class_base+off) for off in (0x1d8,0x1e0)]==[0x1003e25d4,0x1003e2674]
mask=e.calloc(8);e.w64(mask,0xffffffffffffffff);e.w64(0x100757258,mask);e.w64(rage.addr,class_base)
rage.native_imps={'isStarted':0x1003e231c,'setDryWetMix:':0x1003e2258,'setIsStarted:':0x1003e23a0}
av=[]; vols={};out=[]
for j in (0,1):
 m=w.new_obj('_TtC8AudioKit7AKMixer',size=0x500);m.native_imps={'setVolume:':0x10047b638}
 a=w.new_obj('AVAudioMixerNode',size=0x500);av.append(a.addr)
 e.w64(0x10080f548,0x300);e.w64(0x10080f540,0x320);e.w64(m.addr+0x300,a.addr)
 e.w64(rage.addr+0x308+8*j,m.addr)
params={}; trace=[]; refresh=[]
def prop(w,o,e):
 assert o is rage
 return e.rf(rage.addr+0x300) if False else struct.unpack('<d',e.rd(rage.addr+0x300,8))[0]
def readmix(w,o,e):
 e.setd(0,prop(w,o,e)); return True
def volumes(w,o,e):
 j=av.index(o.addr);v=e.s(0);vols[j]=v;out.append([j,v]); return True
def setting(name):
 def wrap(w,o,e):
    assert o is rage
    params[name]=e.d(0);trace.append([name,e.d(0)]);return True
 return wrap
w.sel_handlers.update({'setOutputVolume:':volumes,'dryWetMix':readmix,
 'setThreshold:':setting('threshold_db'),'setMasterGain:':setting('wet_makeup_db')})
# This unrelated separate Reverb profile refresh has no local mixer setter;
# record when invoked, don't create Apple objects or infer Reverb/output PCM.
from unicorn.arm64_const import UC_ARM64_REG_LR,UC_ARM64_REG_PC
def wet_refresh(*_):
 refresh.append(True)
 e.uc.reg_write(UC_ARM64_REG_PC,e.uc.reg_read(UC_ARM64_REG_LR))
e.uc.hook_add(UC_HOOK_CODE,wet_refresh,begin=0x1000ee92c,end=0x1000ee92c)
def ctl(val,prior_started,prior_mix,cached):
 e.wr(rage.addr+0x318,bytes([prior_started]))
 e.wd(rage.addr+0x300,prior_mix);e.wd(rage.addr+0x328,cached)
 vols.update({0:f32(1-prior_mix),1:f32(prior_mix)})
 out.clear();trace.clear();refresh.clear()
 e.call(0x1000f1a5c,floats=(val,),regs={20:cond})
 return dict(raw=val,prior=[prior_started,prior_mix,cached],after_started=e.rd(rage.addr+0x318,1)[0],
     gave=[vols[0],vols[1]],written=out[:],processor_writes=trace[:],separate_room_refresh=len(refresh))
start=ctl(f32(.5),0,0,.625);stop=ctl(0.,1,.625,.125);quiet=ctl(0.,0,0,.625)
assert start['after_started']==1 and start['gave']==[.375,.625]
assert stop['after_started']==quiet['after_started']==0
assert stop['gave']==quiet['gave']==[1.,0.] and stop['written']==[[0,1.],[1,0.]] and quiet['written']==[]
assert stop['processor_writes']==quiet['processor_writes']==[['threshold_db',-20.],['wet_makeup_db',1.]]
assert not e.unknown,e.unknown
print(json.dumps(dict(current_bin=sha(BIN),capture=sha(P),actual_raw_live_17=cue,source_raw_sender='0x1000f1a5c',exact_vtable_stop='0x1003e2674',
 cases=[start,stop,quiet],scope='Executed original app public sender, original wrapper start/stop, actual Swift mix and real AKMixer setter; threshold/masterGain observation at actual wrapper endpoint; separate room refresh intercepted. No full AU sample or stalled wet-core behavior claim.'),indent=2))
