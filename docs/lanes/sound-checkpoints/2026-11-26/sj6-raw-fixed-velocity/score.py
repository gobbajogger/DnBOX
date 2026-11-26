from pathlib import Path
import hashlib,tarfile,sys,io,json,numpy as np
o,b=map(Path,sys.argv[1:])
arc=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
sha=lambda x:hashlib.sha256(x).hexdigest()
assert sha(arc.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
def d(x,y):return float(20*np.log10(np.linalg.norm(x)/np.linalg.norm(y))) if np.any(x) and np.any(y) else None
rows={}
for arm in ('dry','authored'):
    assert (o/f'0-no-{arm}.f32').read_bytes()==(o/f'1-no-{arm}.f32').read_bytes()
    with tarfile.open(arc) as z:
      p=f'./program-2-{arm}/'
      ref=np.load(io.BytesIO(z.extractfile(p+'output.npy').read()),allow_pickle=False)
      m=json.load(z.extractfile(p+'capture.json'))
      assert m['active']==m['after'] and sha(ref.tobytes())==m['pcm_sha256']
      if arm=='authored':
        a=np.load(io.BytesIO(z.extractfile('./program-2-authored-repeat/output.npy').read()),allow_pickle=False)
        assert np.array_equal(a,ref)
      c={x['address']:x['value'] for x in m['after']}
      assert c['205']==1 and c['86']==0 and c['1']==1 and c['3']==0 and c['209']==1
    p=[np.fromfile(o/f'{i}-{arm}.f32','<f4').reshape(-1,2).astype('f8') for i in (0,1)]
    assert p[0].shape==p[1].shape==ref.T.shape
    base,after=(o/f'{i}-{arm}.log' for i in (0,1))
    assert base.read_text().split()[1]==after.read_text().split()[1]
    r={}
    for k,l,h in [('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('final_tail',82534,104584),('whole',16384,148684)]:
       x,y,ref0=p[0][l:h],p[1][l:h],ref.T[l:h]
       r[k]=dict(after_vs_before_level_db=d(y,x),before_signed_dbr=d(x-ref0,ref0),after_signed_dbr=d(y-ref0,ref0))
    rows[arm]=dict(ref_pcm=m['pcm_sha256'],source_osc1_public=1.,source_osc2_public=0.,physical_taper_scope='Default 1 taper, no key≥60 after real Range transposition',fixture_dry_dynamic_unchanged=True,windows=r)
print(json.dumps(rows,indent=2,allow_nan=False))
