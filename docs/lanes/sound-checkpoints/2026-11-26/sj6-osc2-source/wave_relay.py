"""Capture actual normalized modern OSC2 selector through real guest shape switch."""
from pathlib import Path
p=Path('sj6-current-faders/probe.py')
exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'))
label=next(k for k,a in dsp.items() if a[0]=='OSC2')
osc=w.objs[e.r64(core+e.r64(0x1007e3bb0))]
d=dsp[label][1]
e.w64(0x10082f5b8,0x660);e.w64(0x10082f5c8,0x668);e.w64(0x10082f5a0,0x66c)
e.wf(core+0x66c,0.)
meta=e.calloc(0x1000);mask=e.calloc(8);e.w64(mask,0xfffffffffffffff8)
# Tag known actual guest Swift virtual ABI with diagnostic dispatch bridges;
# leave original enum conversion, mode function, and actual DSP setters intact.
for vm,ordinal,_ in e.im.binds:
 if e.im.imports[ordinal][0]=='_swift_isaMask' and vm==0x100757258:e.w64(vm,mask)
e.w64(0x100757258,mask);e.w64(osc.addr,meta)
chosen=[]
def shape():
 value=e.x(0);chosen.append(value)
 e.deferred.append(lambda:e.call(0x1003898a4,d.dsp,value))
def ignore():pass
e.w64(meta+0x908,w.tramp('modern_selectShape_bridge',shape))
for t in (0x920,0x928):e.w64(meta+t,w.tramp('modern_other_mode_flag_observe',ignore))
trace=[]
for s in ('setIsSineOsc:','setIsTriOsc:','setIsSawOsc:','setPwmWidth:','setIsPWM:'):
 def watch(w,o,e,sel=s):
  assert o is osc;trace.append([sel,e.d(0) if sel=='setPwmWidth:' else bool(e.x(2))]);return True
 w.sel_handlers[s]=watch
records={}
for raw,expected in [(0.6666666865348816,2),(0.3333333432674408,1),(0.,0),(1.,3)]:
 trace.clear();chosen.clear()
 e.call(0x1000f5104,floats=(raw,),regs={20:cond})
 assert chosen==[expected] and e.r32(d.dsp+0x38+0x18)==expected,(raw,chosen,trace)
 records[str(raw)]=dict(stored_enum=e.rd(core+0x660,1)[0],mode_choice=chosen.copy(),observed_property_packet=trace.copy())
assert not e.unknown
print(json.dumps(dict(scope='Actual app-facing waveform index conversion, channel owner, actual Swift selector (virtual intermediary bridged to exact DSP C setter); no case-specific tuning.',source=sha(BIN),cases=records),indent=2))
