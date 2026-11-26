"""Current original shared post-fanin trim and stereo selector; no Apple PCM."""
from pathlib import Path
p=Path(__file__).with_name('probe.py')
text=p.read_text().split('cases={}')[0]
exec(compile(text,str(p),'exec'))
av=w.new_obj('AVAudioMixerNode',size=0x500)
mix=w.new_obj('_TtC8AudioKit7AKMixer',size=0x500)
mix.native_imps={'setVolume:':0x10047b638}
e.w64(0x10080f548,0x300);e.w64(0x10080f540,0x320);e.w64(mix.addr+0x300,av.addr)
e.w64(0x1007e3be0,0x640);e.w64(core+0x640,mix.addr)
e.w64(0x1007e3be8,0x658)
e.w64(core+0x658,w.native_array([next(o.addr for o in w.objs.values() if o.cls.endswith('9AKSampler'))]))
e.w64(0x10082f448,0x580)
trace=[]
def capture(w,o,e):
 assert o is av
 trace.append(e.s(0));return True
w.sel_handlers['setOutputVolume:']=capture
r={}
raw_live={p['address']:p['value'] for p in d['parameters']}
assert raw_live['130']==.4141378700733185
for mode,k in ((0,.49881276637272776),(1,.4)):
 e.wr(cond+0x580,bytes([mode]));rows=[]
 for f in sorted({f32(i/127) for i in range(128)}|{raw_live['130']}):
  trace.clear();e.call(0x1000ef80c,floats=(f,),regs={20:cond})
  assert trace==[f32(1-k*f**1.5)],(mode,f,trace)
  rows.append([f,trace[0]])
 r[str(mode)]={'coefficient':k,'authored_brainwave':next(y for x,y in rows if x==raw_live['130']),'rows':rows}
assert not e.unknown
print(json.dumps(dict(scope='Actual newest app resonance setter and Swift mixer object sends directly to external Apple mixer parameter. No external mixer PCM claimed.',sha=sha(BIN),fan_in_trim=r),indent=2))
