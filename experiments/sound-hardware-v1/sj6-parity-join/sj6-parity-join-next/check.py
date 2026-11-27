import ctypes as C,itertools,hashlib,json,os,sys
from pathlib import Path
import numpy as np
prefix=Path('sj6-aux-compose/check.py')
exec(compile(prefix.read_text().split("lib=C.CDLL")[0],str(prefix),'exec'))
lib=C.CDLL(str(Path('sj6-parity-join-next/owner.so').resolve()),mode=os.RTLD_LAZY)
F=C.c_float;P=C.POINTER(F)
def ptr(x):return x.ctypes.data_as(P)
class Block(C.Structure):
    _fields_=[('left',P*4),('right',P*4),('volume',F*4),('pan',F*4),('trim',F),('frames',C.c_uint)]
class Room(C.Structure):
    _fields_=[('l',P),('r',P),('frames',C.c_uint),('send',C.c_uint),('type',C.c_uint),
              ('bypass',C.c_uint),('dry_branch',C.c_uint),('mix',F),('tier',F)]
lib.lab_graph.argtypes=(C.c_void_p,C.POINTER(Block),P,P,C.c_uint,C.c_uint,C.POINTER(Room))
lib.lab_open.argtypes=(F,F,F,C.c_uint)
lib.lab_open.restype=C.c_void_p
lib.lab_close.argtypes=(C.c_void_p,)

ds=[owner(n,.25 if n=='noiseOsc' else None) for n in names]
for _ in range(128):
    for d in ds:d.render(128)
v=[np.zeros((2,150000),dtype='f4') for _ in ds]
e.rand_state=0x12345678
for stage,count,offset in (('held',22050,0),('release',8192,22050)):
    for d in ds:
        if stage=='held':e.call(note_on,d.dsp,36,86,doubles=(0.,))
        else:e.call(note_off,d.dsp,36,0)
    for start in range(0,count,128):
        n=min(count-start,128)
        for a,d in zip(v,ds):a[:,offset+start:offset+start+n]=d.render(n)
assert all(x.any() for x in (v[0],v[2],v[3]))
assert not e.unknown,e.unknown
def execute(stages,frames,sizes,direct):
    q=lib.lab_open(frames,.499,.1,stages)
    assert q
    a=np.empty((2,v[0].shape[1]),dtype='f4')
    pos=0
    # Independently provided wet fixture; not an observed post-HP Apple tap.
    wet=np.ascontiguousarray(v[2]*np.float32(.7)+v[3]*np.float32(.18))
    edges=(7213,12941,16327,a.shape[1])
    try:
      for block in itertools.cycle(sizes):
        if pos==a.shape[1]:break
        end=next(edge for edge in edges if edge>pos)
        n=min(block,end-pos)
        b=Block((P*4)(*[ptr(z[0,pos:pos+n]) for z in v]),
                (P*4)(*[ptr(z[1,pos:pos+n]) for z in v]),
                (F*4)(1,1,1,1),(F*4)(),np.float32(config['osc_section_props']['volume']),n)
        room=Room(ptr(wet[0,pos:]),ptr(wet[1,pos:]),n,int(pos<7213 or 12941<=pos<16327),2,1,1,.44,.404)
        assert lib.lab_graph(q,C.byref(b),ptr(a[0,pos:]),ptr(a[1,pos:]),n,direct,C.byref(room) if stages&8 else None)==1
        pos+=n
    finally:
      lib.lab_close(q)
    assert np.isfinite(a).all() and np.abs(a).max()>.05
    return a
rows=[]
for stages,frames in ((0,11025),(1,11025),(2,11025),(4,11025.375),(7,11025.375),(7,44100),(7,132300),(15,11025.375),(31,11025.375),(31,132300)):
    actual=execute(stages,frames,(127,79,256),0)
    ref=execute(stages,frames,(127,79,256),1)
    alternate=execute(stages,frames,(1,38,111,253),0)
    dx=actual.astype('f8')-ref
    dy=actual.astype('f8')-alternate
    rec=dict(stages=stages,delay_frames=frames,words=actual.size,peak_error=float(abs(dx).max()),
        different_words=int((actual.view('u4')!=ref.view('u4')).sum()),
        partition_peak_error=float(abs(dy).max()),reference_wiring_exact=actual.tobytes()==ref.tobytes(),
        partition_exact=actual.tobytes()==alternate.tobytes(),
        tail_rms=float(np.sqrt(np.mean(actual[:,36000:].astype('f8')**2))),
        sha=sha(actual.tobytes()))
    print(json.dumps(rec),file=sys.stderr)
    assert rec['reference_wiring_exact'] and rec['partition_exact'],rec
    if stages&4:assert rec['tail_rms']>1e-6
    if stages in (0,1,2):assert rec['tail_rms']<1e-6
    rows.append(rec)
print(json.dumps(dict(binary=kind,reference=macho.EXPECTED_SHA,rows=rows,
  scope='Real original independent oscillator receiver inputs into composed host graph. Matched against explicit same-kernel graph, not installed final output. Noise active original-receiver positive control. Does not cover full preset imports, Apple HP, nonlinear Rage or precise Apple Room front-end. Type II return+dry compressor split are wired with an explicitly separate wet fixture.',keeper='No audio emitted'),indent=2))
