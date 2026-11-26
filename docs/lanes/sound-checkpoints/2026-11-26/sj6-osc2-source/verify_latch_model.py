"""Independently verify comparison cutoff-latch arithmetic against current receipt."""
from pathlib import Path
import json
from cutoff_latch_model import LiveCutoff
src=json.loads(Path('sj6-osc2-source/note_cutoff_latch.json').read_text())
ref=src['conductor_observations']
verify={}
for name,v in ref.items():
 mod=LiveCutoff(1.,v['target'] if name=='matched_empty_note_list' else ref['matched_empty_note_list']['target'],v['dest_enum'],v['physical_delay_sec'])
 if v['note_prior_held_count']:mod.keys.add(36)
 trigger=mod.note(48,True)
 assert trigger==v['smoothing_active']
 assert mod.public==1 and mod.applied==v['initial'] and mod.target==v['target']
 for k,exact,trace in v['ticks']:
  mod.tick(128/44100)
  assert mod.applied==exact,(name,k,mod.applied,exact)
 verify[name]=len(v['ticks'])
extended=json.loads(Path('sj6-osc2-source/live_restore_relay.json').read_text())
guard=extended['conductor_observations']['matched_empty_note_list']
state=LiveCutoff(1.,guard['target'],guard['dest_enum'],guard['physical_delay_sec'])
assert state.note(48,True)
for n,a,_ in guard['ticks']:
 if n==41:
  assert state.set_public(guard['target']) is None and state.target==0. and state.running
 state.tick(128/44100)
 assert state.applied==a,(n,a,state.applied)
verify['active_panel_restore']=len(guard['ticks'])
t=LiveCutoff(1.,.3,7,.125)
assert t.note(60,True) and not t.note(64,True) and not t.note(67,True)
for n in (60,64,67):t.note(n,False)
assert t.note(62,True)
print(json.dumps(dict(scope='Clean-room comparison model validated against actual compiled newest control and timer slices; no audio and no claims of platform callback cadence.',exact_normalized_tick_rows=verify),indent=2))
