"""Exact paired native #7 cutoff vs fixed current physical source state.
Run from workspace root with unpacked prior #129-only static receipt path,
latest private oracle + old emu bench, and authorized Physical Cutoff archive.
"""
from pathlib import Path
text=Path('sj6-osc2-source/open_legacy_discriminator.py').read_text().split('baseline,taps=')[0]
exec(compile(text,'validated original source code','exec'))
P=Path('sj6-direct-stage/SJ6-Direct-AUv3-Physical-Cutoff.tgz')
assert pin(P)=='9581a0d4b230b6242723d12f2b58fd09d73f1474ea65f85cf9de3db28cf5f09b'
N={}
with tarfile.open(P) as q:
 for key in ('authored','open'):
  for rpt in ('','-repeat'):
   directory='native-warm-brainwave-osc2-physical-authored-physical-'+key+rpt+'/'
   m=json.load(q.extractfile(directory+'capture.json'))
   z=np.load(BytesIO(q.extractfile(directory+'output.npy').read()),allow_pickle=False)
   assert z.shape==(2,148684) and np.isfinite(z).all() and hashlib.sha256(z.tobytes()).hexdigest()==m['pcm_sha256']
   assert m['installed_au_sha256']==r['installed_au_sha256'] and m['schedule']==r['schedule'] and m['buffers']==r['buffers']
   assert m['parameters']==m['parameter_readback_after']
   readback={a['address']:a['value'] for a in m['parameters']}
   assert readback['7']==(p0['7'] if key=='authored' else 1)
   assert readback['129']==1
   assert readback==p0|{'7':readback['7']},[(k,v,p0.get(k)) for k,v in readback.items() if k!='7' and p0.get(k)!=v]
   N[key+rpt]=z
fixed=render_str.replace(' d.set_param(36,cutoff,True)',' d.set_param(6,10.,True)\n d.set_param(36,cutoff,True)')
exec(compile(fixed,'actual receiver fixed #7 with separate harmonic slot', 'exec'))
replays={}
for key,fc in [('authored',hz),('known_300_hz',300.),('open',20000.001953125)]:
 leaf,_=render(True,4,fc)
 replays[key]=suffix(hp10(leaf))
def corr(a,b):return float(np.dot(a.astype(float).ravel(),b.astype(float).ravel())/(rms(a)*rms(b)*a.size))
def contrast(s,o,i,j):
 return db(o[:,i:j],s[:,i:j]),db(o[:,i:j]-s[:,i:j],s[:,i:j])
O={}
windows=[(i,i+128) for i in range(16384,18560,128)]+[(19000,22000),(24576,37634),(49715,49843),(49875,50003)]
for i,j in windows:
 native,phys=contrast(N['authored'],N['open'],i,j),contrast(replays['authored'],replays['open'],i,j)
 a=N['authored'][:,i:j];b=N['open'][:,i:j];v=replays['authored'][:,i:j];w=replays['open'][:,i:j]
 O[f'{i}:{j}']=dict(native_db=native[0],native_delta_dbr=native[1],fixed_physical_db=phys[0],fixed_physical_delta_dbr=phys[1],signed_cross_difference_correlation=corr(b-a,w-v) if rms(b-a)>1e-10 else None,
  actual_repeat_dbr=contrast(N['authored'],N['authored-repeat'],i,j)[1],
  distinct_300_vs_low=contrast(replays['authored'],replays['known_300_hz'],i,j),
  authored_model_level_db=db(v,a),open_model_level_db=db(w,b))
print(json.dumps(dict(scope='No fitting: true newest native public #7 difference and actual latest fixed physical oscillator/Normal filter; optional conditional external Apple HP reference',source=pin(BIN),native_archive=pin(P),fixed_cases_hz=[hz,300.,20000.001953125],windows=O),indent=2))
