"""Trace current Core note receiver, actual physical cutoff and Normal voice prep."""
from pathlib import Path
text=Path('sj6-osc2-source/open_legacy_discriminator.py').read_text().split('baseline,taps=')[0]
exec(compile(text,'validated original source code','exec'))
orig_blocks=r['buffers'].copy()
def split_boundary(blocks,at):
 off=0
 for i,n in enumerate(blocks):
  if off==at:return blocks.copy()
  if off<at<off+n:return blocks[:i]+[at-off,off+n-at]+blocks[i+1:]
  off+=n
 raise AssertionError(at)
r["buffers"]=split_boundary(orig_blocks,61440)
fixed=render_str.replace(' d.set_param(36,cutoff,True)',' d.set_param(6,10.,True)\n d.set_param(36,cutoff,True)')
fixed=fixed.replace("  frame=n-r['warm_frames']","""  if n in changes:
   d.set_param(36,changes[n],True)
   assert e.rf(d.dsp+0x158)==changes[n]
  frame=n-r['warm_frames']""")
fixed=fixed.replace(" events={}",""" events={}
 p={16384,24576,24704,30080,49459,49715,49843,49875,49971,53971,61440,64000}
 def coef(*_):
  if n in p: rows.append([n,hex(e.x(0)-voice[0]),e.rf(d.dsp+0x158),e.d(0),e.d(1),e.r32(voice[0]+0x340)])
 def prep(*_):
  voice[0]=e.x(0)
 def commit(*_):
  commits.append([n,e.rf(d.dsp+0x158)])
 def setter(*_):
  if e.x(1)==36: setter_calls.append([n,e.s(0)])
 voice=[0]
 for a,cb in ((0x1003b4c54,prep),(0x10037fdbc,coef),(0x1003b5120,commit),(0x100389e5c,setter)):
  e.uc.hook_add(UC_HOOK_CODE,cb,begin=a,end=a)
 e.uc.ctl_remove_cache(0x100389e5c,0x10038a2b4)""")
fixed=fixed.replace("33075+4096","65536-r['warm_frames']")
exec(compile(fixed,'actual modern Normal receiver and exact MIDI ownership','exec'))
res={}
for arm,first in [('authored-noop',hz),('physical-held-step',20000.001953125)]:
 rows=[];commits=[];setter_calls=[];changes={24576:first,61440:hz}
 render(True,4,hz)
 assert setter_calls==[[a,v] for a,v in changes.items()],setter_calls
 pairs={frame for frame,old in commits}
 assert len(pairs)==1 and all((cutoff==first for frame,cutoff in commits))
 contrast=[p[3] for p in rows if p[0]>49459 and p[0]<61440]
 assert contrast and all((q==int(first) for q in contrast)),(arm,sorted(set(contrast)),rows[:3])
 res[arm]=dict(rows=rows,cutoff_receiver_writes=setter_calls,source_voice_commits=commits)
print(json.dumps(dict(scope='Actual latest isolated approved ARM64 OSC2 hardware-neutral saw source, same warm note sequence, varying known physical slot36. Retention at exact compiled Normal coefficients through note handoff; outer app not executed.',source=pin(BIN),columns=['callback_first_frame','voice_coefficient_field','sampler_slot36','physical_cutoff_passed_to_normal_biquad','normal_damping','voice_amp_stage'],cases=res),indent=2))
