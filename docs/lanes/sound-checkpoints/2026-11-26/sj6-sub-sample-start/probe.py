"""Original current Sub Core executable; authorized older released pack in memory only."""
from pathlib import Path
import hashlib,struct,tarfile,zipfile,io,sys,json,re
import numpy as np
from unicorn import UC_HOOK_CODE
exec(compile(Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0],'verified modern owner prefix','exec'))
ipa=next(Path('ab-private/superj6').glob('*.ipa'))
prefix='Payload/SuperJ6.app/Frameworks/SuperJ6_Assets.framework/'
with zipfile.ZipFile(ipa) as z:
    sfz=z.read(prefix+'Sounds/Osc/MySub.sfz')
    data=z.read(prefix+'J6P8R4N2S1')
assert data[:8]==b'AKJ6PCM1'
version,n=struct.unpack_from('<II',data,8); assert version==1 and n==12
cur=16; pack={}
for _ in range(n):
    ln,=struct.unpack_from('<H',data,cur);cur+=2
    f=data[cur:cur+ln].decode();cur+=ln
    frames,rate,off,nb,crc=struct.unpack_from('<IIQQI',data,cur);cur+=28
    assert nb==2*frames and off+nb<=len(data)
    pack[f.split('/')[-1]]=(rate,np.frombuffer(data,dtype='<i2',count=frames,offset=off))
regions=[];g={}
for line in sfz.decode().splitlines():
    kv=dict(re.findall(r'(\w+)=(\S+)',line))
    if line.startswith('<group>'):g=kv
    if line.startswith('<region>'):regions.append(g|kv)
assert len(regions)==12
nat=Path('sj6-direct-stage/SJ6-Brainwave-Direct-AUv3-Fanin.tgz')
assert pin(nat)=='d3821515187f4c086cde7ecbff57bafbae216fbd86becd1ff356fb1fe4a5df9f'
with tarfile.open(nat) as z:
    rows=[]
    for arm in ('sub-only','sub-only-repeat'):
        p='native-warm-brainwave-fanin-authored-'+arm+'/'
        q=json.load(z.extractfile(p+'capture.json'))
        x=np.load(io.BytesIO(z.extractfile(p+'output.npy').read()),allow_pickle=False)
        assert q['installed_au_sha256']=='1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
        assert hashlib.sha256(x.tobytes()).hexdigest()==q['pcm_sha256']
        assert q['parameters']==q['parameter_readback_after']
        rows.append((q,x))
(q,reference),(qr,repeat)=rows
assert q['buffers']==qr['buffers'] and q['schedule']==qr['schedule'] and q['parameters']==qr['parameters']
parameters={a['address']:a['value'] for a in q['parameters']}
assert all(parameters[str(k)]==0 for k in (1,3,16,17,22,23,24,28,40,41,59,103,107,108,109,201))
assert parameters['202']>0 and parameters['205']==1 and q['rate']==44100
events={}
for frame,e,m in cfg['events']:
    on=e['action']=='on'
    events.setdefault(q['warm_frames']+frame,[]).append((on,m['sampler1'] if on else (m['sampler1'],0)))
def render(use_body_start):
    e=Emu(str(BIN)); d=SamplerDSP(e,44100)
    e.call(0x1003898bc,d.dsp,1)
    for r in regions:
        key=int(r['pitch_keycenter'])
        rate,media=pack[r['sample'].replace('\\','/').split('/')[-1]]
        lo,hi=int(r['loop_start']),int(r['loop_end'])
        # Controlled ablation: omit only the incoming zone's lead-in,
        # retaining its own original body and no fitted offset or sample copy out.
        start=lo if use_body_start and key==54 else 0
        assert len(media)>hi>=lo>0
        d.load_sample(dict(note=key,freq=440*2**((key-69)/12),
                    lokey=int(r['lokey']),hikey=int(r['hikey']),loop=True,
                    loop_start=float(lo),loop_end=float(hi),start=float(start),rate=rate),
                    media.astype('f4')/32768.)
    d.build_keymap()
    for k,v in cfg['sampler_control_writes']['sampler1']:d.set_param(k,v,True)
    # The final proof only asserts compiled sampler behavior for admitted physical words.
    now=off=0; t=None; tap=[]; commit=[]; stage=[]
    def prep(*_):
        nonlocal off,t
        v=e.x(0)
        if not e.r32(v+0x340): return
        t=now+off;off+=e.x(1)
        if 49331<=t<=50131:
            tap.append([t,e.r32(v+0x4c8),e.r32(v+0x340),e.r64(v+0x28)==e.r64(v+0x5d8),
                         struct.unpack('<d',e.rd(v+0x10,8))[0],struct.unpack('<d',e.rd(v+0x18,8))[0]])
    def switch(*_):
        v=e.x(19)
        if t is not None and 49331<=t<=50131: commit.append([t,e.r32(v+0x4c8),e.r32(v+0x340),
                   struct.unpack('<d',e.rd(v+0x10,8))[0],e.r64(v+0x28)==e.r64(v+0x5d8)])
    for a,f in ((0x1003b4c54,prep),(0x1003b5160,switch)):
        e.uc.hook_add(UC_HOOK_CODE,f,begin=a,end=a)
    for n in q['buffers']:
        for on,k in events.get(now,[]):
            e.call(0x100389c84 if on else 0x100389c8c,d.dsp,*k)
        off=0; stage.append(d.render(n));now+=n
        if now>=57344:break
    assert not e.unknown
    return np.concatenate(stage,axis=1),tap,commit
base,trace,commit=render(False)
body,body_trace,body_commit=render(True)
assert commit[0][0]==body_commit[0][0]==49875
assert commit[0][1:3]==body_commit[0][1:3] and commit[0][3]!=body_commit[0][3] and commit[0][4] and body_commit[0][4]
assert base[:,:49875].tobytes()==body[:,:49875].tobytes()
def metric(u,v,i,j):
    a=u[:,i:j].astype('f8');b=v[:,i:j].astype('f8')
    r=np.linalg.norm(a);s=np.linalg.norm(b)
    if min(r,s)<1e-20:return [None,None]
    delta=np.linalg.norm(a-b)
    return [float(20*np.log10(delta/s)) if delta else None,float(np.clip((a*b).sum()/(r*s),-1,1))]
windows={}
for name,i,j in [('pending',49715,49843),('crossing',49875,50003),('post',50003,50131),('settled_chord',53971,56500)]:
    windows[name]=dict(frames=[i,j],compiled_body_vs_authored=metric(body,base,i,j),
                      modern_AUv3_repeat=metric(repeat,reference,i,j))
print(json.dumps(dict(scope='Actual current Core with older authorized exact packaged sample bytes and verified projected sampler controls. No matching to private current media identity or full AU graph. No shifts, fitting or sample export.',ipa_sha=pin(ipa),sample_container_sha=hashlib.sha256(data).hexdigest(),
    core_sha=pin(BIN),modern_app_capture_sha=q['pcm_sha256'],incoming_54_original_start=commit[0][3],
    diagnostic_body_start=body_commit[0][3],commit_frame=commit[0][0],precommit_bytes_guard=True,
    trace_slots=['frame','key','amp_stage','active_matches_pending','cursor','base_ratio'],trace=trace,windows=windows),indent=2))
