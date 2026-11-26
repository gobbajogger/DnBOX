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
st=st.replace(marker,'''        extra=np.zeros((8,4,cue['frames']),dtype='f4')
        def op(*_):
            v=e.x(24);key=e.r32(v+0x4c8); f=at+pre_n[key]-1
            from unicorn.arm64_const import UC_ARM64_REG_S15
            fac=struct.unpack('<f',int(e.uc.reg_read(UC_ARM64_REG_S15)).to_bytes(4,'little'))[0]
            extra[:,keys.index(key),f]=[e.s(0),fac,e.rf(v+0x5f8+0x64),e.rf(v+0x5f8+0x70),e.rf(v+0x5f8+0x5c),e.rf(v+0x5f8+0x74),e.rf(v+0x5f8+0x80),e.rf(v+0x5e4)]
        e.uc.hook_add(UC_HOOK_CODE,op,begin=0x1003b77e4,end=0x1003b77e4)
'''+marker)
st=st.replace('obs[name]=(x,control,by,pre)','obs[name]=(x,control,by,pre,extra,[e.rf(a) for a in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)])')
env={'struct':struct};exec(compile(st,'authorized current Main receiver','exec'),env)
stems,binary=env['collect'](d,raw,[],{})
original,controls,by,pre,extra,coeff=stems['Main']
prior=json.load(open('sj6-warm-raw-postvoice/result-final.json'))
if not alternate: assert sha(original.tobytes())==prior['source_roles']['Main']['output_sha']

ans={}
for i,k in enumerate((36,48,52,55)):
 l,r,last,off,ct=by[k]
 a=extra[7,i];y=extra[0,i]
 assert a[last]!=0 and a[r]==0 and not a[r+1:].any()
 z=a[off:r+1]
 ans[k]=dict(key=k,first=l,last=r,count=ct,release_amp_hash=sha(z.tobytes()),gate_off=off,last_nonzero_amp=last,
 signed_amp_at_gate=z[:6].tolist(),
 signed_amp_close=[(n,float(a[n]),float(y[n])) for n in range(off+550,r+1)],
 signed_derivatives=np.diff(z.astype('f8'))[:20].tolist())
print(json.dumps(ans,indent=2))
