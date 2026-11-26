"""Prespecified seeds and separate staggered control (no native target)."""
from pathlib import Path
import hashlib,tarfile,sys,io,json,os,subprocess,numpy as np
o,b=map(Path,sys.argv[1:])
q=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
sha=lambda x:hashlib.sha256(x).hexdigest()
assert sha(q.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(q) as f:
 p='./program-1-dry/'
 r=np.load(io.BytesIO(f.extractfile(p+'output.npy').read()),allow_pickle=False)
 m=json.load(f.extractfile(p+'capture.json'))
assert sha(r.tobytes())==m['pcm_sha256'] and m['after']==m['active']
r=r.T
f=b/'sub-range.private-output/1-dry.private.txt'
lines=f.read_text().splitlines()
k=int(lines[1])+3; at=0; timings=[]
events={}
for z in lines[k:]:
 a=list(map(int,z.split()));n,count=a[:2];assert len(a)==2+2*count
 if count:events[at]=list(zip(a[2::2],a[3::2]))
 timings.append(at);at+=n
assert at==148684 and len(events[49459])==3
shifted=events[49459][:]
events[49459]=shifted[:1]
events[49523]=shifted[1:2]
events[49571]=shifted[2:]
points=sorted(set(timings+[at]+list(events)))
alt=o/'stagger.cfg'
rows=[]
for i,j in zip(points,points[1:]):
 e=events.get(i,[]);rows.append(' '.join(map(str,(j-i,len(e),*(a for p in e for a in p)))))
alt.write_text('\n'.join(lines[:k-1]+[str(len(rows))]+rows)+'\n')
def score(x,y):
 x,y=(z.astype('f8') for z in (x,y))
 return float(20*np.log10(np.linalg.norm(x-y)/np.linalg.norm(y))) if (x!=y).any() else None
d={}
for cue,plan in [('same_frame',f),('staggered',alt)]:
 for seed in ('0x71da45b3','0x243f6a88','0xa5a5a5a5','0x9e3779b9'):
  for setting in ('authored','zero'):
   env=os.environ|{'SJ6_VINTAGE_FIXTURE_SEED':seed}
   if setting=='zero':env['SJ6_VINTAGE_FIXTURE_ZERO']='1'
   states=[];pcm=[]
   for mode in (0,1):
    y=o/'seed.f32'
    p=subprocess.run([o/str(mode),plan,y],env=env,check=True,capture_output=True,text=True)
    pcm.append(np.fromfile(y,'<f4').reshape(-1,2))
    states.append([v.split()[1:] for v in p.stderr.splitlines() if v.startswith('OWNER')])
    y.unlink()
   a,c=pcm
   assert len(states[0])==len(states[1])==4
   # Raw AMP level/stage and gate at actual end of corresponding compact
   # blocks. Pitch/RNG and contributions to later filters may differ.
   assert [s[:6] for s in states[0]]==[s[:6] for s in states[1]]
   if setting=='zero':assert np.array_equal(a,c)
   old,new=(list(x[-1][6:]) for x in states)
   res={}
   for w,l,h in [('same_cue_attack',16384,20794),('first_release',38434,39139),('chord',53869,82534),('final_early',82534,83239),('final_tail',83239,84739)]:
      vals=dict(redraw_vs_retain=score(c[l:h],a[l:h]))
      if cue=='same_frame' and setting=='authored':
       vals.update(before_vs_native=score(a[l:h],r[l:h]),after_vs_native=score(c[l:h],r[l:h]))
      res[w]=vals
   d[f'{cue}:{seed}:{setting}']=dict(final_params_retained=old,final_params_drawn=new,actual_gate_and_amp_state_words_equal=True,rows=res)
print(json.dumps(d,indent=2,allow_nan=False))
