"""Signed chronological hold only, exact capture hashes, no fitted gain or phase.
Known fixed linear prefix/output; diagnostic not the complete original voice.
One-note envelope is settled in qualifying windows. Unknown startup/release
and any remaining whole-route gain are explicitly excluded, not simulated.
"""
import io, argparse, ctypes as c, hashlib, json, tarfile
import numpy as np
from scipy.signal import lfilter
from generate import hp
def db(x):return float(20*np.log10(x))
parser=argparse.ArgumentParser()
parser.add_argument('model')
parser.add_argument('archive')
a=parser.parse_args()
from pathlib import Path
f=c.c_float

coef=[float.fromhex(s) for s in '0x1.851d9c7caa4aap-1 0x1.851d9c7caa4aap+0 0x1.851d9c7caa4aap-1 0x1.7cb2883c57ccap+0 0x1.1b116179f9916p-1'.split()]
result=[]
with tarfile.open(a.archive) as t:
 for tf in t:
    if not tf.name.startswith('./native/native-jupiter-sine-gain-authored-band-') or not tf.name.endswith('/capture.json'):continue
    m=json.load(t.extractfile(tf))
    note=m['schedule'][0]['note']
    assert m['calibration_cue']=='jupiter-band-map-v1' and [(e['frame'],e['note']) for e in m['schedule']]==[(16384,note),(60484,note)]
    k=m['forced_controls']
    for id_,v in {'7':1,'8':0,'11':0,'17':0,'23':0,'32':1,'33':0,'40':0,'41':0,'59':0,'107':0,'108':0,'109':0,'123':0,'129':1,'131':0,'205':1,'209':0}.items():assert k[id_]==v,(id_,k[id_])
    pcm=np.load(io.BytesIO(t.extractfile(tf.name.rsplit('/',1)[0]+'/output.npy').read()),allow_pickle=False)
    assert hashlib.sha256(pcm.tobytes()).hexdigest()==m['pcm_sha256']
    shape=m['ablation_arm'].split('-')[1]
    arm={}
    where=Path(a.model)
    for k in (0,1):
        p=where/f'arm-{k}'/f'test-{note}-{int(shape=="tri")}-0.pcm'
        y=np.fromfile(p,dtype='<f4').astype('f8')
        assert len(y)>=55000-16384
        idx=np.arange(25000-16384,55000-16384)
        f0=440*2**((note-69)/12)
        raw=y[idx]
        # Typed observable already includes the actual .808 mixer input.
        z=y*(5./6)*87/127
        for b,a0 in [(coef[:3],[1,*coef[3:]]),(hp[:3],[1,*hp[3:]]),([1,-1],[1,-float.fromhex('0x1.fd70a4p-1')])]:
            z=lfilter(b,a0,z)
        q=np.array([float.fromhex('0x1.6a09e5e333598p-1'),float.fromhex('0x1.6a09e6ecb41fep-1')])*float(np.float32(.85))*float(np.float32(.404*m['actual_master']))
        z=lfilter([1,-1],[1,-float.fromhex('0x1.ff7ceep-1')],z)[idx,None]*q
        r=pcm[:,25000:55000].T.astype('f8')
        n=np.linalg.norm(r,axis=0)
        ph=np.exp(-2j*np.pi*idx*f0/44100)
        phase=np.angle(np.mean(z*ph[:,None],axis=0)/np.mean(r*ph[:,None],axis=0),deg=True)
        w=np.hanning(len(idx))
        # Relative shape is explicitly separate from signed unaltered PCM.
        # Phase-normalize each stage by its own measured H1; never align time
        # or alter amplitude for signed/absolute metrics.
        nmax=min(18,int(.45*44100/f0))
        harmonics=np.array([np.sum(x*w[:,None]*(ph[:,None]**n),axis=0)
                                 for x in [z,r] for n in range(1,nmax+1)]).reshape(2,nmax,-1)
        norm=harmonics[:,0,:]
        rel=harmonics/(abs(norm[:,None,:])*(norm[:,None,:]/abs(norm[:,None,:]))**np.arange(1,nmax+1)[None,:,None])
        shape_err=20*np.log10(np.linalg.norm(rel[0,1:]-rel[1,1:],axis=0))
        arm[str(k)]=dict(stage_sha256=hashlib.sha256(p.read_bytes()).hexdigest(),
                       stage_rms=float(np.sqrt(np.mean(raw**2))),
                       candidate_vs_native_level_db=[db(v) for v in np.linalg.norm(z,axis=0)/n],
                       candidate_vs_native_signed_residual_dbr=[db(v) for v in np.linalg.norm(z-r,axis=0)/n],
                       candidate_phase_deg=phase.tolist(),h2plus_error_relative_h1_dbr=shape_err.tolist())
    result.append(dict(note=note,shape=shape,repeat=m['repeat'],scope='actual compact producer then identical constructed source suffix; native authentic terminal hold only; not whole owner',
                       arms=arm,source_pcm_sha256=m['pcm_sha256']))
assert len(result)==26
print(json.dumps(result,indent=2))
