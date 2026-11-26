"""Execute exact current Conductor note LFO cutoff-restore slice, then true DSP relay."""
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_PC,UC_ARM64_REG_LR
s=Path('sj6-osc2-source/cutoff_relay.py').read_text().split('legacy=[]')[0]
exec(compile(s,'verified current cutoff relay context','exec'))
raw_live={x['address']:x['value'] for x in d['parameters']}
assert raw_live['181']==1 and raw_live['180']==.3345831036567688 and raw_live['91']==.35310420393943787
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
assert .01<=auth_delay<3
seq=e.calloc(0x200);held=e.calloc(0x100);e.w64(inst+0xb0,seq);e.w64(seq+0xa0,held)
# Unrelated vibrato/note graph and panel publication excluded; cutoff target,
# timer state, and Swift->DSP physical relay are never bridged.
for a in (0x1000ee144,0x1000ee200,0x1002948dc):at(a,skip)
for a in (0x100018c1c,0x100018c40,0x100018c90):at(a,e.uc.emu_stop)
cases={}
for label,n,delay,kind in [('matched_empty_note_list',0,auth_delay,auth_enum),('other_key_already_held',1,auth_delay,auth_enum),('no_lfo_delay',0,0.,auth_enum),('other_destination',0,auth_delay,0xcf)]:
 e.w64(held+0x10,n);e.wd(inst+0x120,delay);e.wr(inst+0x114,bytes([kind]))
 e.wr(cond+0x590,b'\0');e.wr(cond+0x598,b'\0');e.wr(cond+0x5b0,b'\0')
 e.call(0x1000eeca8,floats=(1.,),regs={20:cond})
 assert e.rf(core+0x678)==1
 trace.clear()
 e.call(0x100018b44,regs={19:cond,25:0x100,9:1})
 target=e.rf(cond+0x5a0);current=e.rf(cond+0x5a8);active=e.rd(cond+0x598,1)[0]
 assert target==(raw_live['180'] if label=='matched_empty_note_list' else 1)
 assert current==1 and active==int(label=='matched_empty_note_list') and not trace
 rows=[]
 for i in range(40):
  trace.clear()
  e.call(0x100269fa8,doubles=(128/44100,),regs={20:cond})
  assert e.rf(core+0x678)==1 and e.rf(inst+0x110)==raw_live['180']
  rows.append([i+1,e.rf(cond+0x5a8),trace.copy()])
 if active:
  assert 211.95989990234375<=next(row[-1][2][2] for row in reversed(rows) if row[-1])<220
 else:assert not any(r[-1] for r in rows)
 if active:
  trace.clear()
  e.call(0x1000eeca8,floats=(raw_live['180'],),regs={20:cond})
  assert not trace
  assert e.rf(core+0x678)==raw_live['180'] and e.rf(cond+0x5a0)==0 and e.rd(cond+0x598,1)==b"\1"
  for j in range(45):
   trace.clear()
   e.call(0x100269fa8,doubles=(128/44100,),regs={20:cond})
   rows.append([41+j,e.rf(cond+0x5a8),trace.copy()])
 cases[label]=dict(note_prior_held_count=n,physical_delay_sec=delay,dest_enum=kind,target=target,initial=current,smoothing_active=bool(active),ticks=rows)
assert not e.unknown
print(json.dumps(dict(scope='Actual current compiled first-note selection and cutoff target setter, real source timer tick at proposed offline callbacks, real latest physical DSP messages. Only unrelated vibrato/panel side effects stubbed. Does not assert actual Apple timer period.',source=sha(BIN),authored_lfo_ui_index=raw_live['181'],actual_enum=auth_enum,actual_delay=auth_delay,public_cutoff_still=1,conductor_observations=cases),indent=2))
