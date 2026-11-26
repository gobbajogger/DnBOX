from pathlib import Path
import hashlib,tarfile,json,sys,struct
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_PC,UC_ARM64_REG_LR,UC_ARM64_REG_X29
p=Path('sj6-modern-owner/probe.py')
exec(compile(p.read_text().split('def trial(')[0],str(p),'exec'))
e=Emu(str(BIN))
path=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
assert pin(path)=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(path) as t:m=json.load(t.extractfile('./program-2-dry/capture.json'))
rv={int(r['address']):r['value'] for r in m['active']}
assert all(rv[i]==0 for i in (3,8,57,63,86,105,123,130,201)) and rv[205]==1
keys=[i['note']-12 for i in m['schedule'] if i['action']=='on']
assert keys==[36,48,52,55]
core,frame,held,empty=[e.calloc(n) for n in (0x600,0x800,128,0x70)]
slots=(0x1007e3ba8,0x1007e3bb0,0x1007e3b98,0x1007e3bb8)
objects=[e.calloc(0x400) for i in slots]
for i,(s,v) in enumerate(zip(slots,objects)):e.w64(s,0x100+8*i);e.w64(core+0x100+8*i,v)
for i,s in enumerate((0x1007e3b80,0x1007e3b88)):e.w64(s,0x200+2*i);e.wr(core+0x200+2*i,bytes([0,127]))
for i,s in enumerate((0x1007e3c00,0x1007e3c08,0x1007e3c10,0x1007e3c18,0x1007e3b38)):
    e.w64(s,0x220+8*i);e.wd(core+0x220+8*i,0.)
# Empty neutral per-note offsets (fixture; no installed AU private state assertion).
for i,s in enumerate((0x10082f618,0x10082f620,0x10082f628,0x10082f630,0x10082f638,0x10082f640)):
    e.w64(s,0x250+8*i);e.w64(core+0x250+8*i,empty)
gen=e.calloc(0x70);e.w64(0x10082f648,0x2a0);e.w64(core+0x2a0,gen);e.w64(gen+0x10,1);e.w64(gen+0x20,0);e.w64(gen+0x28,127);e.w64(gen+0x30,0)
f=frame+0x200
e.w64(f-0xc8,core);e.w64(f-0xb8,held);e.w64(f-0xd8,0);e.w32(f-0xdc,0);e.w32(f-0x118,86)
seen=[];rows=[]
def exit_guest(reg=UC_ARM64_REG_LR):
    e.uc.reg_write(UC_ARM64_REG_PC,e.uc.reg_read(reg))
def setup(*_):e.uc.reg_write(UC_ARM64_REG_X29,f)
def send(*_):
    src=e.x(20)
    assert src in objects
    seen.append([objects.index(src),e.x(0),e.x(1),e.x(2),e.d(0),hex(e.uc.reg_read(UC_ARM64_REG_LR)-4)])
    exit_guest()
def rand(*_):e.setx(0,0);exit_guest()
def tune(*_):e.setd(0,0.);exit_guest()
def retain(*_):exit_guest()
hooks={0x1002f8da4:setup,0x1002f91a0:lambda *_:e.uc.emu_stop(),0x1004b1e90:send,0x1005fa020:retain,0x1005fa02c:retain,0x1002fa6c0:rand,0x1002fa870:tune}
for pc,h in hooks.items():e.uc.hook_add(UC_HOOK_CODE,h,begin=pc,end=pc)
for k in keys:
 for mask in range(16):
    seen.clear();e.wr(held,b'\0')
    e.call(0x1002f8da4,regs={22:mask,23:k,25:core,26:86})
    dest=[i for i in range(4) if (mask>>i)&1]
    assert [s[0] for s in seen]==dest,(mask,k,seen)
    assert all(s[1:5]==[k,86,0,0.] for s in seen)
    assert e.rd(held,1)[0]==mask
    rows.append(dict(key=k,admitted_owner_mask=mask,calls=list(seen)))
assert not e.unknown
print(json.dumps(dict(sha=pin(BIN),roles=['MainA','MainB','Sub','Noise'],rows=rows,
 boundaries='actual current note-dispatch body after owner admission mask, verified 16 owner subsets x4 physically ranged Raw note keys. Retain/release and neutral tuning/RNG framework selectors mocked as labeled; actual velocity helper, branch and arguments executed. AKSampler call intercepted at virtual thunk only; masks are fixtures, not observed loaded AU conductor state. No duplicated admitted sampler sends.'),indent=2))
