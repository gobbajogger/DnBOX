from pathlib import Path
import ctypes as c, hashlib, io, json, math, re, sys, tarfile, zipfile,struct
import numpy as np
o,b=map(Path,sys.argv[1:])
# Reuse authorized matching sample capsule decoder, no compiled app import.
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('ipa=')[1].split('nat=Path(')[0]
exec(compile('ipa='+s,'existing private SFZ container decoder','exec'))
sha=lambda z:hashlib.sha256(z).hexdigest()
assert sha(data)=='ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740'
assert sha(sfz)=='3668ef83c102b4952a19bc6fd5adc585eb69d0eff4dd4743be9488b0771fc75d'
arc=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert sha(arc.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(arc) as t:
 def take(a):
  p=f'./program-2-{a}/'
  m=json.load(t.extractfile(p+'capture.json'))
  r=np.load(io.BytesIO(t.extractfile(p+'output.npy').read()),allow_pickle=False).copy()
  assert sha(r.tobytes())==m['pcm_sha256'] and m['active']==m['after'] and m['frames']==r.shape[1]==148684
  return r,m
 refs=dict((a,take(a)) for a in ('authored','dry','authored-repeat'))
assert np.array_equal(refs['authored'][0],refs['authored-repeat'][0])
from dose import collect
m=refs['dry'][1]
raw={int(v['address']):v['value'] for v in m['active']}
stems,binary=collect(m,raw,regions,pack)
true_sum=np.float32(stems['Main'][0]+stems['Sub'][0])
assert true_sum.shape==(2,148684) and not true_sum[:,:16384].any()
base=json.load(open('sj6-j60-warm-center/result.json'))
def admit(m):
 v={int(x['address']):x['value'] for x in m['active']}
 required={0:raw[0],11:0,16:0,17:0,22:0,23:0,24:0,28:0,41:0,59:0,63:0,89:0,95:0,99:0,103:0,107:0,108:0,109:0,142:0,201:0,208:1,215:.5,216:.5,217:1}
 return m['rate']==44100 and all(v.get(k)==q for k,q in required.items()) and v.get(40) in (0,1)
for a in ('authored','dry','authored-repeat'):assert admit(refs[a][1])
for k in (28,103,107,108,109):
 import copy
 bad=copy.deepcopy(refs['dry'][1]);next(z for z in bad['active'] if int(z['address'])==k)['value']=.5
 assert not admit(bad)
from current import instantiate, NEW
changed=set(k for k,v in raw.items() if v!=next(z['value'] for z in refs['authored'][1]['active'] if int(z['address'])==k))
assert changed=={40} and raw[40]==0
assert all(refs[a][1]['blocks']==m['blocks'] for a in refs)
prior=json.load(open('sj6-warm-postvoice-physical/result.json'))
assert all(sha(stems[x][0].tobytes())==prior['source_roles'][x]['output_sha'] for x in stems)
callback_t=c.CFUNCTYPE(None,c.POINTER(c.c_float),c.POINTER(c.c_float),c.c_uint,c.c_uint)
def run(a,replace):
    cfg=b/f'sub-range.private-output/2-{a}.private.txt'
    assert sha(cfg.read_bytes()) in base['controls_compact_fixture_sha']
    name=f'{a}-{int(replace)}'
    p=o/f'{name}.so';p.write_bytes((o/'center.so').read_bytes())
    lib=c.CDLL(str(p)); leaf=instantiate(a=='authored') if replace else None
    lib.upstream.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_int]
    lib.upstream(true_sum.ctypes.data,148684,1,1)
    lib.j60_bind.argtypes=[callback_t,c.c_int]
    modes=[]; ns=[]; fails=[]; captures=[hashlib.sha256(),hashlib.sha256()]
    @callback_t
    def probe(l,r,n,mode):
      try:
        assert mode==(2 if a=='authored' else 1),(a,mode)
        ns.append(n);modes.append(mode)
        L,R=(np.ctypeslib.as_array(x,shape=(n,)) for x in (l,r))
        frame=np.stack([L,R]);captures[0].update(frame.tobytes())
        if replace:
            q=leaf.process(frame); assert np.isfinite(q).all()
            if a=='dry': assert np.array_equal(frame,q)
            else: L[:],R[:]=q
            captures[1].update(q.tobytes())
      except Exception as err:
        fails.append(repr(err))
    lib.j60_bind(probe,replace)
    lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
    f=o/f'{name}.f32';argv=(c.c_char_p*3)(b'probe',str(cfg).encode(),str(f).encode())
    assert lib.run_render(3,argv)==0 and not fails,fails[:1]
    cs=(c.c_uint*3)();lib.physical_counts(cs)
    assert list(cs)==[148684]*3 and lib.reached_frames()==148684
    assert sum(ns)==148684 and ns==m['blocks'],(sum(ns),len(ns),len(m['blocks']),next(((i,j,k) for i,(j,k) in enumerate(zip(ns,m['blocks'])) if j!=k),None))
    if leaf:assert not leaf.e.unknown,leaf.e.unknown
    x=np.fromfile(f,'<f4').reshape(-1,2).T.copy();f.unlink()
    assert x.shape==true_sum.shape and np.isfinite(x).all()
    rows=dict(mode=modes[0],partitions=len(ns),frames=sum(ns),input_chronological_sha=captures[0].hexdigest(),replaced_chronological_sha=captures[1].hexdigest() if replace else None,output_sha=sha(x.tobytes()))
    return x,rows
ans={}
for a in ('dry','authored'):
    ans[a]=[run(a,on) for on in (False,True)]
    assert ans[a][0][1]['input_chronological_sha']==ans[a][1][1]['input_chronological_sha']
assert np.array_equal(ans['dry'][0][0],ans['dry'][1][0])
def rms(x):return float(np.sqrt(np.mean(x*x)))
def metric(x,y):
    x,y=x.astype('f8'),y.astype('f8')
    u=rms(x-y)
    return dict(signed_dbr=20*np.log10(u/rms(y)) if u else None,level_db=20*np.log10(rms(x)/rms(y)))
win={}
for k,l,r in [('attack',16384,20794),('held',20794,38434),('first-release',38434,49459),('chord',53869,82534),('final-release',82534,104584),('full',16384,148684)]:
  refauth,refdry=(refs[a][0][:,l:r] for a in ('authored','dry'))
  w=win[k]={}
  for a,ref in [('authored',refauth),('dry',refdry)]:
    vv=[metric(x[:,l:r],ref) for x,_ in ans[a]]
    for p in vv:assert np.isfinite(p['signed_dbr'])
    old=prior['windows'][k][a][2]
    assert all(abs(vv[0][k]-val)<1e-6 for k,val in old.items())
    w[a]=vv
  dv=[(x[:,l:r].astype('f8')-y[:,l:r].astype('f8')) for (x,_),(y,_) in zip(ans['authored'],ans['dry'])]
  refd=refauth.astype('f8')-refdry.astype('f8')
  w['authored_minus_dry']=[{**metric(x,refd),'mid':metric(x.mean(axis=0),refd.mean(axis=0)),'side':metric(x[0]-x[1],refd[0]-refd[1])} for x in dv]
  w['actual_local_replacement_delta']=metric(ans['authored'][1][0][:,l:r],ans['authored'][0][0][:,l:r])
print(json.dumps(dict(engine_sha=NEW, same_words_and_partitions=True, dry_bypass_word_exact=True, reference_hashes={x:refs[x][1]['pcm_sha256'] for x in refs},
  arms=['physical local suffix with corrected compact J60','same with reached whole authentic current Mac ARM64 J60V2 leaf instead'],
  controls={'known_raw_J60_profile':{'40':1,'41':0,'208':1,'215':.5,'216':.5,'217':1},'authored_physical_leaf_in_call_order':[1,1,1,1,0]},
  reached={a:[r for _,r in ans[a]] for a in ans},windows=win),allow_nan=False,indent=2))
