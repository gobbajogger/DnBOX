"""Execute exact current Conductor note LFO cutoff-restore slice, then true DSP relay."""
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_PC,UC_ARM64_REG_LR
s=Path('sj6-osc2-source/cutoff_relay.py').read_text().split('legacy=[]')[0]
exec(compile(s,'verified current cutoff relay context','exec'))
A=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
assert sha(A)=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(A) as tf:
 c=json.load(tf.extractfile('./program-1-dry/capture.json'))
 assert c['active']==c['after']
raw_live={str(x['address']):x['value'] for x in c['active']}
assert raw_live['181']==1 and raw_live['180']==1 and raw_live['91']==0
for n,a in enumerate((0x180,0x188,0x190,0x198,0x210,0x1a8,0x1c0,0x1b0,0x1c8,0x1b8,0x1d0,0x1a0)):
 e.w64(0x10082f000+a,0x610+n*8)
# Current source creates this immutable destination table at .059d9c.
TABLE=0x1007d7a10
assert e.rd(TABLE+0x20,2)==bytes([0xcf,7])
e.w64(inst+0x1a0,TABLE)
def at(pc,fn): e.uc.hook_add(UC_HOOK_CODE,lambda *_:fn(),begin=pc,end=pc)
def skip():e.uc.reg_write(UC_ARM64_REG_PC,e.uc.reg_read(UC_ARM64_REG_LR))
at(0x1000ec664,e.uc.emu_stop)
e.sets(8,raw_live['181']);e.call(0x1000ec5e8,regs={19:cond,25:0x100})
auth_enum=e.rd(inst+0x114,1)[0]
assert auth_enum==7 and e.rf(inst+0x118)==raw_live['181']
e.call(0x1000f8ddc,floats=(raw_live['91'],),regs={20:cond})
auth_delay=e.rd_(inst+0x120)
assert 0<=auth_delay<=.01
seq=e.calloc(0x200);held=e.calloc(0x100);e.w64(inst+0xb0,seq);e.w64(seq+0xa0,held)
# Unrelated vibrato/note graph and panel publication excluded; cutoff target,
# timer state, and Swift->DSP physical relay are never bridged.
for a in (0x1000ee144,0x1000ee200,0x1002948dc):at(a,skip)
for a in (0x100018c1c,0x100018c40,0x100018c90):at(a,e.uc.emu_stop)
cases={}
for label,n,delay,kind in [('solidify_exact_empty_note_list',0,auth_delay,auth_enum),('long_delay_positive_guard',0,.8,auth_enum),('other_key_held',1,.8,auth_enum)]:
 e.w64(held+0x10,n);e.wd(inst+0x120,delay);e.wr(inst+0x114,bytes([kind]))
 e.wf(inst+0x110,raw_live['180'] if label.startswith('solidify') else .4)
 e.wr(cond+0x590,b'\0');e.wr(cond+0x598,b'\0');e.wr(cond+0x5b0,b'\0')
 e.call(0x1000eeca8,floats=(raw_live['7'],),regs={20:cond})
 trace.clear()
 e.call(0x100018b44,regs={19:cond,25:0x100,9:1})
 armed=bool(e.rd(cond+0x598,1)[0])
 assert armed==(label=='long_delay_positive_guard')
 cases[label]={'armed':armed,'depth':raw_live['89'],'physical_delay':delay,'base':e.rf(inst+0x110),
 'cutoff_panel':raw_live['7'],'destination_enum':auth_enum,'physical_sends':trace.copy()}
assert not e.unknown
print(json.dumps(dict(source=sha(BIN),cases=cases),indent=2))
