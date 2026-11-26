"""Execute the actual current constructor through its first Apple-owned audio node."""
from pathlib import Path
import json
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_X29,UC_ARM64_REG_LR
s=Path('sj6-modern-owner/probe.py')
exec(compile(s.read_text().split('def trial(')[0],str(s),'exec'))
paths.BIN=str(BIN);import objcmeta as o
from swiftrt import SwiftWorld
e=Emu(str(BIN)); w=SwiftWorld(e)
c='_TtC8AudioKit7AKMixer'
apple_class=0x1007a06e8
imp=dict(o.classes[c][1])['init']; assert imp==0x10047b944
obj=w.new_obj(c,role='oscSection');obj.native_imps={'init':imp}
av=e.r64(apple_class);assert w.class_objs[av]=='AVAudioMixerNode'
core=e.calloc(0x700);bp=e.calloc(0x800)+0x300
off=(0x110,0x118,0x120,0x128);roles=['MainA','MainB','Sub','Noise']
sources=[w.new_obj('AKSampler',role=i).addr for i in roles]
for n,x in zip(off,sources):e.w64(core+n,x)
array=w.native_array([0]*4)
filtered=w.native_array([0]*4);e.w64(filtered+0x10,0)
e.w64(0x100757080,filtered)
for i,ofs in enumerate((0x10080f548,0x10080f540,0x10080f550,0x10080f558)):e.w64(ofs,0x250+i*8)
seen=[]
def start(*_):
  e.uc.reg_write(UC_ARM64_REG_X29,bp);e.w64(bp-0x1c8,off[0]);e.w64(bp-0x1c0,off[2])
def capture_pack(*_):
  assert [e.r64(array+0x20+i*8) for i in range(4)]==sources
  seen.append('CoreGeneratorBank actual ordered AKSampler inputs')
def got_inner(*_):
  assert e.x(0)==filtered and all(e.r64(filtered+0x20+i*8)==x for i,x in enumerate(sources))
  assert e.r64(filtered+0x10)==4
  seen.append('AKMixer initialized with all four current lane nodes')
def both(*_):seen.append('Reached actual AKMixer init')
def alloc():
  ptr=e.x(0)
  if ptr==av:
    assert e.uc.reg_read(UC_ARM64_REG_LR)==0x10047b968
    seen.append('First opaque audio-owned boundary: AVAudioMixerNode allocWithZone; no host audio callback / no samples')
    e.uc.emu_stop()
  else:
    assert ptr==obj.addr
    e.setx(0,obj.addr)
e.handlers['_objc_allocWithZone']=alloc
e.handlers['_swift_getObjectType']=lambda:None
e.handlers['_swift_isUniquelyReferencedNonObjC_nonNull_bridgeObject']=lambda:e.setx(0,1)
for pc,h in {0x1002b8914:start,0x1002b893c:capture_pack,0x10047c200:got_inner,imp:both}.items():
 e.uc.hook_add(UC_HOOK_CODE,h,begin=pc,end=pc)
# Explicit Swift constructor allocator surrogate before first Apple render node.
def mixer_metadata(*_):e.setx(20,obj.addr)
e.uc.hook_add(UC_HOOK_CODE,mixer_metadata,begin=0x10047baf8,end=0x10047baf8)
e.call(0x1002b8914,regs={28:core,23:off[1],27:off[3],25:array})
assert seen==['CoreGeneratorBank actual ordered AKSampler inputs','AKMixer initialized with all four current lane nodes','Reached actual AKMixer init','First opaque audio-owned boundary: AVAudioMixerNode allocWithZone; no host audio callback / no samples'],(seen,e.unknown,w.msglog[-4:])
assert not e.unknown
print(json.dumps(dict(binary=pin(BIN),cached_raw_graph_execution=False,actual_pcm_frames=0,roles=roles,reached=seen,
   missing='Actual installed oscSection AVAudioMixerNode output PCM immediately before instrument Unison and instrument HP; no substitution for absent Apple node', guards='read-only constructor slice, synthetic object metadata/retain allocation, original ordered array and wrapper dispatch, explicitly stop at external Apple allocation; no fake format or zero audio'),indent=2))
