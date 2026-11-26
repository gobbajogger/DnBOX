from pathlib import Path
import tarfile,json,sys,struct
m=Path('sj6-current-faders/probe.py')
exec(compile(m.read_text().split('cases={}')[0],str(m),'exec'))
A=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
assert sha(A)=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(A) as tf:
 capture=json.load(tf.extractfile('./program-1-dry/capture.json'))
 assert capture['active']==capture['after']
def extract(a):
 return {int(k):f32(v) for k,v in a.items()}
a=capture['active']
raw=extract(a) if isinstance(a,dict) else {int(x['address']):f32(x['value']) for x in a}
assert [raw[x] for x in [72,73,74,75,105,89,136,137]]==[.5,0.,.5,.5,0.,0.,0.,0.]
bank=['globalbend','globalBendTuning','layer1BendTuning','layer2BendTuning','sampler1Detune','sampler2Detune','sampler1SemitoneOffset','sampler2SemitoneOffset']
for i,n in enumerate(bank):e.w64(0x1007e3b40+i*8,0x500+8*i)
for k in (24,25,26,27): e.w64(0x1007e3b00+k*8,0x600+(k-24)*8)
e.w64(0x1007e3bf8,0x698)
# Slots already verified for Main & Sub by prior actual property probes.
sampler2=w.new_obj('_TtC8AudioKit9AKSampler',size=0x500)
e.w64(0x1007e3ba0,0x580);e.w64(core+0x580,sampler2.addr)
for i,x in enumerate((('monoSynth1',0x1007e3bc0),('monoSynth2',0x1007e3bc8))):
 h=w.new_obj('_TtC8AudioKit9AKSampler',size=0x500)
 e.w64(x[1],0x588+8*i);e.w64(core+0x588+8*i,h.addr)
# Shadow external helper getters only. Record the real sampler-facing message;
# no emulation of Apple platform property closures needed for this receipt.
roles={e.r64(core+e.r64(g)):n for n,pid,pc,g,*_ in defs}
roles[sampler2.addr]='Sampler2'
bank_lfo=e.calloc(0x80)
e.w64(inst+0x108,bank_lfo)
e.w64(0x1007e3be8,0x648)
e.w64(core+0x648,w.native_array(list(roles.keys())))
trace=[]
for sel in ['setPitchBend:','setVibratoDepth:','setVibratoFrequency:','setPitchADSRSemitones:','pitchBend']:
 def watch(w,o,e,sel=sel):
  if sel=='pitchBend':e.setd(0,0.)
  else:trace.append([sel,roles.get(o.addr,'shim'),e.d(0)])
  return True
 w.sel_handlers[sel]=watch
def ck(k,pc,val):
 trace.clear(); e.call(pc,floats=(val,),regs={20:cond})
 return {'id':k,'val':val,'target':hex(pc),'writes':trace.copy()}
rows=[]
for k,pc in [(72,0x1000f4c44),(73,0x1000f4c58),(74,0x1000f4ba4),(75,0x1000f4bc0)]:
 rows.append(ck(k,pc,raw[k]))
e.w64(0x10082f550,0x658)
rows.append(ck(57,0x1000f48bc,raw[57]))
vib=[]
for r in [0.,.01,1.]:
 e.wf(bank_lfo+0x18,r)
 vib.append(ck('effective-zero-vib-depth-'+str(r),0x1000ee200,0.))
 e.wf(bank_lfo+0x18,r)
 vib.append(ck('positive-vib-depth-holdout-'+str(r),0x1000ee200,.5))
calls.clear();e.call(defs[0][2],floats=(raw[1],),regs={20:cond});f1=calls.copy()
calls.clear();e.call(defs[1][2],floats=(raw[3],),regs={20:cond});f2=calls.copy()
assert f1==[('OSC1',0,f32(.808*raw[1]))] and f2==[('OSC2',0,f32(.808*raw[3]))]
# Stop only for this scope at the exact Conductor A-tuning callee, after the
# authentic normalized conversion + explicit nearest rounding has executed.
from unicorn import UC_HOOK_CODE
a_ref=[]
def observe(*_):
 a_ref.append(e.d(0));e.uc.emu_stop()
e.uc.hook_add(UC_HOOK_CODE,observe,begin=0x10025e5d8,end=0x10025e5d8)
for a in (raw[206],.5,.75):e.call(0x1000f4bdc,floats=(a,),regs={20:cond})
assert a_ref==[440.,400.,500.]
assert not e.unknown
print(json.dumps({'raw_summary':{x:raw[x] for x in (57,72,73,74,75,89,105,106,136,137,206)},
 'rows':rows,'vibrato_gate_sweep':vib,'fader_writes':f1+f2,'a_reference_converted':a_ref,'source':sha(BIN)},indent=2))
