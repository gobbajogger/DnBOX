"""Actual current cutoff and note/cursor-independent physical receiver fanout."""
from pathlib import Path
p=Path('sj6-current-faders/probe.py')
exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'))
e.w64(0x10082f448,0x580);e.w64(0x1007e3be8,0x658)
e.w64(0x100814770,0x330);e.w64(0x10080cc28,0x390)
models={}
for label,pid,pc,of,*_ in defs:
 v=w.objs[e.r64(core+e.r64(of))]
 v.native_imps['setUserCutoff:']=0x10055ac64
 e.wd(v.addr+0x330,float('nan'));models[label]=v.addr
# Matches current constructor append slots (Sub, OSC1, OSC2) at
# 0x1002b8fbc..0x1002b9058; Noise is sent separately at 0x1000eef28.
e.w64(core+0x658,w.native_array([models[x] for x in ('Sub','OSC1','OSC2')]))
trace=[]
def endpoint(w,o,e):
 label,d=dsp[o.addr]; addr=e.x(2);actual=e.s(0);assert addr==36
 trace.append([label,addr,actual]);e.deferred.append(lambda:d.set_param(addr,actual,True))
 return True
w.sel_handlers['setParameterWithAddress:value:']=endpoint
e.wr(cond+0x580,b'\0')
rows=[]
for x,expected in ((.4,300.),(1.,20000.001953125),(.4,300.)):
 trace.clear();e.call(0x1000eee00,floats=(x,),regs={20:cond})
 assert trace==[[a,36,expected] for a in ('Sub','OSC1','OSC2','Noise')],trace
 rows.append(dict(public_input=f32(x),executed_real_receiver_writes=trace.copy()))
for off,val in ((0x10082f568,0x678),(0x10082f258,0x590),(0x10082f250,0x598),(0x10082f240,0x5a0),(0x10082f248,0x5a8),(0x10082f260,0x5b0)):e.w64(off,val)
e.wr(cond+0x598,b'\0') # disabled/timer idle
e.wf(inst+0x110,.3345831036567688) # independent observed lfo1Dest1BaseValue property
legacy=[]
for x,expected in ((.3345831036567688,211.95989990234375),(1.,20000.001953125)):
 trace.clear();e.call(0x1000eeca8,floats=(x,),regs={20:cond})
 assert trace==[[a,36,expected] for a in ('Sub','OSC1','OSC2','Noise')]
 assert e.rf(core+0x678)==f32(x) and e.rf(cond+0x5a8)==f32(x)
 assert e.rf(inst+0x110)==.3345831036567688
 legacy.append(dict(normalized=f32(x),legacy_core_level=e.rf(core+0x678),lfobase_inst_110_unchanged=e.rf(inst+0x110),physical=trace.copy()))
assert not e.unknown
print(json.dumps(dict(scope='Actual current #7 base setter with disabled LFO and actual shared cutoff setter through actual Swift AKSampler closure to physical DSP addr36. No enclosing panel snapshot watcher asserted.',actual_executable=sha(BIN),rows=rows,legacy=legacy),indent=2))
