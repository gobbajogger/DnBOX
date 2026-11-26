"""Run actual installed Brainwave signed fan-in cue through typed compact.
Only receipts feed the renderer. Verify PCM separately if scoring."""
import io,sys,tarfile,hashlib,json,collections,subprocess
from pathlib import Path
import numpy as np
a,o,bin=map(Path,sys.argv[1:]);o.mkdir()
def sha(x):return hashlib.sha256(x).hexdigest()
assert sha(a.read_bytes())=='d3821515187f4c086cde7ecbff57bafbae216fbd86becd1ff356fb1fe4a5df9f'
arms={'all':{},'osc1-only':{3:0,202:0},'osc2-only':{1:0,202:0},'sub-only':{1:0,3:0}}
RR={};CC={}
def read(t,p):return t.extractfile(p).read()
spec=None
with tarfile.open(a) as t:
 for rep in (False,True):
  for arm,muted in arms.items():
   d='native-warm-brainwave-fanin-authored-'+arm+('-repeat' if rep else '')
   m=json.loads(read(t,d+'/capture.json'));x=np.load(io.BytesIO(read(t,d+'/output.npy')),allow_pickle=False)
   assert sha(x.tobytes())==m['pcm_sha256']
   assert m['installed_au_sha256']=='1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
   assert m['s4_input_sha256']=='17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943'
   v={int(k['address']):k['value'] for k in m['parameters']}
   assert m['parameters']==m['parameter_readback_after'] and x.shape==(2,148684)
   assert m['rate']==44100 and m['warm_frames']==16384 and not x[:,:16384].any()
   for i in (11,16,17,22,23,24,28,40,41,59,103,107,108,109,201):assert v[i]==0
   if spec is None:spec=(v,m['schedule'],m['buffers'])
   old_v,times,ns=spec
   assert v==old_v|muted and m['schedule']==times and m['buffers']==ns and m['repeat'] is rep
   RR[arm,rep]=x.astype('f8')
   if rep:continue
   events=collections.defaultdict(list)
   for e in times:
    assert e['action'] in ['on','off'];events[e['frame']].append((e['note'],e['velocity'] if e['action']=='on' else 0))
   lines=['120',str(len(v))]+[f'{i} {w:.9g}' for i,w in sorted(v.items())]+[str(len(ns))]
   at=0
   for n in ns:
    ev=events.pop(at,[])
    lines.append(' '.join(map(str,(n,len(ev),*(j for p in ev for j in p)))))
    at+=n
   assert not events and at==148684
   cfg=o/(arm+'.private.txt');p=o/(arm+'.private.f32')
   cfg.write_text('\n'.join(lines)+'\n')
   subprocess.run([str(bin.resolve()),str(cfg),str(p)],check=True)
   y=np.fromfile(p,'<f4').reshape(-1,2).T
   assert np.isfinite(y).all() and y.shape==x.shape and not y[:,:16384].any()
   CC[arm]=y.astype('f8')
def db(a,b):
 p,q=np.linalg.norm(a),np.linalg.norm(b)
 return 20*np.log10(p/q) if p and q else None
def score(x,y):
 return {'level_db':db(x,y),'signed_dbr':db(x-y,y)}
r={}
for k,i,j in [('held',24576,37634),('pending',49459,49875),('commit',49875,50003),('chord',53971,81434),('release',82534,115302)]:
 rn={a:RR[a,False][:,i:j] for a in arms}; cn={a:CC[a][:,i:j] for a in arms}
 ans={'windows':[i,j],
      'native_fanin':score(sum(x for a,x in rn.items() if a!='all'),rn['all']),
      'compact_fanin':score(sum(x for a,x in cn.items() if a!='all'),cn['all']),
      'arms':{}}
 for a in arms:
  ans['arms'][a]={'compact_vs_native':score(cn[a],rn[a]),
    'native_repeat':score(RR[a,True][:,i:j],rn[a]),
    'compact_vs_all':db(cn[a],cn['all']),'native_vs_all':db(rn[a],rn['all'])}
 r[k]=ans
print(json.dumps(r,indent=2,allow_nan=False))
