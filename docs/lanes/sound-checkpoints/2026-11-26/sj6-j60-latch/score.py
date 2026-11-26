from pathlib import Path
import hashlib,tarfile,sys,io,json,numpy as np
from emu import Emu
from j60 import J60
from unicorn import UC_HOOK_CODE
import ctypes as ct
import paths
o,b=map(Path,sys.argv[1:])
src=Path(paths.BIN)
sha=lambda x:hashlib.sha256(x).hexdigest()
assert sha(src.read_bytes())=='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
arc=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert sha(arc.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
gain=np.float32(float.fromhex((o/'route.log').read_text().split()[1]))
q=lambda p:np.fromfile(p,'<f4').reshape(-1,2).T.copy()
a,c,y=[q(o/v) for v in ('tap-0.f32','tap-1.f32','typed-out.f32')]
ns=np.fromfile(o/'blocks.i32',dtype='<u4')
assert np.sum(ns)==148684 and all(ns>0) and all(ns<=128)
assert np.array_equal(c*gain,y)
assert (o/'typed-out.f32').read_bytes()==(b/'sub-range.private-output/2-authored.private.f32').read_bytes()
coff=q(b/'sub-range.private-output/2-dry.private.f32')
assert np.array_equal(a*gain,coff)
refs={}
meta={}
with tarfile.open(arc) as f:
 for r in ('dry','authored','authored-repeat'):
    p=f'./program-2-{r}/'
    x=np.load(io.BytesIO(f.extractfile(p+'output.npy').read()),allow_pickle=False)
    m=json.load(f.extractfile(p+'capture.json'))
    assert m['active']==m['after'] and sha(x.tobytes())==m['pcm_sha256']
    refs[r]=x; meta[r]=m
assert np.array_equal(refs['authored'],refs['authored-repeat'])
read=lambda arm:{i['address']:i['value'] for i in meta[arm]['active']}
wp=read('authored');wd=read('dry')
assert {k for k in wp if wp[k]!=wd[k]}=={'40'} and [wp[k] for k in ('40','41','208','215','216','217')]==[1,0,1,.5,.5,1]
import struct
def oracle(bypass=False, inspect=False):
 e=Emu(str(src)); j=J60(e,1,1.,1.,1.,sr=44100)
 assert [j.get(i) for i in range(5)]==[1,1,1,1,0]
 if bypass:e.wr(j.obj+0x29,b'\0')
 count=changed=0; seen={}
 previous=None
 def before(*_):
    seen['addr']=e.x(8)+0x368
    seen['old']=bytes(e.rd(seen['addr'],8))
 def fresh(*_):
    # v0 contains both rounded new values; guard via integer lane register.
    from unicorn.arm64_const import UC_ARM64_REG_Q0
    seen['new']=(e.uc.reg_read(UC_ARM64_REG_Q0)&((1<<64)-1)).to_bytes(8,'little')
 def done(*_):
    nonlocal count, changed, previous
    actual=bytes(e.rd(seen['addr'],8))
    returned=struct.pack('<2f',e.s(0),e.s(1))
    assert actual==seen['new'] and returned==seen['old']
    if previous is not None: assert previous==returned
    previous=actual; count+=1; changed+=actual!=returned
 if not bypass:
    e.uc.hook_add(UC_HOOK_CODE,lambda *_:e.setx(9,0),begin=0x100005440,end=0x100005440)
 if inspect:
    for site,fn in [(0x5414,before),(0x5450,fresh),(0x545c,done)]:
       site+=0x100000000
       e.uc.hook_add(UC_HOOK_CODE,fn,begin=site,end=site)
 ret=np.empty_like(a); at=0
 for n in ns:
    ret[:,at:at+n]=j.process(a[:,at:at+n]); at+=n
 assert at==a.shape[1] and not e.unknown and np.isfinite(ret).all()
 return ret,count,changed
stopped,_,_=oracle(bypass=True)
assert np.array_equal(stopped,a)
wet,count,changed=oracle(inspect=True)
assert count==323664 and changed>0, (count,changed)
def runtime(name):
 lib=ct.CDLL(str(o/f'{name}.so')); fun=lib.witness_connected
 fun.argtypes=[ct.c_void_p,ct.c_void_p,ct.c_void_p,ct.c_uint]
 val=a.copy(); fun(val[0].ctypes.data,val[1].ctypes.data,ns.ctypes.data,len(ns))
 return val
stock,right,latch=map(runtime,('stock','right','latch'))
assert np.array_equal(stock,c)
assert not np.array_equal(right,latch)
dry=np.float32(1.3)*a
def metric(x,y):
 x,y=[v.astype('f8') for v in (x,y)]
 return 20*float(np.log10(np.linalg.norm(x-y)/np.linalg.norm(y)))
rows={}
for tag,l,r in [('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584)]:
 w=wet[:,l:r]
 rows[tag]={k:dict(signed_dbr=metric((v-dry)[:,l:r],w),
                   per_lr_dbr=[metric(a,b) for a,b in zip((v-dry)[:,l:r],w)]) 
             for k,v in [('stock',stock),('post_and_right',right),('post_right_latch',latch)]}
print(json.dumps(dict(wet_prior_vs_return_exact_channel_words=2*count,new_wet_writes_changed_frames=changed,
       old_bypass_identity=True,stock_tap_exact=True,input_sha=sha(a.tobytes()),schedule_sha=sha(ns.tobytes()),
       notes="Build9 genuine 96k wet is observed by exact register/memory tap sequence. Candidate uses old post1 and old mix at the exact update clock, no fitted time shifting or direct-path experiment. 44.1k host bridge and isolated Raw J60-I only.",rows=rows),indent=2,allow_nan=False))
