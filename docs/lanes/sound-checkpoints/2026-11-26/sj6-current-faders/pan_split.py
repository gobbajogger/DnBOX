"""Current original stereo split sender through actual AKMixer: not native PCM."""
from pathlib import Path
p=Path(__file__).with_name('probe.py')
exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'))
e.w64(0x10082f538,0x670);e.w64(0x1007f5020,0x20);e.w64(0x1007f4ff8,0x28)
e.w64(0x10080f548,0x300);e.w64(0x10080f550,0x320)
owner=e.calloc(0x60);e.w64(core+0x670,owner)
outer=[];mixes=[]
for i in range(5):
 v=w.new_obj('_TtC8AudioKit7AKMixer',size=0x500)
 a=w.new_obj('AVAudioMixerNode',size=0x500);v.native_imps={'setPan:':0x10047b798}
 e.w64(v.addr+0x300,a.addr);outer.append(a);mixes.append(v.addr)
e.w64(owner+0x28,w.native_array(mixes))
trace=[]
def handler(w,o,e):
 if o not in outer:return False
 trace.append([outer.index(o),e.s(0)])
 return True
# Real AKMixer setPan reaches native AVAudioMixerNode setPan.
w.sel_handlers['setPan:']=handler
vals={x['address']:x['value'] for x in d['parameters']}
assert vals['61']==0
tests={}
for x in (0.,.125,.25,.5,.75,1.):
 trace.clear();e.call(0x1000f0164,floats=(x,),regs={20:cond})
 assert trace==[[i,f32(x*k)] for i,k in enumerate([1.,-.95,.5,-.59,.33])],(x,trace,w.msglog[-5:])
 tests[str(x)]=trace.copy()
assert not e.unknown
print(json.dumps(dict(scope='Current ARM64 indexed stereo spread sender and actual Swift five AKMixers -> observed public AV pan parameter; no inferred channel gain.',source=sha(BIN),selected_authored_reference_readback_zero=vals['61'],pan_values=tests),indent=2))
