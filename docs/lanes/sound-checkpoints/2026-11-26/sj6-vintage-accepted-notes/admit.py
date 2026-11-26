"""Execute authentic current eligible-note mask selection using controlled ledger."""
from pathlib import Path
from unicorn.arm64_const import UC_ARM64_REG_X25
s=Path('sj6-native-vintage-owner/events.py').read_text().split('v=go(')[0]
exec(compile(s,'source real note emit fixture', 'exec'))
e.handlers["_swift_beginAccess"]=lambda:None
cond=e.calloc(0x900);i=e.calloc(0x900);q=e.calloc(0x900)
arr=e.calloc(0x100);vwt=e.calloc(0x100);nw=e.calloc(0x100)
e.w64(cond+0x100,i);e.w64(i+0x20,core)
e.w64(0x10082f5e8,0x600);e.w64(0x10082f338,0x200)
e.w64(cond+0x200,arr)
e.w64(vwt+0x48,16);e.wr(vwt+0x50,b'\x07')
e.w32(nw+0x14,0);e.w32(nw+0x20,1);e.w32(nw+0x28,2)
def after_admit(*_):
    # Keep consumed authenticated allowed mask in x22; bridge only
    # platform Swift copy/layout prep needed by our narrower emitted stage.
    e.w64(f-0xc8,core)
    e.uc.reg_write(UC_ARM64_REG_X25,core)
    e.w64(f-0xb8,meta+0x200)
    e.uc.reg_write(UC_ARM64_REG_PC,0x1002f8da4)
e.uc.hook_add(UC_HOOK_CODE,after_admit,begin=0x1002f8d34,end=0x1002f8d34)
e.uc.hook_add(UC_HOOK_CODE,init,begin=0x1002f8bf4,end=0x1002f8bf4)
result=[]
for key,mode,existing,mask,expect in (
 (36,0,[] ,3,3),
 (48,0,[(36,3,0)],3,3),
 (52,0,[(48,3,0)],3,3),
 (55,0,[(48,3,0),(52,3,0)],3,3),
 (55,0,[(55,3,0)],3,0),
 (55,0,[(55,1,0)],3,2),
 (55,0,[(55,3,1)],3,3),
 (55,1,[(55,3,0)],3,3)):
  e.wd(core+0x328,5*float(np.float32(.8268753290176392))**2.02)
  e.wr(core+0x600,bytes([mode])); e.w64(arr+0x10,len(existing))
  for j,rr in enumerate(existing):e.wr(arr+32+j*16,bytes(rr)+bytes(13))
  before=len(calls); e.call(0x1002f8bf4,regs={19:cond,21:nw,22:0x100,23:key,25:mask,26:86,27:vwt,28:q})
  r=calls[before:]
  assert sum(1<<(o-1) for k,o,d in r)==expect,(key,mode,existing,r)
  result.append(dict(key=key,poly_dedup_override_mode_bit=mode,ledger_provided=existing,requested=mask,expected_pass=expect, actual_outputs=r))
assert not e.unknown
print(json.dumps(dict(binary=pin(BIN),scope='Actual current event admission from note ledger plus per-owner native detune sends. Caller/mode bit and minimal ledger layout are supplied for differential authored semantics; no claim of the saved AU instance actor metadata.', cases=result),indent=2))
