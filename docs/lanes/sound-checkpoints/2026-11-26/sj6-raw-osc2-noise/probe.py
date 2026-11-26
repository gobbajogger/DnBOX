from pathlib import Path
import numpy as np
p=Path('sj6-current-faders/probe.py')
exec(compile(p.read_text().split('cases={}')[0],str(p),'exec'))
for k,v in dict(LOADDATA=0x10038988c,BUILDKEYMAP=0x100389c54).items():setattr(sampler,k,v)
raw_archive=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
assert sha(raw_archive)=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
rp={}
with tarfile.open(raw_archive) as t:
 for arm in ('dry','authored','authored-repeat'):
    d=json.load(t.extractfile(f'./program-2-{arm}/capture.json'))
    assert d['active']==d['after']
    rp[arm]=({int(v['address']):f32(v['value']) for v in d['active']},d['pcm_sha256'])
assert all(r[3]==0 and r[201]==0 for r,_ in rp.values())
assert rp['authored']==rp['authored-repeat']
src=json.loads(Path('recovered-sound/checkpoint/dnbox-sound-bench/sj6/brainwave-normal-sub/four-sources.json').read_text())
gain=[p for p in defs if p[0] in ('OSC2','Noise')]
from unicorn import UC_HOOK_CODE
rows={}
for label,pid,pc,g,top,exp in gain:
 d=next(d for k,d in dsp.values() if k==label)
 e.call(0x1003898bc,d.dsp,1)
 # Real modern sampler receiver, deterministic deliberately synthetic marker.
 y=(.45*np.sin(np.arange(4096)*.043)).astype('f4')
 d.load_sample(dict(note=36,freq=130.81278265,lokey=0,hikey=127,loop=True,loop_start=0.,loop_end=4095.,start=0.,rate=44100.),y)
 d.build_keymap()
 for k,v in {0:0,1:0,2:0,3:0,7:0,8:0,9:0,10:.002,11:0,12:1,13:.8,14:0,15:.1,20:0,25:0,28:0,29:0,32:1,36:20000,37:440,38:1,39:0,40:0,41:0,42:0,43:0,44:0,51:1}.items(): d.set_param(k,v,True)
 # Warm actual owner first, then control transition. No rerouting or summing.
 e.call(0x100389c84,d.dsp,36,86)
 def receive(raw):
    calls.clear()
    e.call(pc,floats=(raw,),regs={20:cond})
    assert calls==[(label,0,f32(top*pow(raw,exp)))],calls
    return calls[0][2]
 def render(n):
    a=np.asarray([d.render(128) for _ in range(n)])
    assert np.isfinite(a).all()
    return a
 pos=receive(.5)
 p0=render(64)
 assert np.max(np.abs(p0))>.01
 actual=receive(rp['dry'][0][int(pid)])
 z=render(64)
 # Parameter ramp allowed only as disclosed operational transition, not saved-cold state.
 zz=np.max(np.abs(z),axis=(1,2))
 last=max(np.flatnonzero(zz),default=-1)
 assert actual==0. and not np.any(z[-32:])
 receive(.5)
 p1=render(64)
 assert np.max(np.abs(p1))>.01
 rows[label]=dict(sender=hex(pc),parameter=int(pid),authored_and_dry_exact_word=actual,positive_probe=pos,
     positive_peak=float(np.max(np.abs(p0))),retarget_last_nonzero_128_block=int(last),settled_mute_exact_stereo_zero_frames=4096,reattack_peak=float(np.max(np.abs(p1))))
assert not e.unknown,e.unknown
print(json.dumps(dict(executable=sha(BIN),source_capture=sha(raw_archive),original_terminal_sha={k:r[1] for k,r in rp.items()},result=rows,
 scope='actual-current native normalized sender -> actual Swift sampler setter -> actual compiled Core receiver and active synthetic source marker. No full installed internal PCM or unrelated modulation claim. Existing verified native four-lane source summing is separate.'),indent=2))
