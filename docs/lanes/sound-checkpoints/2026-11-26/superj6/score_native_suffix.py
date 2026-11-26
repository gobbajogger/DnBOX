#!/usr/bin/env python3
"""Compare two exact native-leaf ends, explicitly not a full parity claim."""
from pathlib import Path
import ctypes as C,hashlib,json,tarfile,sys
from io import BytesIO
import numpy as np
if len(sys.argv)!=2:
    raise SystemExit("score_native_suffix.py native-chain.private.tgz")
D=Path(__file__).resolve().parent
def sha(x): return hashlib.sha256(x).hexdigest()
refpath=D.parent/'SJ6-Brainwave-Warm-Main-Sub.native.private.tgz'
assert sha(refpath.read_bytes())=='9db19abc7c56183e745e1f3de150559b319dabda4817ebd30af9a9db356c4b53'
with tarfile.open(refpath) as f:
    arm='native-warm-brainwave-main-only/'
    r=json.load(f.extractfile(arm+'capture.json'))
    ref=np.load(BytesIO(f.extractfile(arm+'output.npy').read()),allow_pickle=False)
    assert sha(ref.tobytes())==r['pcm_sha256'] and r['rate']==44100 and ref.shape==(2,148684)
    assert r["parameters"]==r["parameter_readback_after"]
    raw={str(x["address"]):x["value"] for x in r["parameters"]}
    assert all(raw[str(k)]==0 for k in (11,16,17,22,23,24,28,40,41,59,103,107,201,202))
native=Path(sys.argv[1])
assert sha(native.read_bytes())=='7457978e9374d93763144e467579981dd9269a487fbb151b47a6e3c2c4d5f136'
tape=tarfile.open(native)
def get(name): return tape.extractfile(name).read()
manifest={}
for line in get("results.sha256").decode().splitlines():
    h,name=line.split(maxsplit=1)
    assert sha(get(name))==h
    manifest[name]=h
require_prefix={'before':'2c88b98af86f8b870489dec68706b83f1718f9fb318d7da80207d5fed7ca13a9',
           'candidate':'1d91604fda38b2cc8aaf0146375f24152ce1a899fdb54e0b939ca9f3c1d4a1d3'}
lib=C.CDLL(str(D/'suffix/settled_suffix.so'))
f=lib.sj6_settled_postcrusher
ptr=np.ctypeslib.ndpointer(dtype=np.float32,ndim=2,flags='C_CONTIGUOUS')
f.argtypes=[ptr,ptr,C.c_uint,C.POINTER(C.c_uint),C.c_uint,C.c_float]
blocks=(C.c_uint*len(r['buffers']))(*r['buffers'])
def connected(which):
    hpbase=f'{which}-hpf'
    crbase=f'{which}-crusher'
    hp=json.loads(get(hpbase+'/capture.json'))
    cr=json.loads(get(crbase+'/capture.json'))
    bindings=[json.loads(get(f"{which}-{stage}-bind.json")) for stage in ('hpf','crusher')]
    for leaf,bind,role,kind,wanted in [
        (hp,bindings[0],hpbase,'aufx/hpas/appl',require_prefix[which]),
        (cr,bindings[1],crbase,'aufx/dist/appl',manifest[hpbase+'/hpf-out.f32'])
    ]:
        assert leaf['au_component']==kind and leaf['node_bypass'] is False and leaf['au_latency_sec']==0
        assert leaf['source_pcm_sha256']==leaf['input_file_sha256']==leaf['input_pcm_sha256']==leaf['bypass_pcm_sha256']==wanted
        assert leaf['frames']==148684 and leaf['sample_rate']==44100 and bind['source_pcm_sha256']==wanted
        assert leaf['tape_binding_sha256']==manifest[role+'-bind.json']
        assert leaf['ordered_block_sizes']==leaf['block_frames']==r['buffers']
        assert leaf['scheduled_events']==r['schedule'] and leaf['native_before']==leaf['native_after']
    assert hp['native_before']=={'0':10,'1':0}
    assert bindings[1]['parent_hpf_receipt_sha256']==manifest[hpbase+'/capture.json']
    assert cr['forced_parameter_values']=={'3':.44387006759643555,'4':0,'5':0,'15':0}
    assert all(cr['native_before'][k]==v for k,v in cr['forced_parameter_values'].items())
    x=get(crbase+'/crusher-out.f32')
    assert sha(x)==cr['connected_wet_port_pcm_sha256']==cr['output_pcm_sha256']
    x=np.frombuffer(x,dtype='<f4').reshape((2,148684))
    assert np.isfinite(x).all() and not x[:,:16384].any()
    y=np.empty_like(x)
    assert f(x,y,r['frames'],blocks,len(blocks),raw['0'])==1
    return y
a=connected('before');b=connected('candidate')
def ratio(p,q):return float(20*np.log10(max(1e-30,p)/max(1e-30,q)))
def metrics(x,q):
    x=x.astype(np.float64);q=q.astype(np.float64)
    v=np.linalg.norm(x);z=np.linalg.norm(q)
    spec=lambda w:20*np.log10(np.maximum(1e-12,np.abs(np.fft.rfft(w*np.hanning(w.shape[1])))))
    sx,sq=spec(x),spec(q)
    m=sq>=sq.max()-50
    return {'level_db':ratio(v,z),'null_dbr':ratio(np.linalg.norm(x-q),z),
            'spectral_db':float(np.sqrt(np.mean(np.square(sx[m]-sq[m])))),
            'correlation':float(np.sum(x*q)/(v*z)) if v and z else None}
windows=[('attack',16384,18432),('held',24576,37634),('first_release',38434,49459),
         ('pending',49459,49875),('after_handoff',49875,53971),('chord',53971,81434),
         ('final_release',82534,115302),('late_tail',115302,148684)]
out={'scope':'single scale-order A/B through actual Apple HP+Crusher and verified static dry suffix; partial upstream source equivalence; no fit or normalize',
     'windows':{},'output_digests':{'before':sha(a.tobytes()),'candidate':sha(b.tobytes())}}
for name,l,h in windows:
    out['windows'][name]={'frames':[l,h],
       'before_vs_original':metrics(a[:,l:h],ref[:,l:h]),
       'candidate_vs_original':metrics(b[:,l:h],ref[:,l:h]),
       'candidate_vs_before':metrics(b[:,l:h],a[:,l:h])}
print(json.dumps(out,indent=2))
