from pathlib import Path
import hashlib, tarfile, json, numpy as np,sys,ctypes as C, struct
p=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
sha=lambda z:hashlib.sha256(z).hexdigest()
assert sha(p.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(p) as t: d=json.load(t.extractfile('./program-2-dry/capture.json'))
assert d['active']==d['after']
raw={int(v['address']):v['value'] for v in d['active']}
st=Path('sj6-raw-voice-stage/dose.py').read_text().replace("for name,id in (('Main',1),('Sub',202)):","for name,id in (('Main',1),):")
alternate=len(sys.argv)>4 and sys.argv[4]=='lower'
if alternate:
 assert raw[7]==raw[180]==1
 raw[7]=raw[180]=float(np.float32(.8))
 st=st.replace('raw[1]==raw[7]==raw[180]==raw[205]==raw[209]==1','raw[1]==raw[205]==raw[209]==1 and raw[7]==raw[180]')
 st=st.replace('assert cutoff==30000.0078125','assert 5500<cutoff<29000')
marker="        # This instruction is common"
assert st.count(marker)==1
st=st.replace(marker,'''        extra=np.zeros((7,4,cue['frames']),dtype='f4')
        def op(*_):
            v=e.x(24);key=e.r32(v+0x4c8); f=at+pre_n[key]-1
            from unicorn.arm64_const import UC_ARM64_REG_S15
            fac=struct.unpack('<f',int(e.uc.reg_read(UC_ARM64_REG_S15)).to_bytes(4,'little'))[0]
            extra[:,keys.index(key),f]=[e.s(0),fac,e.rf(v+0x5f8+0x64),e.rf(v+0x5f8+0x70),e.rf(v+0x5f8+0x5c),e.rf(v+0x5f8+0x74),e.rf(v+0x5f8+0x80)]
        e.uc.hook_add(UC_HOOK_CODE,op,begin=0x1003b77e4,end=0x1003b77e4)
'''+marker)
st=st.replace('obs[name]=(x,control,by,pre)','obs[name]=(x,control,by,pre,extra,[e.rf(a) for a in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)])')
env={'struct':struct};exec(compile(st,'authorized current Main receiver','exec'),env)
stems,binary=env['collect'](d,raw,[],{})
original,controls,by,pre,extra,coeff=stems['Main']
prior=json.load(open('sj6-warm-raw-postvoice/result-final.json'))
if not alternate: assert sha(original.tobytes())==prior['source_roles']['Main']['output_sha']
lib=C.CDLL(sys.argv[1])
fun=lib.witness
fun.argtypes=[C.c_void_p,C.c_void_p,C.c_uint]
cmp=np.zeros_like(pre);model=cmp.copy()
shared=C.CDLL(sys.argv[2]); phy=shared.physical_replay
old=C.CDLL(sys.argv[3]).physical_replay
phy.argtypes=old.argtypes=[C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p,C.c_uint]
coeff=np.array(coeff,dtype='f4')
observed=[]; exact_words=0; pairs=[]

for i,k in enumerate((36,48,52,55)):
 l,r=by[k][:2]
 x=pre[i,l:r+1].copy()
 y=cmp[i,l:r+1].copy()
 fun(x.ctypes.data,y.ctypes.data,len(x));cmp[i,l:r+1]=y
 g,rz,h,blend,trim=extra[2:,i,l:r+1]
 v=np.stack((g,h,blend,trim))
 assert np.all(rz==0) and np.all(v==v[:,[0]]), (i,v[:,0],v[:,-1])
 pp=v[:,0].copy();observed.append(pp.tolist())
 yy=np.zeros_like(x)
 phy(x.ctypes.data,yy.ctypes.data,pp.ctypes.data,coeff.ctypes.data,len(x))
 t0=np.zeros_like(yy)
 old(x.ctypes.data,t0.ctypes.data,pp.ctypes.data,coeff.ctypes.data,len(x))
 exact_words+=int(np.count_nonzero(yy.view('u4')==t0.view('u4')))
 pairs.append((yy.copy(),t0))
 model[i,l:r+1]=yy
assert np.isfinite(cmp).all() and np.isfinite(model).all()
def score(x,y):
 a,b=[v.astype('f8') for v in (x,y)]; n=np.linalg.norm
 r=n(a-b);return dict(rms_delta_db=20*float(np.log10(n(a)/n(b))), raw_signed_dbr=20*float(np.log10(r/n(b))) if r else None, cosine=float(a@b/n(a)/n(b)))
windows={}
for t,l,r in [('attack',16384,20794),('held',20794,38434),('release',38434,38993),('chord',53869,82534),('last_release',82534,83093)]:
 # Control all upstream variation; both process genuine exact per-voice source input. Double is metric-only.
 v=extra[:,:,l:r]; pre_f=pre[:,l:r]
 rows={}
 for i,k in enumerate((36,48,52,55)):
  y=v[0,i]; q=cmp[i,l:r]
  if not np.any(y): continue
  # Remove no gain from sign metrics; report coherent H1 phase on settled windows.
  h=np.exp(-2j*np.pi*np.outer(np.arange(1,9),np.arange(l,r))*440*2**((k-69)/12)/44100)
  hq,hy=h@q,h@y
  errs=[{'angle_deg':float(np.angle(a/b,deg=True)),'amp_db':20*float(np.log10(abs(a/b)))} for a,b in zip(hq,hy)]
  phase=errs[0]['angle_deg']
  rows[k]=dict(phys=score(model[i,l:r],y),filter_only=score(q,y),filter_and_common_source_weight=score(q*v[1,i],y*v[1,i]),h1_phase_deg=phase,harmonics_same_input=errs,
             actual_knob_internal_range=[float(v[2,i][y!=0].min()),float(v[2,i][y!=0].max())],
             actual_res_stage_range=[float(v[3,i][y!=0].min()),float(v[3,i][y!=0].max())])
 windows[t]=rows
calls={k:getattr(shared,k+'_calls')() for k in ('compact','fallback','bad')}
assert calls['bad']==0 and calls['compact']+calls['fallback']>123515
from scipy.signal import hilbert
aa,bb=[np.concatenate([p[i] for p in pairs]).astype('f8') for i in (0,1)]
delta=aa-bb
rel=lambda x,y:float(20*np.log10(np.linalg.norm(x)/np.linalg.norm(y))) if np.linalg.norm(x) else None
quality=dict(whole_signed_vs_exact_dbr=rel(delta,bb),level_delta_vs_exact_db=rel(aa,bb),peak_abs_error=float(np.max(np.abs(delta))),signed_bias=float(delta.mean()),max_near_dc_signed_window_bias=float(np.max(np.abs(np.convolve(delta,np.ones(256)/256,mode='valid')))))
shared.reached_tanh_peak.restype=C.c_float
print(json.dumps(dict(reached_tanh_peak=shared.reached_tanh_peak(),vs_exact=quality,calls=calls,current_source=binary, native_main_output=sha(original.tobytes()), actual_pre=sha(pre.tobytes()), compact_same_input=sha(cmp.tobytes()),note_cold_boundaries=by,physical_cutoff_hz=controls[36],alternate=alternate,physical_controls=observed, packed_vs84b_word_exact=exact_words,
 same_input='All four actual leaves separately drive stock compact exact OTA at current Raw authored cutoff/res clamp without AMP modifications. The exact original .77e4 output and next stage s15 are captured before multiplier; no terminal fitted gain or phase.',windows=windows),indent=2,allow_nan=False))
