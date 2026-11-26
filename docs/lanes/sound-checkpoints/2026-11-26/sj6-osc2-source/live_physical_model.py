"""No-fit current source response to predeclared OSC2 physical #7 step/no-op."""
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
fixed=fixed.replace("  frame=n-r['warm_frames']","  if n in changes:d.set_param(36,changes[n],True)\n  frame=n-r['warm_frames']")
fixed=fixed.replace('  if n>=r[\'warm_frames\']+33075+4096:break','  if n>=65536:break')
exec(compile(fixed,'static modern DSP response to physical parameter writes','exec'))
def model(c):
 global changes
 changes=c
 leaf,_=render(True,4,hz)
 return suffix(hp10(leaf))
a=model({24576:hz,61440:hz});b=model({24576:20000.001953125,61440:hz})
assert a[:,:24576].tobytes()==b[:,:24576].tobytes()
D={}
for L,H in [(24576,24704),(24576,26624),(26624,28672),(30000,37634),(49459,49875),(49875,50131),(57000,61000),(61440,63488),(63500,65536)]:
 x,y=a[:,L:H],b[:,L:H]
 D[f'{L}:{H}']={'step_level_db':db(y,x),'step_waveform_delta_dbr':db(y-x,x)}
print(json.dumps(dict(scope='Actual compiled modern Normal receiver, live physical address36 exact steps, with separate guarded candidate HP/suffix, no Mac app timeline inference or amplitude fitting.',source=pin(BIN),steps=[24576,61440],windows=D),indent=2))
