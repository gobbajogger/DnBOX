"""Test source-derived empty-held-note cutoff restoration against direct Mac static and dynamic cases."""
from pathlib import Path
import json
from cutoff_latch_model import LiveCutoff
text=Path('sj6-osc2-source/open_legacy_discriminator.py').read_text().split('baseline,taps=')[0]
exec(compile(text,'verified typed source stages and same-input custody','exec'))
STATIC=Path('sj6-direct-stage/SJ6-Direct-AUv3-Physical-Cutoff.tgz')
DYN=Path('sj6-direct-stage/SJ6-Direct-AUv3-Dynamic-Cutoff.tgz')
assert pin(STATIC)=='9581a0d4b230b6242723d12f2b58fd09d73f1474ea65f85cf9de3db28cf5f09b'
assert pin(DYN)=='6f007e4e30abd5c91b11a1013a24b2d66e18412c874b4c368192d5b4743c1413'
identity=r
orig=p0
def native(src,layout):
 arms={}
 for name in ('authored','open'):
  for repeat in ('','-repeat'):
   pre=f'native-warm-brainwave-osc2-{layout}-authored-physical-{name}{repeat}/'
   with tarfile.open(src) as t:
    target=np.load(BytesIO(t.extractfile(pre+'output.npy').read()),allow_pickle=False)
    m=json.load(t.extractfile(pre+'capture.json'))
   assert m['installed_au_sha256']==identity['installed_au_sha256'] and m['schedule']==identity['schedule'] and sum(m['buffers'])==148684
   assert m['rate']==44100 and m['warm_frames']==16384 and target.shape==(2,148684)
   assert hashlib.sha256(target.tobytes()).hexdigest()==m['pcm_sha256'] and np.isfinite(target).all()
   before,after=[{d['address']:d['value'] for d in m[k]} for k in ('parameters','parameter_readback_after')]
   assert before==after==orig|{'7':float(layout=='physical' and name=='open') or orig['7']}
   if layout=='live':
    reality=m['physical_steps']
    assert [(ev['frame'],ev['raw']) for ev in reality]==[(24576,1. if name=='open' else orig['7']),(61440,orig['7'])]
    for q in reality:
     projection={d['address']:d['value'] for d in q['full_readback']}
     assert projection==before|{'7':q['raw']}
   arms[name+repeat]={'pcm':target,'receipt':m}
 assert all(q['receipt']['buffers']==arms['authored']['receipt']['buffers'] for q in arms.values())
 return arms
pfix=render_str
pfix=pfix.replace(' d.set_param(36,cutoff,True)',' d.set_param(6,10.,True)\n d.set_param(36,cutoff,True)')
pfix=pfix.replace("  frame=n-r['warm_frames']","  timeline(n,nframes,d)\n  frame=n-r['warm_frames']")
pfix=pfix.replace("  if n>=r['warm_frames']+33075+4096:break","  if n>=82944:break")
exec(compile(pfix,'actual generated OSC2 and documented source-based cutoff owner glue','exec'))
cfemu=Emu(str(BIN));cfworld=SwiftWorld(cfemu);cfcond=cfemu.calloc(0x800)
cfemu.w64(0x10082f448,0x580);cfemu.wr(cfcond+0x580,b'\0')
def cv(raw):
 cfemu.call(0x1000efa2c,floats=(raw,),regs={20:cfcond})
 return float(np.float32(cfemu.d(0)))
obs=json.loads(Path('sj6-osc2-source/note_cutoff_latch.json').read_text())
assert obs['source']==pin(BIN) and obs['actual_enum']==7
def model(layout,name,fix):
 """Candidate scheduler is actual offline block elapsed time, never optimized."""
 val=1. if layout=='physical' and name=='open' else orig['7']
 live=LiveCutoff(val,orig['180'],obs['actual_enum'],obs['actual_delay'])
 # Redundant ensure mode is never inferred from expected output.
 current=cv(val); prev=0.; states=[]
 steps={24576:1. if name=='open' else orig['7'],61440:orig['7']} if layout=='live' else {}
 notes={}
 for m in r['schedule']:notes.setdefault(m['frame'],[]).append(m)
 def clock(at,n,d):
  nonlocal current,prev
  z=live.tick(prev) if fix else None
  if at in steps:
   t=steps[at]; z=live.set_public(t) if fix else t
   states.append(dict(frame=at,action='write',public=t,target=live.target if fix else t))
  for m in notes.get(at,[]):
   if fix and live.note(m['note'],m['action']=='on'):
    states.append(dict(frame=at,action='first-note-restore',public=live.public,target=live.target))
  if z is not None:
   physical=cv(z)
   if physical!=current:d.set_param(36,physical,True);current=physical
  prev=n/44100
 globals()['timeline']=clock
 leaf,_=render(True,4,current)
 return suffix(hp10(leaf)),states
def metrics(x,y):
 return {'level_db':db(x,y),'null_dbr':db(x-y,y)}
out={}
windows=dict(attack=(16384,18432),before_live_step=(20000,24576),first_step=(24576,26624),
  held_before_chord=(30000,37634),near_crossing=(49715,49843),owner_crossing=(49875,50131),
  held_after_chord=(53971,61440),after_restore=(64000,80000))
for fixture,source in [('physical',STATIC),('live',DYN)]:
 n=native(source,fixture);r=n['authored']['receipt']
 outputs={f:{a:model(fixture,a,f) for a in ('authored','open')} for f in (False,True)}
 scores={}
 for label,(i,j) in windows.items():
  a,b,re=[n[k]['pcm'][:,i:j] for k in ('authored','open','authored-repeat')]
  fresh={}
  for f in (False,True):
   p,q=[outputs[f][k][0][:,i:j] for k in ('authored','open')]
   fresh['latch' if f else 'base_no_latch']={'vs_authored':metrics(p,a),'vs_open':metrics(q,b),'model_contrast':metrics(q,p),'signed_contrast_null_dbr':db((q-p)-(b-a),b-a)}
  scores[label]={'native_contrast':metrics(b,a),'authored_native_repeat':metrics(re,a),**fresh}
 out[fixture]={'state_events':{a:outputs[True][a][1] for a in ('authored','open')},'scores':scores}
print(json.dumps(dict(scope='Clean-room source-derived filter owner replay, original signed native refs, actual current compiled source OSC2 through conditional HP/suffix; real frame tape dt, not actual Mac scheduler evidence; no fit.',original_sha=pin(BIN),results=out),indent=2))
