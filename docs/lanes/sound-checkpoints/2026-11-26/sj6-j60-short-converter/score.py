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
from unicorn import UC_HOOK_CODE
from current import instantiate,NEW
import math,struct,os
callback_t=c.CFUNCTYPE(None,c.POINTER(c.c_float),c.POINTER(c.c_float),c.c_uint,c.c_uint)
tlib=c.CDLL(str(o/'taps.so'));scratch=(c.c_float*2)()
past=json.load(open('sj6-postvoice-split/result.json'))
def wire(leaf,n):
    e=leaf.e;taps=getattr(tlib,f'tap{n}');taps.argtypes=[c.c_void_p,c.c_void_p,c.c_float,c.c_int,c.c_void_p]
    state=[0,0,None]
    from unicorn.arm64_const import UC_ARM64_REG_LR
    def predict(*_):
      adr=e.x(0);f=e.d(0);q=math.floor(f);u=f-q
      d={0x10000300c:0,0x10000308c:1}[e.uc.reg_read(UC_ARM64_REG_LR)]
      j=(q+1-n//2)&127
      v=adr+0x10100+4*j
      taps(e.rd(v,4*n),e.rd(v+0x400,4*n),u,d,scratch)
      state[d]+=1;state[2]=d
    def substitute(d):
      def h(*_):
        assert state[2]==d
        e.sets(0,scratch[0]);e.sets(1,scratch[1]);state[2]=None
      return h
    for pc,h in [(0x10000395c,predict),(0x10000300c,substitute(0)),(0x10000308c,substitute(1))]:
      e.uc.hook_add(UC_HOOK_CODE,h,begin=pc,end=pc)
    return state
def run(a,arm):
    cfg=b/f'sub-range.private-output/2-{a}.private.txt'
    assert sha(cfg.read_bytes()) in base['controls_compact_fixture_sha']
    name=f'{a}-{arm}'
    fd=os.memfd_create(name)
    os.write(fd,(o/'center.so').read_bytes())
    p=f'/proc/self/fd/{fd}'
    lib=c.CDLL(p)
    leaf=instantiate(a=='authored') if arm!='compact' else None
    ww=wire(leaf,int(arm)) if leaf and arm.isdigit() else None
    lib.upstream.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_int,c.c_int]
    lib.upstream(true_sum.ctypes.data,148684,1,1,3)
    lib.j60_bind.argtypes=[callback_t,c.c_int]
    seen=[]; fails=[]; h=hashlib.sha256()
    @callback_t
    def probe(l,r,n,mode):
      try:
        assert mode==(2 if a=='authored' else 1)
        seen.append(n)
        L,R=(np.ctypeslib.as_array(x,shape=(n,)) for x in (l,r))
        frame=np.stack((L,R));h.update(frame.tobytes())
        if leaf:
            q=leaf.process(frame);assert np.isfinite(q).all()
            if a=='dry':assert np.array_equal(frame,q)
            L[:],R[:]=q
      except Exception as er:fails.append(repr(er))
    lib.j60_bind(probe,int(leaf is not None))
    lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
    of=os.memfd_create(name+'-pcm');f=f'/proc/self/fd/{of}';argv=(c.c_char_p*3)(b'probe',str(cfg).encode(),str(f).encode())
    assert lib.run_render(3,argv)==0 and not fails,fails[:1]
    assert seen==m['blocks'] and h.hexdigest()==past['reached'][a][3]['input_chronological_sha']
    if ww:
        want=[323664,148684] if a=='authored' else [0,0]
        assert ww==[*want,None],(ww,want)
    assert not leaf or not leaf.e.unknown
    x=np.fromfile(f,'<f4').reshape(-1,2).T.copy();os.close(of)
    assert x.shape==true_sum.shape
    return x
dry=run('dry','16');assert sha(dry.tobytes())==past['reached']['dry'][3]['output_sha']
samples={k:run('authored',k) for k in ('compact','oracle','4','8','16')}
assert sha(samples['oracle'].tobytes())==past['reached']['authored'][3]['output_sha']
def m(x,y):
    a=x.astype('f8');b=y.astype('f8')
    u=np.linalg.norm(a-b);v=np.linalg.norm(b)
    return [float(20*np.log10(u/v)) if u else None,float(20*np.log10(np.linalg.norm(a)/v))]
wins={}
for k,l,r in [('attack',16384,20794),('held',20794,38434),('first-release',38434,49459),('chord',53869,82534),('final-release',82534,104584),('full',16384,148684)]:
    ref=refs['authored'][0][:,l:r];refd=ref.astype('f8')-refs['dry'][0][:,l:r]
    base2=samples['oracle'][:,l:r].astype('f8')-dry[:,l:r]
    wins[k]={j:{'same_input_current_J60_delta':m(x[:,l:r].astype('f8')-dry[:,l:r],base2),
                 'installed_authored_absolute':m(x[:,l:r],ref),
                 'installed_authored_minus_dry':m(x[:,l:r].astype('f8')-dry[:,l:r],refd)} for j,x in samples.items()}
print(json.dumps(dict(binary=NEW, columns=['signed_dbr','level_db'], exact_current_96k_middle=True,
  source='exact currently regenerated Raw postvoice, guard physical suffix, exact physical J60 input digest and real callback schedule', signed_dry_bypass_bit_exact=True,
  clock='actual converter table and rationals unmodified, only helper returns changed',windows=wins),indent=2,allow_nan=False))
