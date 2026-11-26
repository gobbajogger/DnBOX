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
prior=json.load(open('sj6-warm-raw-postvoice/result-final.json'))
assert all(sha(stems[x][0].tobytes())==prior['source_roles'][x]['output_sha'] for x in stems)
def run(arm):
    cfg=b/'sub-range.private-output/2-dry.private.txt'
    assert sha(cfg.read_bytes()) in base['controls_compact_fixture_sha']
    lines=cfg.read_text().splitlines()
    assert lines[2]=='0 0.849336267'
    q=lines.copy()
    id_,raw_expected={'Main':(202,raw[202]),'Sub':(1,raw[1])}[arm]
    j=next(i for i in range(2,2+int(lines[1])) if lines[i].split()[0]==str(id_))
    assert np.float32(float(lines[j].split()[1]))==np.float32(raw_expected)
    q[j]=f'{id_} 0'
    conf=o/f'{arm}.cfg';conf.write_text('\n'.join(q)+'\n')
    p=o/f'{arm}.so';p.write_bytes((o/'center.so').read_bytes())
    lib=c.CDLL(str(p)); lib.upstream.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_int]
    lib.upstream(true_sum.ctypes.data,148684,0,0)
    tr=np.zeros((4,148684),dtype='f4')
    lib.audit_begin.argtypes=[c.c_void_p,c.c_uint];lib.audit_begin(tr.ctypes.data,tr.shape[1])
    lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
    f=o/f'{arm}.f32';argv=(c.c_char_p*3)(b'probe',str(conf).encode(),str(f).encode())
    assert lib.run_render(3,argv)==0
    assert lib.reached_frames()==148684 and lib.audit_frames()==148684
    assert not tr[2:].any() and np.isfinite(tr).all()
    f.unlink()
    return tr[:2]
def db(a,b):return float(20*np.log10(a/b))
def opt(x,y):
    x,y=x.astype('f8'),y.astype('f8')
    rx,ry=(np.linalg.norm(z) for z in (x,y));assert rx>0 and ry>0
    return dict(rms=[float(z/np.sqrt(len(x))) for z in (rx,ry)],level_db=db(rx,ry),signed_dbr=db(np.linalg.norm(x-y),ry),signed_cos=float(x@y/(rx*ry)))
def harmonics(x,f,l,r):
    return np.array([2*np.mean(x[l:r]*np.exp(-2j*np.pi*np.arange(l,r)*f*h/44100)) for h in range(1,9)])
def hscore(x,y,f,l,r):
    h,g=harmonics(x,f,l,r),harmonics(y,f,l,r)
    return dict(h1_level_db=db(abs(h[0]),abs(g[0])),h1_phase_deg=float(np.angle(h[0]/g[0],deg=True)),
                source_harmonics_dbc=[db(abs(i),abs(g[0])) for i in g],
                compact_harmonics_dbc=[db(abs(i),abs(h[0])) for i in h])
cases={}
for arm in ('Main','Sub'):
    pre,post=run(arm)
    orig,_,by,orig_pre=stems[arm];orig=orig[0]
    ref_pre=np.float32(orig_pre[0]+orig_pre[1]+orig_pre[2]+orig_pre[3])
    windows={}
    for k,l,r in (('attack',16384,20794),('held',20794,38434),('release',38434,39032),('chord',53869,82534),('last-release',82534,83132)):
       windows[k]={'pre':opt(pre[l:r],ref_pre[l:r]),'post':opt(post[l:r],orig[l:r])}
       if k=='held':
        for z,x,y in [('pre',pre,ref_pre),('post',post,orig)]:windows[k][z]['harmonic']=hscore(x,y,440*2**((36-(12 if arm=='Sub' else 0)-69)/12),l,r)
    cases[arm]=dict(prehash=sha(ref_pre.tobytes()),tap_sha=sha(np.stack([pre,post]).tobytes()), windows=windows)
print(json.dumps(dict(first_stage_comparison='Actual compact solo carrier into OTA and standalone exact current sampled/generated receiver at raw normalized owner projection, current signed post-OTA same app outputs reproduced. Pre is not same fader/AMP ownership before physical placement. No terminal PCM used as stage source.', cases=cases),indent=2))
