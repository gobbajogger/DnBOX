"""Execute old authorized Swift senders -> property -> real DSP -> VoicePrep.

Minimal fake platform object custody only: no Mac audio, app timer, media/UI,
full original graph, or LFO scheduling. Normal mode, no live cutoff writes.
"""
from emu import Emu
from swiftrt import SwiftWorld
from sampler import SamplerDSP
from unicorn import UC_HOOK_CODE
from pathlib import Path
from hashlib import sha256
import objcmeta
import json
import struct
import ctypes as C
import paths
import os

assert sha256(Path(paths.BIN).read_bytes()).hexdigest()=="a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1"
E=Emu(); W=SwiftWorld(E); E.deferred=[]
D=SamplerDSP(E,44100); O=SamplerDSP(E,44100)
c=E.calloc(0x1000); i=E.calloc(0x1000); g=E.calloc(0x1000)
cls='_TtC8AudioKit9AKSampler'
sam,other=[W.new_obj(cls,size=0x400) for _ in range(2)]
E.w64(0x100809a40,0x100); E.w64(c+0x100,i); E.w64(i+32,g)
for n,a in enumerate((0x10080a260,0x10080a258,0x10080a458,0x10080a590,0x10080a598,0x10080a578),1):
    E.w64(a,0x200+n*0x10)
params={
    129:(0x1000ed7cc,.4000000059604645,36,300.0),
    130:(0x1000ee1cc,.4141378700733185,8,7.86447811126709),
    131:(0x1000eddf4,.5313359498977661,39,.13283398747444153),
    132:(0x1000ef1e8,.5168501734733582,16,1.7785197496414185),
    133:(0x1000ef3a0,.47146177291870117,17,1.9349507093429565),
    134:(0x1000ee16c,.08876017481088638,18,.0023471745662391186),
    135:(0x1000ef558,.604790449142456,19,4.653963088989258),
    136:(0x1000edf88,.4559931755065918,7,.09260699152946472),
    137:(0x1000eee54,.9064822196960449,40,.9971304535865784),
}
E.wf(g+E.r64(0x10080a578),params[129][1]); E.wf(g+E.r64(0x10080a590),params[136][1])
for a,b in ((0x1007bee88,W.native_array([sam.addr])),(0x1007bee58,other.addr)):
    E.w64(a,0x600+(a&0xfff)%256); E.w64(g+E.r64(a),b)
imps=dict(objcmeta.classes[cls][1]); ivars=objcmeta.ivars(cls)
pmap={}
for node,dsp in ((sam,D),(other,O)):
    node.native_imps=imps
    au=W.new_obj('_TtC8AudioKit18AKSamplerAudioUnit',size=0x400)
    pmap[au.addr]=dsp
    for n,ptr,off,sz in ivars:
        if n=='internalAU': E.w64(node.addr+off,au.addr)
        if sz==8 and n.startswith(('filter','userCutoff','keyTracking')):
            E.wd(node.addr+off,float('nan'))
recv=[]
def receiver(w,o,e):
    if o.addr not in pmap:return False
    dsp=pmap[o.addr]; n=e.x(2); v=e.s(0)
    recv.append((n,v,dsp is D))
    e.deferred.append(lambda:dsp.set_param(n,v,True))
    return True
W.sel_handlers['setParameterWithAddress:value:']=receiver
# Attach the actual original AKMixer setter to the oscillator-sum receptor.
mix=W.new_obj('_TtC8AudioKit7AKMixer',size=0x500)
av=W.new_obj('AVAudioMixerNode',size=0x500)
mix.native_imps={'setVolume:':0x1004667d4}
E.w64(mix.addr+E.r64(0x1007ea7b8),av.addr)
E.w64(0x1007bee80,0x680);E.w64(g+0x680,mix.addr)
E.w64(0x10080a458,0x6a0)
out=[]
def capture(w,o,e):
 assert o is av
 out.append(e.s(0))
 return True
W.sel_handlers['setOutputVolume:']=capture
def f32(x):return struct.unpack('<f',struct.pack('<f',x))[0]
rows={}
for ota,coeff in ((0,0x1006097c0),(1,0x1006068f0)):
 E.wr(c+0x6a0,bytes([ota]))
 assert E.r64(i+32)==g
 k=E.rd_(coeff); a=[]
 for raw in sorted({f32(j/127) for j in range(128)}|{params[130][1]}):
  out[:]=[];recv[:]=[]
  E.call(0x1000ee1cc,floats=(raw,),regs={20:c},max_insns=1000000)
  expected=f32(1-k*pow(raw,1.5))
  assert out==[expected],(ota,raw,k,expected,out,E.unknown)
  assert [x[0] for x in recv]==[8,8],recv
  a.append([raw,out[0],recv[0][1],recv[1][1]])
 rows[str(ota)]={'coeff':k,'coeff_address':hex(coeff),'min_mixer_gain':min(v[1] for v in a),'max_mixer_gain':max(v[1] for v in a),'authored_brainwave':next(v for v in a if v[0]==params[130][1]),'rows':a}
unrelated={}
for ota in (0,1):
 E.wr(c+0x6a0,bytes([ota]))
 for aid,(pc,raw,slot,v) in params.items():
  if aid==130:continue
  out[:]=[]; recv[:]=[]
  E.call(pc,floats=(raw,),regs={20:c},max_insns=1000000)
  assert not out
  unrelated.setdefault(str(ota),{})[str(aid)]=recv.copy()
assert not E.unknown
print(json.dumps({'scope':'real old source resonance control->actual two AKSampler DSP receiver sends and real AKMixer setter->observed AVAudioMixer API; no Apple PCM, no current AU','input_columns':['resonance_raw','AVAudioMixer outputVolume','first real DSP f32 param8','second real DSP f32 param8'],'ota_states':rows,'controls_proved_not_to_write_fanin_volume':unrelated},indent=2))
