from pathlib import Path
x=Path('recovered-sound/sj6-sub-handoff/shared-clock/native_matrix.py').read_text()
x=x.split('actual=[]')[0]
exec(compile(x,'pinned current source sampler','exec'))
q=' def prep(*_):'
assert fn.count(q)==1
fn=fn.replace(q,''' anchor=[]
 def prep(*_):''')
a='  if relative>=33075-16:'
assert fn.count(a)==1
fn=fn.replace(a,'''  if relative>=33075 and e.r32(v+0x340)==1:
   anchor.append([relative,struct.unpack('<d',e.rd(v+0x308,8))[0],
                           struct.unpack('<d',e.rd(v+0x320,8))[0]])
'''+a)
fn=fn.replace('return dict(who=who','return dict(anchor=anchor,who=who')
exec(compile('def alternate('+fn,'current source actual binary64 stage-1 state','exec'))
out=[]
for label,lead in [('held',0),('midrelease',11025),('released',22050),('staggered',11025)]:
 r=alternate('sampler1',lead,'staggered' if label=='staggered' else 'default')
 prep=r['anchor']
 lastarm=33235 if label=='staggered' else 33075
 ys=[p for p in prep if lastarm<=p[0]<=r['commit'][0][0]]
 assert len(ys) in (27,28)
 frames=[p[0] for p in ys]
 assert frames[-1]==r['commit'][0][0]
 y=ys[0][1];step=ys[0][2]
 assert step==-y/27 and all(p[2]==step for p in ys)
 for i,(_,a,d) in enumerate(ys):
  assert y==a,(label,i,y,a)
  y+=step
  assert (y<=0)==(i==len(ys)-1),(label,i,y)
 out.append(dict(case=label,anchor=ys[0][1].hex(),step=step.hex(),frames=frames,commit=frames[-1],retargets=[[r[0],r[1].hex(),r[2].hex()] for r in prep if r[0] in (33075,33155,33235)]))
print(json.dumps(dict(cases=out,sha=pin(BIN)),indent=2))
