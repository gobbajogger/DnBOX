"""Execute current source's admitted note sender, with controlled dispatch metadata."""
from pathlib import Path
s=Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0]
exec(compile(s,'verified original current receiver','exec'))
from unicorn.arm64_const import UC_ARM64_REG_PC,UC_ARM64_REG_X29,UC_ARM64_REG_LR
e=Emu(str(BIN))
core=e.calloc(0x900); meta=e.calloc(0x900); stack=e.calloc(0x900); f=stack+0x400
# The guest owns all branch/range/RNG arithmetic after the admitted NOTE
# mask. Class metadata and neutral transport/tuning endpoints are bridged.
for loc,n in ((0xb80,0x300),(0xb88,0x302),(0xc00,0x308),(0xc08,0x310),(0xc10,0x318),(0xc18,0x320),(0xb38,0x328),(0xba8,0x330),(0xbb0,0x338)):
 e.w64(0x1007e3000+loc,n)
e.wr(core+0x300,b'\x00\x7f\x00\x7f')
for n in (0x308,0x310,0x318,0x320):e.wd(core+n,86.)
e.w64(core+0x330,meta+0x20);e.w64(core+0x338,meta+0x100)
e.w64(f-0xc8,core);e.w64(f-0xb8,meta+0x200);e.w64(f-0xd8,0);e.w32(f-0xdc,0)
rows=[]; words=[]; calls=[]; logical=[]
def handler():
   assert e.x(1)==8
   # Preset-independent high/low ladder, eight bit-exact deterministic host
   # words; controlled entropy probe, not original recorded Swift PRNG.
   bit=((len(words)*3+1)%8)<<61
   words.append(bit)
   e.w64(e.x(0),bit)
e.handlers['_swift_stdlib_random']=handler
e.handlers['_objc_retain']=lambda:None
e.handlers['_objc_release']=lambda:None
e.uc.hook_add(UC_HOOK_CODE,lambda *_:logical.append(e.x(0)),begin=0x1002fa6c0,end=0x1002fa6c0)
def bridge(path,fn):
    def invoke(*_):
        fn()
        e.uc.reg_write(UC_ARM64_REG_PC,e.uc.reg_read(UC_ARM64_REG_LR))
    e.uc.hook_add(UC_HOOK_CODE,invoke,begin=path,end=path)
bridge(0x1002fa870,lambda:e.setd(0,0.))
bridge(0x1002b9ef0,lambda:None)
def send():
   osc={meta+0x20:1,meta+0x100:2}[e.x(20)]
   calls.append([int(e.x(0))&255,osc,e.d(0)])
bridge(0x1004b1e90,send)
def init(*_):e.uc.reg_write(UC_ARM64_REG_X29,f)
e.uc.hook_add(UC_HOOK_CODE,init,begin=0x1002f8da4,end=0x1002f8da4)
for stop in (0x1002f914c,0x1002f91a0):
 e.uc.hook_add(UC_HOOK_CODE,lambda *_:e.uc.emu_stop(),begin=stop,end=stop)
def go(depth):
 e.wd(core+0x328,depth);rr=[]
 for frame,key,mask in [(16384,36,3),(49459,48,3),(49459,52,3),(49459,55,3),(66000,57,1),(66100,59,2),(66200,61,0)]:
   start=len(words);out=len(calls);d0=len(logical)
   e.call(0x1002f8da4,regs={22:mask,23:key,25:core,26:86})
   rr.append(dict(frame=frame,key=key,admitted_layer_mask=mask,entropy_words=len(words)-start,draws=len(logical)-d0,physical_source_calls=calls[out:]))
 return rr
v=go(5*float(np.float32(.8268753290176392))**2.02)
z=go(0.)
for v1,z1 in zip(v,z):
   mask=v1['admitted_layer_mask']
   assert v1['draws']==z1['draws']==mask.bit_count(),(v1,z1)
   assert all(abs(x[2])<=.035 for x in v1['physical_source_calls'])
   assert all(x[2]==0 for x in z1['physical_source_calls'])
assert v[1]['physical_source_calls']!=v[2]['physical_source_calls']
assert not e.unknown
print(json.dumps(dict(binary=pin(BIN),gate='Covers identical admitted Note event fragment, does not assert top conductor produces these masks',vintage=v,zero=z),indent=2))
