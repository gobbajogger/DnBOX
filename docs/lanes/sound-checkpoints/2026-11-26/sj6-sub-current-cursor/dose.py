from pathlib import Path
import numpy as np
from unicorn import UC_HOOK_CODE
def collect(cue,raw,regions,pack):
    p=Path('sj6-modern-owner/probe.py')
    env={}
    exec(compile(p.read_text().split('def trial(')[0],'pinned current original source','exec'),env)
    Emu,SamplerDSP,BIN,cfg=[env[k] for k in ('Emu','SamplerDSP','BIN','cfg')]
    f32=lambda x:float(np.float32(x))
    def a(p,l,h,b):return f32(l+(h-l)*float(f32(raw[p]))**b)
    shared=dict(((k,v) for k,v in (
        (1,0),(2,0),(3,0),(7,0),(8,0),(9,0),(10,a(36,.002,16,3.33)),(11,0),
        (12,a(37,.002,24,3.35)),(13,a(38,0,1,2.5)),(14,0),(15,a(39,.002,24,3.35)),(20,1),(25,0),
        (28,0),(29,0),(32,1),(37,440),(38,1),(39,0),(40,0),(42,0),(43,0),(44,0),(51,1))))
    assert all(raw[k]==0 for k in (3,5,6,28,32,33,35,57,63,78,86,89,103,105,123,131,136,137,201,210,212))
    assert all(raw[k]==.5 for k in (72,74)) and np.float32(raw[82])==np.float32(2/3)
    assert np.float32(raw[206])==np.float32(.6) and raw[1]==raw[7]==raw[180]==raw[205]==raw[209]==1
    keys=(36,48,52,55); times=[16384,49459,49459,49459]; ev={}
    for q in cue['schedule']:
        k=q['note']-12
        ev.setdefault(q['frame'],[]).append((k,86 if q['action']=='on' else 0))
    obs={}
    for name,id in (('Sub',202),):
        e=Emu(str(BIN))
        from swiftrt import SwiftWorld
        w=SwiftWorld(e);e.deferred=[]
        # Actually execute Swift OTA normalized sender math. Value #7 and
        # readback #180 agree, so original note ingress cannot substitute a
        # different authored base at this same stable cue.
        cond=e.calloc(0x900);e.w64(0x10082f448,0x580);e.wr(cond+0x580,b'\1')
        e.call(0x1000efa2c,floats=(raw[7],),regs={20:cond})
        cutoff=f32(e.d(0));assert cutoff==30000.0078125
        d=SamplerDSP(e,44100)
        if name=='Main':d.load_placeholder()
        else:
            e.call(0x1003898bc,d.dsp,1)
            for g in regions:
                sr,med=pack[g['sample'].replace('\\','/').split('/')[-1]]
                K,L,H,I,J=(int(g[x]) for x in ('pitch_keycenter','lokey','hikey','loop_start','loop_end'))
                d.load_sample(dict(note=K,freq=440*2**((K-69)/12),lokey=L,hikey=H,loop=True,
                            loop_start=float(I),loop_end=float(J),start=0.,rate=sr), med.astype('f4')/32768.)
            d.build_keymap()
        for k,v in cfg['sampler_control_writes']['oscSampler1' if name=='Main' else 'sampler1'][:57 if name=='Main' else 44]:
            d.set_param(k,v,True)
        control=shared|{0:f32(.808*raw[id]),36:cutoff,41:1 if name=='Main' else 0}
        for k,v in control.items():d.set_param(k,v,True)
        if name=='Main':
            e.call(0x1003898c4,d.dsp,4);e.call(0x1003898a4,d.dsp,2)
        at=0; by={k:[None,None,None,None,0] for k in keys}; out=[]
        offsets={k:0 for k in keys}
        pre=np.zeros((4,cue['frames']),dtype='f4')
        pre_n={k:0 for k in keys}
        import struct
        sites=(0x1003b8224,0x1003b82b4,0x1003b836c,0x1003b8370,0x1003b83b0,0x1003b86ac)
        counts=dict.fromkeys(sites,0)
        rows={k:[] for k in keys}
        cursor=lambda v:struct.unpack('<d',e.rd(v+0x10,8))[0]
        def obs_pc(pc):
            def sample(*_):
                counts[pc]+=1
                k=e.r32(e.x(24)+0x4c8)
                if pc in (0x1003b836c,0x1003b8370) and k in rows and len(rows[k])<16:
                    rows[k].append([hex(pc),at,cursor(e.x(24)),e.d(0),e.d(1),e.d(2)])
                if pc==0x1003b8224:
                    assert e.x(0)==e.x(24)+8 and e.x(1)==e.r64(e.x(24)+0x28)
            return sample
        for pc in sites:e.uc.hook_add(UC_HOOK_CODE,obs_pc(pc),begin=pc,end=pc)
        def before(*_):
            key=e.r32(e.x(24)+0x4c8)
            assert key in pre_n
            f=at+pre_n[key];pre_n[key]+=1
            pre[keys.index(key),f]=e.s(0)
        e.uc.hook_add(UC_HOOK_CODE,before,begin=0x1003b77e0,end=0x1003b77e0)
        # This instruction is common audible per-voice completion, after
        # source's chosen OTA and ordinary AMP/velocity application.
        def tap(*_):
            v=e.x(24);key=e.r32(v+0x4c8)
            assert key in offsets
            f=at+offsets[key];offsets[key]+=1
            b=by[key];b[0]=f if b[0] is None else b[0];b[1]=f
            b[4]+=1
            if e.rf(v+0x5e4):b[2]=f
            if e.r32(v+0x340)==7:b[3]=f if b[3] is None else b[3]
        e.uc.hook_add(UC_HOOK_CODE,tap,begin=0x1003b7d40,end=0x1003b7d40)
        for n in cue['blocks']:
            offsets=dict.fromkeys(keys,0);pre_n=dict.fromkeys(keys,0)
            for k,v in ev.get(at,[]):e.call(0x100389c84 if v else 0x100389c8c,d.dsp,k,v,floats=(0.,))
            out.append(d.render(n).copy()); assert max(offsets.values())<=n and pre_n==offsets,(name,at,pre_n,offsets)
            at+=n
        assert at==cue['frames'] and not e.unknown
        for k,begin in zip(keys,times):
            b=by[k]
            assert b[0]==begin and b[1]-begin+1==b[4] and b[2]==b[1]-1 and b[3] in (38434,82534),(name,k,b)
        x=np.ascontiguousarray(np.concatenate(out,axis=1),dtype='f4')
        assert np.isfinite(x).all() and np.max(np.abs(x))>0
        obs[name]=(x,control,by,pre)
    return obs,env['pin'](BIN),counts,rows
