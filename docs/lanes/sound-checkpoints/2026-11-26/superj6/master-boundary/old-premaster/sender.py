"""Old instrument master callback through actual AKMixer Swift outputVolume property."""
import json,math,struct
from pathlib import Path
from hashlib import sha256
from swiftrt import SwiftWorld
from emu import Emu
import paths
assert sha256(Path(paths.BIN).read_bytes()).hexdigest()=='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
E=Emu(); W=SwiftWorld(E); trace=[]
float32=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
c=E.calloc(0x1000); i=E.calloc(0x1000)
av=W.new_obj('AVAudioMixerNode','test-output',size=0x500)
mx=W.new_obj('_TtC8AudioKit7AKMixer','test-attenuator',size=0x500)
mx.native_imps={'setVolume:':0x1004667d4}
E.w64(mx.addr+E.r64(0x1007ea7b8),av.addr)
E.w64(c+E.r64(0x1000ec000+7458816+2624),i)
E.w64(i+200,mx.addr)
def out(w,obj,e):
 assert obj is av
 trace.append(e.s(0))
 return True
W.sel_handlers['setOutputVolume:']=out
rows=[]
for x in (0.,.25,.3150142729282379,.5,.75,1.,1.32):
 x=float32(x)
 trace[:]=[]
 E.call(0x1000ecef8,floats=(x,),regs={20:c})
 assert trace==[float32(x*.404)],(x,trace,E.unknown)
 rows.append([x,trace[0]])
assert not E.unknown,E.unknown
print(json.dumps(dict(scope="authenticated old master sender -> real old Swift Mixer -> observed public AVAudioMixer parameter setter; no Apple mixer PCM advertised",rows=rows),indent=2))
