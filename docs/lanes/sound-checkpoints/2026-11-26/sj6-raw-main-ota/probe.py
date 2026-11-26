from pathlib import Path
import hashlib, tarfile, json, numpy as np,sys,ctypes as C, struct
p=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
sha=lambda z:hashlib.sha256(z).hexdigest()
assert sha(p.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(p) as t: d=json.load(t.extractfile('./program-2-dry/capture.json'))
assert d['active']==d['after']
raw={int(v['address']):v['value'] for v in d['active']}
st=Path('sj6-raw-voice-stage/dose.py').read_text().replace("for name,id in (('Main',1),('Sub',202)):","for name,id in (('Main',1),):")
marker="        # This instruction is common"
assert st.count(marker)==1
st=st.replace(marker,'''        extra=np.zeros((4,4,cue['frames']),dtype='f4')
        def op(*_):
            v=e.x(24);key=e.r32(v+0x4c8); f=at+pre_n[key]-1
            from unicorn.arm64_const import UC_ARM64_REG_S15
            fac=struct.unpack('<f',int(e.uc.reg_read(UC_ARM64_REG_S15)).to_bytes(4,'little'))[0]
            extra[:,keys.index(key),f]=[e.s(0),fac,e.rf(v+0x5f8+0x64),e.rf(v+0x5f8+0x70)]
        e.uc.hook_add(UC_HOOK_CODE,op,begin=0x1003b77e4,end=0x1003b77e4)
'''+marker)
st=st.replace('obs[name]=(x,control,by,pre)','obs[name]=(x,control,by,pre,extra)')
env={'struct':struct};exec(compile(st,'authorized current Main receiver','exec'),env)
stems,binary=env['collect'](d,raw,[],{})
original,controls,by,pre,extra=stems['Main']
prior=json.load(open('sj6-warm-raw-postvoice/result-final.json'))
assert sha(original.tobytes())==prior['source_roles']['Main']['output_sha']
lib=C.CDLL(sys.argv[1])
fun=lib.witness
fun.argtypes=[C.c_void_p,C.c_void_p,C.c_uint]
cmp=np.zeros_like(pre)
for i,k in enumerate((36,48,52,55)):
 l,r=by[k][:2]
 x=pre[i,l:r+1].copy()
 y=cmp[i,l:r+1].copy()
 fun(x.ctypes.data,y.ctypes.data,len(x));cmp[i,l:r+1]=y
assert np.isfinite(cmp).all()
def score(x,y):
 a,b=[v.astype('f8') for v in (x,y)]; n=np.linalg.norm
 return dict(rms_delta_db=20*float(np.log10(n(a)/n(b))), raw_signed_dbr=20*float(np.log10(n(a-b)/n(b))), cosine=float(a@b/n(a)/n(b)))
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
  rows[k]=dict(filter_only=score(q,y),filter_and_common_source_weight=score(q*v[1,i],y*v[1,i]),h1_phase_deg=phase,harmonics_same_input=errs,
             actual_knob_internal_range=[float(v[2,i][y!=0].min()),float(v[2,i][y!=0].max())],
             actual_res_stage_range=[float(v[3,i][y!=0].min()),float(v[3,i][y!=0].max())])
 windows[t]=rows
print(json.dumps(dict(current_source=binary, native_main_output=sha(original.tobytes()), actual_pre=sha(pre.tobytes()), compact_same_input=sha(cmp.tobytes()),note_cold_boundaries=by,
 same_input='All four actual leaves separately drive stock compact exact OTA at current Raw authored cutoff/res clamp without AMP modifications. The exact original .77e4 output and next stage s15 are captured before multiplier; no terminal fitted gain or phase.',windows=windows),indent=2,allow_nan=False))
