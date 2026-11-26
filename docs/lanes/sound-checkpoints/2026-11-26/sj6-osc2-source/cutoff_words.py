"""Authentic current Swift source cutoff curve, physical Float32 deliverable."""
from pathlib import Path
p=Path('sj6-current-faders/probe.py')
exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'))
rows={}
# Direct physical conversion called at 0x1000eee3c by current setter.
e.w64(0x10082f448,0x580)
for mode in (0,1):
 e.wr(cond+0x580,bytes([mode]))
 a=[]
 for raw in [0.,.125,.25,.4,.5,.75,1.]:
  e.call(0x1000efa2c,floats=(raw,),regs={20:cond})
  a.append([f32(raw),e.d(0),f32(e.d(0))])
 rows[mode]=a
assert not e.unknown
print(json.dumps(dict(scope='Direct actual current normalized cutoff converter called by Swift sender before real .setUserCutoff: property',source=sha(BIN),cases=rows),indent=2))
