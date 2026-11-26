from pathlib import Path
import sys,re,io,tarfile,json,hashlib,numpy as np
o,b=map(Path,sys.argv[1:])
h=lambda x:hashlib.sha256(x).hexdigest()
a=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert h(a.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
def read(p,k):
    log=(o/f'{p}-{k}.txt').read_text()
    d,c,m=log.splitlines()
    suffix=int(d.rsplit('=',1)[1])
    n,clips,cp=re.fullmatch(r'classic_ring_words=(\d+) saturated=(\d+) peakCode=(.*)',c).groups()
    v=m.split()
    x=np.fromfile(o/f'{p}-{k}.f32','<f4').reshape(-1,2)
    return (x,dict(suffix=bool(suffix),instrument_master=float.fromhex(v[1]),pre_fx=max(map(float.fromhex,v[3:5])),
           pre_master=max(map(float.fromhex,v[6:8])),output=float(abs(x).max()),ring_n=int(n),ring_clips=int(clips),ring_peak_code=float(cp)))
def metric(x,y):
    x,y=(a.astype('f8') for a in (x,y))
    if not np.linalg.norm(y):return None
    return dict(level_db=float(20*np.log10(np.linalg.norm(x)/np.linalg.norm(y))) if x.any() else None,
                signed_dbr=float(20*np.log10(np.linalg.norm(x-y)/np.linalg.norm(y))) if np.any(x-y) else None)
res={}; variants=('base','center','joined')
with tarfile.open(a) as t:
 for i in range(3):
  for arm in ('dry','authored'):
   k=f'{i}-{arm}'
   p=f'./program-{k}'
   y=np.load(io.BytesIO(t.extractfile(p+'/output.npy').read()),allow_pickle=False)
   m=json.load(t.extractfile(p+'/capture.json'))
   assert m['pcm_sha256']==h(y.tobytes()) and y.shape==(2,148684)
   assert m['active']==m['after'] and m['rate']==44100 and m['warm_frames']==16384
   words={int(x['address']):x['value'] for x in m['after']}
   rejects={j:words.get(j) for j in [11,16,17,22,23,24,28,40,41,59,63,103,107,108,109,142] if words.get(j)!=0}
   arms={a:read(a,k) for a in variants}
   assert all(v['suffix']==(not rejects) for _,v in arms.values())
   same=np.array_equal(arms['center'][0],arms['joined'][0])
   if rejects:assert same
   assert (b/'sub-range.private-output'/f'{k}.private.f32').read_bytes()==arms['base'][0].tobytes()
   assert len({v['instrument_master'] for _,v in arms.values()})==1
   rr=dict(name=m['wanted']['program'],blocked_by=rejects,rejection_no_op=same if rejects else None,
           peaks={p:x[1] for p,x in arms.items()})
   if not rejects and arm=='dry':
    rr['signed']={p:{nm:metric(x[l:u],y.T[l:u]) for nm,l,u in [('whole',16384,148684),('held',20794,38434),('release',38434,49459),('chord',53869,82534)]} for p,(x,_) in arms.items()}
   res[k]=rr
stress={}
for key in [f'{e}-{n}' for e in ('bypass','classic','classic-legacy') for n in (1,6)]:
 arms={a:read(a,key) for a in variants}
 if key.startswith('classic'):
  assert not arms['joined'][1]['suffix'] and np.array_equal(arms['joined'][0],arms['center'][0])
  assert arms['joined'][1]['ring_n']==(10240 if 'legacy' in key else 20480)
 stress[key]={a:v for a,(_,v) in arms.items()}
print(json.dumps(dict(programs=res,stress=stress),indent=2,allow_nan=False))
