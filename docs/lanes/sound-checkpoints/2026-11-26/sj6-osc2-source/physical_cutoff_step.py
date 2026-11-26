"""Compare two actual compiled filter input states, no fit or new native reference."""
from pathlib import Path
text=Path('sj6-osc2-source/open_legacy_discriminator.py').read_text().split('baseline,taps=')[0]
exec(compile(text,'validated original source code','exec'))
fixed=render_str.replace(' d.set_param(36,cutoff,True)',' d.set_param(6,10.,True)\n d.set_param(36,cutoff,True)')
exec(compile(fixed,'independent harmonic slot correctly retained','exec'))
a,_=render(True,4,hz)
b,_=render(True,4,20000.001953125)
st={}
for w,i,j in [('attack',16384,18432),('held',24576,37634),('near_crossing',49715,49843),('crossing',49875,50003)]:
 x,y=a[:,i:j],b[:,i:j]
 st[w]={'actual_leaf_cutoff_static_change_db':db(y,x),'actual_leaf_cutoff_static_delta_dbr':db(y-x,x)}
print(json.dumps(dict(scope='Actual modern DSP #36 fixed lower vs wide physical cutoff, same OSC2 readbacks, shape and cue; no private full app route asserted.',source=pin(BIN),low_hz=hz,open_hz=20000.001953125,windows=st),indent=2))
