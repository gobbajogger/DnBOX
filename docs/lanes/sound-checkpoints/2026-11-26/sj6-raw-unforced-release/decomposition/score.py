from pathlib import Path
import io,sys,json,hashlib,tarfile,numpy as np
o,b,_=map(Path,sys.argv[1:])
p=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert hashlib.sha256(p.read_bytes()).hexdigest()=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(p) as t:
    p='./program-2-dry/'
    r=np.load(io.BytesIO(t.extractfile(p+'output.npy').read()),allow_pickle=False).T
    m=json.load(t.extractfile(p+'capture.json'))
assert hashlib.sha256(r.T.copy().tobytes()).hexdigest()==m['pcm_sha256'] and m['active']==m['after']
f=lambda n,k:np.fromfile(o/n,'<f4').reshape(-1,k)
src,actual=f('tap.f32',6)[:,:2],f('end.f32',2)
gain=np.float32(float.fromhex((o/'tail.txt').read_text().splitlines()[-1].split()[1]))
assert src.shape==actual.shape==r.shape==(148684,2)
center=np.array(src*gain,dtype='<f4',order='C')
noteoff=38434
# All four states propagated from the actual pre-HP zero-origin waveform;
# unchanged HP and three remaining real dynamic state variables. Implement
# in F64 for transparent additive component attribution; check vs F32 replay.
def run(tape):
    from scipy.signal import lfilter
    one=np.float64
    a=one(float.fromhex('0x1.ffa2ap-1'));b=one(float.fromhex('0x1.10427p-19'));c=one(float.fromhex('0x1.ff4564p-1'))
    hp=lfilter([a,-2*a,a],[1,-(1+c-b),c],tape,axis=0)
    cr=lfilter([1,-1],[1,-float.fromhex('0x1.fd70a4p-1')],hp,axis=0)
    return lfilter([1,-1],[1,-float(np.float32(.999))],cr*gain,axis=0)
a=src.astype('f8');pre=a.copy();pre[noteoff:]=0
env=a.copy();env[:noteoff]=0;env[49459:]=0
baseline,ring,drive=map(run,(a,pre,env))
level=lambda x:float(np.sqrt(np.mean(x*x)))
db=lambda x,y:float(20*np.log10(level(x)/level(y))) if level(x)>0 and level(y)>0 else None
out={}
for k,i,j in [('release',38434,49459),('0-16ms',38434,39139),('16-50ms',39139,40639),('50-100ms',40639,42844),('100-200ms',42844,47254)]:
    ys=r[i:j];y=actual[i:j]; full=baseline[i:j];h=ring[i:j];d=drive[i:j]
    assert db(full-h-d,full)<-130 and db(full-y,y)<-62
    yr=dict(rms_native=level(ys),rms_center_input=level(center[i:j]),rms_full=level(y),rms_inherited_only=level(h),rms_compact_release_only=level(d),
        inherited_only_signed_vs_native_dbr=db(h-ys,ys),total_actual_vs_native_dbr=db(y-ys,ys),
        drive_only_signed_vs_native_dbr=db(d-ys,ys),float_replay_error_dbr=db(full-y,y),
        drive_vs_inherited_dbr=db(d,h),corr_inherited_and_drive=float(np.vdot(h,d)/(np.linalg.norm(h)*np.linalg.norm(d))),
        corr_inherited_native=float(np.vdot(h,ys)/(np.linalg.norm(h)*np.linalg.norm(ys))),corr_drive_native=float(np.vdot(d,ys)/(np.linalg.norm(d)*np.linalg.norm(ys))))
    out[k]=yr
ct=x=int(np.flatnonzero(np.any(center[noteoff:49459]!=0,axis=1))[-1]+noteoff)
# Deliver only generated compact data, no authenticated original trace.
tr=Path('/tmp/SJ6-Raw-Release-Generated-Compact');tr.mkdir(exist_ok=True)
a0,a1=34324,49459
cwt={}
for n,x in [('center-only',center),('center-plus-hp-dc',actual)]:
    d=x[a0:a1].tobytes(order='C'); name=n+'.f32'
    (tr/name).write_bytes(d); cwt[name]=hashlib.sha256(d).hexdigest()
records=dict(schema='interleaved little-endian stereo Float32 at 44100',abs_begin_frame=a0,abs_end_excluded_frame=a1,noteoff_abs_frame=noteoff,last_nonzero_compact_abs_frame=ct,PCM_SHA256=cwt)
(tr/'manifest.json').write_text(json.dumps(records,indent=2)+'\n')
print(json.dumps(dict(windows=out,witness=records,master=float(gain)),indent=2,allow_nan=False))
