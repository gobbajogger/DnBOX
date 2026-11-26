from loaded import *
from scipy.signal import lfilter
import ctypes as c,sys,io,tarfile
o,b=map(Path,sys.argv[1:])
a=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
sha=lambda x:hashlib.sha256(x).hexdigest()
assert pin(a)=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
private=c.create_string_buffer(extract(Emu(str(BIN))))
lib=[]
def replay(case,bank):
 path=o/f'render-{case}-{bank}.so'
 path.write_bytes((o/'probe.so').read_bytes())
 e=c.CDLL(str(path)); lib.append(e)
 e.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
 e.witness_bank.argtypes=[c.c_void_p]
 if bank:e.witness_bank(private)
 q=b/'sub-range.private-output'/f'{case}.private.txt'
 out=o/'temp.f32'
 argv=(c.c_char_p*3)(b'probe',str(q).encode(),str(out).encode())
 assert e.run_render(3,argv)==0
 block=out.read_bytes()
 r=np.frombuffer(block,'<f4').reshape(-1,2)
 assert r.shape==(148684,2) and not r[:16384].any()
 return r,sha(block)
def s(x,y):
 x,y=x.astype('f8'),y.astype('f8')
 n=np.linalg.norm
 return {'delta_db':float(20*np.log10(n(x)/n(y))) if n(x)>0 else None,'signed_dbr':float(20*np.log10(n(x-y)/n(y))) if n(x-y)>0 else None}
def suffix(x):
 # Sensitivity only, not authentic Solidify #108 admission.
 a,b,z=[float.fromhex(t) for t in ('0x1.ffa2ap-1','0x1.10427p-19','0x1.ff4564p-1')]
 hp=lfilter([a,-2*a,a],[1,-(1+z-b),z],x,axis=0)
 cr=lfilter([1,-1],[1,-float.fromhex('0x1.fd70a4p-1')],hp,axis=0)
 return lfilter([1,-1],[1,-float(np.float32(.999))],cr,axis=0)
rows={}
for case in ('0-dry','1-dry','1-authored','2-dry'):
    y0,h0=replay(case,False); y1,h1=replay(case,True)
    f=json.load(open('sj6-solidify-sine-phase/result.json'))[case]['variants']['1:2']
    assert f['candidate_pcm_sha']==h0
    assert (h0!=h1)==case.startswith('1')
    if not case.startswith('1'):
        rows[case]={'unchanged_byte_identical':True,'sha':h0}; continue
    with tarfile.open(a) as t:
        name=f'./program-{case}/'
        m=json.load(t.extractfile(name+'capture.json'))
        r=np.load(io.BytesIO(t.extractfile(name+'output.npy').read()),allow_pickle=False)
        assert m['active']==m['after'] and m['warm_frames']==16384 and m['pcm_sha256']==sha(r.tobytes())
        r=r.T
    variants={'analytic_real_route':y0,'original_bank_real_route':y1}
    if case.endswith('dry'):
       variants.update(analytic_fixed_suffix_sensitivity=suffix(y0), original_bank_fixed_suffix_sensitivity=suffix(y1))
    ranges={'one_attack':(16384,20794),'one_hold':(20794,38434),'one_release':(38434,39139),
            'one_modal_late':(39139,40639),'chord_hold':(53869,82534),'final_early':(82534,83239),'final_modal_late':(83239,84739),'whole':(16384,148684)}
    rows[case]=dict(reference_pcm=m['pcm_sha256'],compact_arms_sha=[h0,h1],
                    windows={k:{t:s(z[l:u],r[l:u]) for t,z in variants.items()} for k,(l,u) in ranges.items()})
    if case.endswith('dry'):
        t=np.arange(8115)
        a1=float.fromhex('0x1.10427p-19'); a2=float.fromhex('0x1.ff4564p-1')
        z=next(p for p in np.roots([1,-(1+a2-a1),a2]) if p.imag>0)
        M=np.column_stack([(z**t).real,(z**t).imag,float.fromhex('0x1.fd70a4p-1')**t,float(np.float32(.999))**t])
        modal={}
        for name,at in [('one',39139),('last',83239)]:
            vv={'native':r}|{k:v for k,v in variants.items() if k.endswith('sensitivity')}
            modal[name]={}
            for k,v in vv.items():
                y=v[at:at+len(t),0].astype('f8')
                fit=np.linalg.lstsq(M[:1500],y[:1500],rcond=None)[0]
                p=M@fit
                modal[name][k]={'coords':fit.tolist(),'disjoint_tail_residual_dbr':s(p[1500:],y[1500:])['signed_dbr']}
        rows[case]['fixed_nonadmitting_modal']=modal
print(json.dumps(dict(bin_sha=pin(BIN),bank_sha=bank_sha,cases=rows),indent=2,allow_nan=False))
