from pathlib import Path
import hashlib,tarfile,sys,io,json, numpy as np
o,b=map(Path,sys.argv[1:])
digest=lambda x:hashlib.sha256(x).hexdigest()
arc=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert digest(arc.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
refs={}; meta={}
with tarfile.open(arc) as f:
 for a in ('dry','authored','authored-repeat'):
    p=f'./program-2-{a}/'
    x=np.load(io.BytesIO(f.extractfile(p+'output.npy').read()),allow_pickle=False)
    m=json.load(f.extractfile(p+'capture.json'))
    assert m['active']==m['after'] and digest(x.tobytes())==m['pcm_sha256']
    refs[a]=x; meta[a]=m
assert np.array_equal(refs['authored'],refs['authored-repeat'])
def read_fixture(a):
 p=b/f'sub-range.private-output/2-{a}.private.txt'
 z=p.read_text().split();n=int(z[1]);t=z[2:2+2*n]
 kv=dict(zip(map(int,t[::2]),map(float,t[1::2])))
 return kv,z[2+2*n:],digest(p.read_bytes())
ia,ta,sha1=read_fixture('authored'); id_,td,sha2=read_fixture('dry')
diff={i:(ia[i],id_[i]) for i in ia if ia[i]!=id_[i]}
assert ia.keys()==id_.keys() and ta==td and diff=={40:(1.,0.)}
# Stage scheduling includes exact original warm prefix and MIDI event frames.
at=0;q=iter(map(int,ta[1:])); sched=[]; parts=[]
for _ in range(int(ta[0])):
 n,k=next(q),next(q); parts.append(n)
 for e in range(k): sched.append([at,next(q),next(q)])
 at+=n
assert at==148684
for a,m in meta.items():
 assert m['blocks']==parts and m['frames']==at and m['rate']==44100 and m['warm_frames']==16384
 assert sched==[[e['frame'],e['note'],e['velocity']] for e in m['schedule']]
 assert m['cue_sha256']==meta['authored']['cue_sha256'] 
ref=lambda a:{int(i['address']):i['value'] for i in meta[a]['active']}
ra,rd=map(ref,('authored','dry'))
assert {i for i in ra if ra[i]!=rd[i]}=={40}
for i in (40,41,208,215,216,217):assert ia[i]==ra[i] and id_[i]==rd[i]
data={};logs={}
for arm in ('stock','post','both'):
 for a in ('authored','dry'):
  k=f'{arm}-{a}'
  x=np.fromfile(o/f'{k}.f32',dtype='<f4').reshape(-1,2).T
  assert x.shape==(2,148684) and np.isfinite(x).all()
  data[k]=x;logs[k]=' '.join((o/f'{k}.log').read_text().split()[:2])
assert np.array_equal(data['stock-dry'],data['both-dry']) and np.array_equal(data['post-dry'],data['both-dry'])
def rms(a):return float(np.sqrt(np.mean(a*a)))
def dbr(x,y):return float(20*np.log10(x/y)) if x and y else None
windows=[('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584),('full',16384,148684)]
result={}
for w,l,h in windows:
 rd_,ra_=[refs[a][:,l:h].astype('f8') for a in ('dry','authored')]
 result[w]={}
 for arm in ('stock','post','both'):
  c,d=[data[f'{arm}-{a}'][:,l:h].astype('f8') for a in ('authored','dry')]
  delta=c-d;refdelta=ra_-rd_
  result[w][arm]={
    'authored_signed_dbr':dbr(rms(c-ra_),rms(ra_)),
    'dry_signed_dbr':dbr(rms(d-rd_),rms(rd_)),
    'authored_level_vs_ref_db':dbr(rms(c),rms(ra_)),
    'dry_level_vs_ref_db':dbr(rms(d),rms(rd_)),
    'authored_vs_stock_absolute_dBFS':dbr(rms(c-data['stock-authored'][:,l:h]),1.),
    'actual_condition_delta_vs_actual_AU_dbr':dbr(rms(delta-refdelta),rms(refdelta)),
    'absolute_ref_error_dBFS':dbr(rms(c-ra_),1.)}
keys=set(ia)&set(ra)
assert keys==set(id_)&set(rd)
assert all(np.float32(ia[i])==np.float32(ra[i]) and np.float32(id_[i])==np.float32(rd[i]) for i in keys)
print(json.dumps(dict(meta_keys=list(meta['authored']), original_sha={a:meta[a]['pcm_sha256'] for a in refs}, compact_sha={a:digest(z.tobytes()) for a,z in data.items()},
   dry_flags_bit_identical=True,authored_repeat_bit_identical=True,schedule=sched, matching_native_cue_sha256=meta['authored']['cue_sha256'], exact_partitions=len(parts),controls_compact_fixture_sha=[sha1,sha2],
   shared_normalized_binary32_exact_for_both_arms=len(keys),
   compact_only_normalized_ids=sorted(set(ia)-set(ra)), native_only_normalized_ids=sorted(set(ra)-set(ia)),
   scene_trace=logs,windows=result),allow_nan=False,indent=2))
