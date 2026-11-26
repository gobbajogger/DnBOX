from pathlib import Path
import json,tarfile,sys,hashlib
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_X29
p=Path('sj6-current-faders/stereo_trim.py')
exec(compile(p.read_text().split('r={}')[0].replace("p=Path(__file__).with_name('probe.py')","p=Path('sj6-current-faders/probe.py')"),str(p),'exec'))
A=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
assert sha(A)=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(A) as t:
 c=json.load(t.extractfile('./program-2-dry/capture.json'))
 assert c['active']==c['after']
raw={int(v['address']):f32(v['value']) for v in c['active']}
assert raw[8]==0. and raw[130]==0. and raw[209]==1. and raw[1]==1. and raw[3]==0.
trace.clear();e.wr(cond+0x580,b'\x01')
e.call(0x1000ef80c,floats=(raw[130],),regs={20:cond})
assert trace==[1.]
unity=trace[0]
e.call(0x1000ef80c,floats=(.5,),regs={20:cond})
assert trace[-1]<unity # actual same receiver responds when nonzero
positive=trace[-1]
fields={}
for l,_,_,g,*_ in defs:
 fields[l]=[e.r64(g),e.r64(core+e.r64(g))]
for k,l in enumerate(('osc1Booster','osc2Booster')):
 addr=0x1007e3bd0+8*k; off=0x590+8*k
 e.w64(addr,off)
 o=w.new_obj('_TtC8AudioKit9AKBooster',size=0x500)
 e.w64(core+off,o.addr);fields[l]=[off,o.addr]
tokens={p[1]:k for k,p in fields.items()}
out=e.calloc(256);stack=e.calloc(2048);bp=stack+800
def frame(*_):
 e.uc.reg_write(UC_ARM64_REG_X29,bp)
 e.w64(bp-0x1c8,fields['OSC1'][0]);e.w64(bp-0x1c0,fields['Sub'][0])
got=[]
def serialize(*_):
 got.extend(tokens[e.r64(out+0x20+8*k)] for k in range(4));e.uc.emu_stop()
e.uc.hook_add(UC_HOOK_CODE,frame,begin=0x1002b8914,end=0x1002b8914)
e.uc.hook_add(UC_HOOK_CODE,serialize,begin=0x1002b893c,end=0x1002b893c)
e.call(0x1002b8914,regs={28:core,23:fields['OSC2'][0],27:fields['Noise'][0],25:out})
assert got==['OSC1','OSC2','Sub','Noise']
bt=[]
def bk(w,o,e):
 bt.append((tokens[o.addr],e.d(0)));return True
w.sel_handlers['setGain:']=bk
e.uc.hook_add(UC_HOOK_CODE,lambda *_:e.uc.emu_stop(),begin=0x1002b8f88,end=0x1002b8f88)
e.call(0x1002b8f60,regs={28:core})
assert bt==[('osc1Booster',.5),('osc2Booster',.5)] and not e.unknown
r=json.loads(Path('sj6-raw-release-tap/result.json').read_text())
assert r['original_pcm']==c['pcm_sha256']
windows={k:r['window_stage_levels'][k] for k in ('held','release_0_16ms','release_16_50ms','release_50_100ms')}
print(json.dumps(dict(executable=sha(BIN),voice_mixer_input_order=got,constructor_booster_write_outside_array=bt,
 old_pcm_comparison_scope='native terminal vs selected compact stage, not common-input internal source data',
 raw_postvoice_mixer_validated_volume=unity, positive_resonance_guard_volume=positive,
 master_kept=r['untouched_master'],official_ref_sha=c['pcm_sha256'],windows=windows),indent=2))
