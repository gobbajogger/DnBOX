"""Match new direct Mac static-open receipts to the actual compiled OSC2 leaf.
Usage: python sj6-osc2-source/open_match.py /private/capture/directory.
"""
import sys
if len(sys.argv)!=2:raise SystemExit(__doc__)
CAP=__import__('pathlib').Path(sys.argv[1])
defs=__import__('pathlib').Path('sj6-osc2-source/transition.py').read_text().split('res={}')[0]
a=defs.index('with tarfile.open(ARCH)');b=defs.index('def render(',a)
defs=defs[:a]+defs[b:]
exec(compile(defs,'validated standalone OSC2 plus guarded suffix declarations','exec'))
def read(repeat):
 q=CAP/('native-warm-brainwave-osc2-static-authored-osc2-only'+('-repeat' if repeat else ''))
 m=json.loads((q/'capture.json').read_text())
 y=np.load(q/'output.npy',allow_pickle=False)
 assert m['installed_au_sha256']=='1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
 assert m['s4_input_sha256']=='17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943'
 assert m['osc2_static_writes']=={'87':0,'95':0,'99':0,'129':1,'131':0,'136':0,'137':0}
 assert m['parameters']==m['parameter_readback_after']
 c={k['address']:k['value'] for k in m['parameters']}
 assert all(c[k]==v for k,v in m['osc2_static_writes'].items())
 assert (c['3'],c['0'],c['209'],c['130'])==(1,.3150142729282379,0,.4141378700733185)
 assert not any(c[str(k)] for k in (1,11,16,17,22,23,24,28,40,41,59,103,107,108,109,201,202))
 assert y.shape==(2,148684) and m['rate']==44100 and m['warm_frames']==16384 and m['repeat'] is repeat
 assert hashlib.sha256(y.tobytes()).hexdigest()==m['pcm_sha256']
 assert np.isfinite(y).all() and not y[:,:16384].any()
 return y,m,c
x,r,p0=read(False);y,n,p1=read(True)
assert p0==p1 and all(r[a]==n[a] for a in ('schedule','buffers'))
with tarfile.open(ARCH) as t:
 old=json.load(t.extractfile('native-warm-brainwave-fanin-authored-osc2-only/capture.json'))
 old_pcm=np.load(BytesIO(t.extractfile('native-warm-brainwave-fanin-authored-osc2-only/output.npy').read()),allow_pickle=False)
 assert old['schedule']==r['schedule'] and old['buffers']==r['buffers']
 cv={k['address']:k['value'] for k in old['parameters']}
 assert set(p0)==set(cv) and all(p0[k]==cv[k] for k in cv if k not in r['osc2_static_writes'])
# Run current physical slot values from app setter and exact tested hardcoded
# documented table, not normalized values treated as Hz or tuned to capture.
cut=json.loads(Path('sj6-osc2-source/cutoff_words.json').read_text())
assert cut['source']==pin(BIN)
hz=cut['cases']['0'][-1][2];assert hz==20000.001953125
render_str=defs[defs.index('def render('):defs.index('def rms(')]
# Explicitly change three static filter receivers and turn off both DSP
# pitch LFO paths; retain per-layer rates, fader, ADSR, glide and wave.
render_str=render_str.replace(' d.set_param(36,cutoff,True)',' for slot,value in ((7,0.),(39,0.),(40,0.),(2,0.),(4,0.)):d.set_param(slot,value,True)\n d.set_param(36,cutoff,True)')
exec(compile(render_str,'source-bound static receiver control variant','exec'))
leaf,taps=render(True,4,hz)
z=suffix(leaf);zh=suffix(hp10(leaf))
out={}
for name,i,j in [('attack',16384,18432),('held',24576,37634),('before_chord',49331,49459),('near_crossing',49715,49843),('crossing',49875,50003)]:
 target=x[:,i:j];a=z[:,i:j];b=zh[:,i:j];rr=y[:,i:j]
 corr=lambda a,b:float(np.dot(a.astype(float).ravel(),b.astype(float).ravel())/(rms(a)*rms(b)*a.size))
 out[name]=dict(source_db=db(a,target),source_null_dbr=db(a-target,target),source_cor=corr(a,target),model_hpf_db=db(b,target),model_hpf_null_dbr=db(b-target,target),native_repeat_null=db(rr-target,target),native_open_vs_authored_db=db(target,old_pcm[:,i:j]))
print(json.dumps(dict(scope='matched actual current normalized static/open OSC2 capture and original current guest source, ideal or approximate external Apple HP, no real internal buses or sample corrections.',received_physical_cutoff_hz=hz,results=out),indent=2))
