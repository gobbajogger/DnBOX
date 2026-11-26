"""Independent holdout for original before-vs-after chord cutoff restore."""
from pathlib import Path
pre=Path('sj6-osc2-source/score_live_latch.py').read_text().split('out={}')[0]
pre=pre.replace("steps={24576:1. if name=='open' else orig['7'],61440:orig['7']} if layout=='live' else {}","steps={m['frame']:m['raw'] for m in r['physical_steps']}")
exec(compile(pre,'same existing comparison with declared captured control events','exec'))
A=Path('sj6-direct-stage/SJ6-AUv3-Chord-Cutoff-Timing.tgz')
assert pin(A)=='1bec364fa3118259d1f666af72dabaa322eb135f4a264538fb212eacbf8b02d3'
data={}
with tarfile.open(A) as t:
 for i,case in enumerate(('before','after')):
  data[case]={}
  for name in ('authored','open'):
   for rep in ('','-repeat'):
    p=f'sj6-{case}-chord/native-warm-brainwave-osc2-live-authored-physical-{name}{rep}/'
    m=json.load(t.extractfile(p+'capture.json'))
    x=np.load(BytesIO(t.extractfile(p+'output.npy').read()),allow_pickle=False)
    assert m['schedule']==identity['schedule'] and m['installed_au_sha256']==identity['installed_au_sha256']
    assert m['rate']==44100 and m['warm_frames']==16384 and x.shape==(2,148684)
    assert hashlib.sha256(x.tobytes()).hexdigest()==m['pcm_sha256'] and np.isfinite(x).all()
    assert {w['address']:w['value'] for w in m['parameters']}==orig
    assert {w['address']:w['value'] for w in m['parameter_readback_after']}==orig
    assert [(w['frame'],w['raw']) for w in m['physical_steps']]==[(49331,1. if name=='open' and not i else orig['7']),(49587,1. if name=='open' else orig['7']),(70000,orig['7'])]
    for q in m['physical_steps']:assert {w['address']:w['value'] for w in q['full_readback']}==orig|{'7':q['raw']}
    data[case][name+rep]=(m,x)
assert all(m['buffers']==data['before']['authored'][0]['buffers'] for d in data.values() for m,x in d.values())
out={}
for case,items in data.items():
 models={}
 for arm in ('authored','open'):
  r=items[arm][0]
  models[arm],steps=model(case,arm,True)
  print(case,arm,steps,file=sys.stderr)
 out[case]={}
 for n,lo,hi in [('before',48000,49331),('near_crossing',49459,49875),('whole_chord_held',54000,69872),('restored',72000,80000)]:
  a,b,rep=[items[k][1][:,lo:hi] for k in ('authored','open','authored-repeat')]
  c,d=[models[k][:,lo:hi] for k in ('authored','open')]
  out[case][n]={'native':metrics(b,a),'model':metrics(d,c),'authored_absolute':metrics(c,a),'native_repeat':metrics(rep,a)}
print(json.dumps({'source':pin(BIN),'native_capture':pin(A),'score':out,'scope':'Source-derived comparison-only timing holdout; exact actual ARM leaf replay and actual native independent held difference, no fit.'},indent=2))
