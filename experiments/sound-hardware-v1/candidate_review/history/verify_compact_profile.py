#!/usr/bin/env python3
"""Independent bounded equivalence review of the explicitly selected compact profile."""
import ctypes as C, datetime, hashlib, json, pathlib, random, shutil, struct, subprocess
from verify_candidate import Env, Params, Work, Api, bind, raw, WORK_NAMES

HERE=pathlib.Path(__file__).resolve().parent
BASE=HERE.parent
OUT=HERE/'compact_profile_review'
EXPECTED={'dnbx_core.c':'3d8f898428b297478abb35692de56952451caed38cc02db53e16a19a54a7408b',
          'dnbx_core.h':'86acaf17ee9c48dac151919098ab12de3acf457fab2bd9fdf57ac966f36c0962'}
FIELDS=[('amp.'+n,*b) for n,b in zip(('attack_s','hold_s','decay_s','sustain','release_hold_s','release_s'),((0,16),(0,16),(0,16),(0,1),(0,16),(0,16)))]
FIELDS += [('filter.'+n,*b) for n,b in zip(('attack_s','hold_s','decay_s','sustain','release_hold_s','release_s'),((0,16),(0,16),(0,16),(0,1),(0,16),(0,16)))]
FIELDS += [('waveform',0,1),('pulse_width',.03,.5),('gain',0,4),('sub_gain',0,.5),('cutoff_hz',20,20000),('resonance',0,1),('filter_env_hz',-20000,20000),('lfo_hz',0,16),('lfo_pwm_depth',0,.47),('tune_semitones',-48,48),('pan',-1,1)]
assert len(FIELDS)==23
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
def bits(x):return struct.unpack('<I',struct.pack('<f',x))[0]
def value(b):return struct.unpack('<f',struct.pack('<I',b))[0]
def nextbits(x,positive):
    b=bits(x)
    if value(b)==0:return 1 if positive else 0x80000001
    return b+(1 if (value(b)>0)==positive else -1)
def putword(p,index,b):C.memmove(C.addressof(p)+4*index,struct.pack('<I',b),4)
def snapshot(p):return raw(p)

def compile_all():
    OUT.mkdir();sources=OUT/'snapshot';sources.mkdir()
    original=BASE/'hardware_candidate/size_variants/compact_profile'
    for n,h in EXPECTED.items():
        assert sha(original/n)==h,(n,sha(original/n),h)
        shutil.copy2(original/n,sources/n)
    shutil.copy2(HERE/'frozen_03/snapshot/fastmath.h',sources/'fastmath.h')
    commands=[];libs={}
    for name,src,extra in [('accepted',HERE/'frozen_03/snapshot',[]),('diagnostic',sources,[]),('compact',sources,['-DDNBX_DIAGNOSTIC_COUNTERS=0'])]:
        path=OUT/(name+'.so')
        argv=['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-fno-fast-math','-ffp-contract=off','-DX0X_HOST','-fPIC','-shared']+extra+[str(src/'dnbx_core.c'),'-o',str(path)]
        cp=subprocess.run(argv,text=True,capture_output=True);commands.append({'argv':argv,'exit_code':cp.returncode});(OUT/(name+'.build.log')).write_text(cp.stdout+cp.stderr);cp.check_returncode()
        libs[name]=bind(path)
        if name!='accepted':
            fn=libs[name].dnbx_diagnostic_counters_enabled;fn.restype=C.c_uint32;fn.argtypes=[]
    # Reject a nonsensical build profile explicitly rather than silently pick a mode.
    argv=['cc','-std=c11','-fno-fast-math','-ffp-contract=off','-DX0X_HOST','-DDNBX_DIAGNOSTIC_COUNTERS=2','-c',str(sources/'dnbx_core.c'),'-o',str(OUT/'invalid_profile.o')]
    cp=subprocess.run(argv,text=True,capture_output=True);commands.append({'argv':argv,'exit_code':cp.returncode});(OUT/'invalid_profile.build.log').write_text(cp.stdout+cp.stderr)
    invalid_rejected=cp.returncode!=0 and 'DNBX_DIAGNOSTIC_COUNTERS_must_be_zero_or_one' in cp.stderr
    (OUT/'COMMANDS.json').write_text(json.dumps(commands,indent=2)+'\n')
    return libs,invalid_rejected

def boundaries(libs):
    apis={k:Api(v) for k,v in libs.items()};records=[]
    for index,(name,lo,hi) in enumerate(FIELDS):
        blo,bhi=bits(lo),bits(hi);flo,fhi=value(blo),value(bhi)
        samples={
            'lo':blo,'lo_inside':nextbits(lo,True),'lo_outside':nextbits(lo,False),
            'hi':bhi,'hi_inside':nextbits(hi,False),'hi_outside':nextbits(hi,True),
            'mid':bits((flo+fhi)/2),'plus_zero':0,'minus_zero':0x80000000,
            'plus_inf':0x7f800000,'minus_inf':0xff800000,
            'plus_qnan':0x7fc00001,'minus_qnan':0xffc00001,
            'plus_snan':0x7f800001,'minus_snan':0xff800001,
            'plus_max':0x7f7fffff,'minus_max':0xff7fffff,
            'plus_subnormal':1,'minus_subnormal':0x80000001}
        for label,b in samples.items():
            x=value(b);expected=0 if flo<=x<=fhi else -2;returns=[];state_equal=True;unchanged=True;states=[]
            for mode,api in apis.items():
                s=api.part();p=api.params();putword(p,index,b);before=snapshot(s)
                rc=api.d.dnbx_set_layer(s,1,C.byref(p));returns.append(rc);states.append(snapshot(s))
                if expected!=0:unchanged &= before==snapshot(s)
            state_equal=len(set(states))==1
            records.append({'field':name,'input':label,'binary32_hex':f'{b:08x}','expected_return':expected,'returns':dict(zip(apis,returns)),'state_equal':state_equal,'rejection_unchanged':unchanged,'pass':all(x==expected for x in returns) and state_equal and unchanged})
    for mode_value in (0,1,2,0xffffffff):
        returns=[];states=[];unchanged=True;expected=0 if mode_value<2 else -2
        for api in apis.values():
            s=api.part();p=api.params();p.oscillator_mode=mode_value;before=snapshot(s)
            returns.append(api.d.dnbx_set_layer(s,1,C.byref(p)));states.append(snapshot(s))
            if expected:unchanged &= before==snapshot(s)
        records.append({'field':'oscillator_mode','value':mode_value,'expected_return':expected,'returns':dict(zip(apis,returns)),'state_equal':len(set(states))==1,'rejection_unchanged':unchanged,'pass':returns==[expected]*3 and len(set(states))==1 and unchanged})
    return {'cases':records,'case_count':len(records),'all_pass':all(x['pass'] for x in records)}

def capability_checks(libs):
    off=libs['compact'];on=libs['diagnostic'];checks=[]
    checks.append({'name':'profile_query','diagnostic':on.dnbx_diagnostic_counters_enabled(),'compact':off.dnbx_diagnostic_counters_enabled(),'pass':on.dnbx_diagnostic_counters_enabled()==1 and off.dnbx_diagnostic_counters_enabled()==0})
    for n in (0,1,256,1024,1025):
        api=Api(off);s=api.part();off.dnbx_note_on(s,60,127)
        w=Work();C.memset(C.addressof(w),0x5a,C.sizeof(w));l=(C.c_float*1025)(*[123.25]*1025);r=(C.c_float*1025)(*[124.25]*1025)
        before=(raw(s),raw(w),raw(l),raw(r));rc=off.dnbx_render(s,l,r,n,C.byref(w));after=(raw(s),raw(w),raw(l),raw(r))
        checks.append({'name':'nonnull_work_'+str(n),'return_code':rc,'all_unchanged':before==after,'pass':rc==-5 and before==after})
    # Nonnull work with uninitialized state retains the documented first validity check.
    s=C.create_string_buffer(off.dnbx_part_size());w=Work();rc=off.dnbx_render(s,None,None,0,C.byref(w))
    checks.append({'name':'uninitialized_precedes_profile_check','return_code':rc,'pass':rc==-3})
    s=Api(off).part();before=raw(s);rc=off.dnbx_render(s,None,None,0,None)
    checks.append({'name':'zero_frames_null_work','return_code':rc,'pass':rc==0 and raw(s)==before})
    return {'cases':checks,'all_pass':all(x['pass'] for x in checks)}

def compare_audio_state(libs):
    rng=random.Random(0xD0B0C0DE);apis={k:Api(d) for k,d in libs.items()};records=[];total_frames=0
    for sr in (8000,44100,48000,192000):
        parts={mode:[api.part(sr),api.part(sr)] for mode,api in apis.items()}
        params=[[apis['accepted'].params([.01,0,.05,.7,0,.1]) for _ in range(2)] for _ in range(2)]
        for part in range(2):
            for layer in range(2):
                p=params[part][layer];p.filter=Env(.02,.01,.1,.3,.02,.2);p.waveform=(part*2+layer)/3;p.pan=(-.6 if layer==0 else .6);p.lfo_hz=7;p.lfo_pwm_depth=.4
                for mode,api in apis.items():api.put(parts[mode][part],p,layer)
            for note in range(60,66):
                for mode,api in apis.items():api.d.dnbx_note_on(parts[mode][part],note,90+note%38)
        pos=0;blocks=0;events=0;hashes={k:hashlib.sha256() for k in libs};all_equal=True;states_equal=True;counter_equal=True
        works={k:Work() for k in ('accepted','diagnostic')}
        while pos<32768:
            part=rng.randrange(2);kind=rng.randrange(8);events+=1
            if kind==0:
                args=(rng.randrange(48,80),rng.choice([0,1,64,100,127]));op='dnbx_note_on'
            elif kind==1:args=(rng.randrange(48,80),);op='dnbx_note_off'
            elif kind==2:args=(rng.randrange(2),);op='dnbx_set_sustain'
            elif kind==3:args=(rng.randrange(2),);op='dnbx_set_velocity_mode'
            elif kind==4:
                layer=rng.randrange(2);idx=rng.randrange(23);name,lo,hi=FIELDS[idx];p=params[part][layer]
                chosen=rng.choice([lo,hi,(lo+hi)/2]);putword(p,idx,bits(chosen));args=(layer,C.byref(p));op='dnbx_set_layer'
            elif kind==5:args=(rng.randrange(218),rng.choice([-0.0,0.0,1.32,70.0,-.008403362]));op='dnbx_store_raw'
            elif kind==6:args=();op='dnbx_all_off'
            else:args=(rng.randrange(60,66),127);op='dnbx_note_on'
            returns=[]
            for mode,api in apis.items():returns.append(getattr(api.d,op)(parts[mode][part],*args))
            all_equal &= len(set(returns))==1
            if kind==6:
                for note in range(60,66):
                    for mode,api in apis.items():api.d.dnbx_note_on(parts[mode][part],note,127)
                events+=6
            for part_index in range(2):states_equal &= len({raw(parts[k][part_index]) for k in libs})==1
            n=min(rng.choice([1,3,7,16,31,64,127,256,511,1024]),32768-pos)
            for part_index in range(2):
                outputs=[]
                for mode,api in apis.items():
                    l=(C.c_float*n)();r=(C.c_float*n)();work=works.get(mode)
                    rc=api.d.dnbx_render(parts[mode][part_index],l,r,n,C.byref(work) if work is not None else None)
                    assert rc==0
                    blob=raw(l)+raw(r);outputs.append(blob);hashes[mode].update(blob)
                all_equal &= len(set(outputs))==1
                states_equal &= len({raw(parts[k][part_index]) for k in libs})==1
            counter_equal &= raw(works['accepted'])==raw(works['diagnostic'])
            total_frames+=2*n;pos+=n;blocks+=1
        records.append({'sample_rate':sr,'frames_per_part':pos,'parts':2,'blocks_per_part':blocks,'event_or_control_calls':events,'stereo_audio_sha256':{k:h.hexdigest() for k,h in hashes.items()},'bitwise_audio_equal':all_equal,'bitwise_state_equal_after_every_event_and_block':states_equal,'render_counter_equal_when_enabled':counter_equal,'pass':all_equal and states_equal and counter_equal})
    return {'seed_hex':'D0B0C0DE','total_frames_over_parts':total_frames,'cases':records,'all_pass':all(x['pass'] for x in records)}

def main():
    declaration={'created_at_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'scope':'Compare compact typed-bound validation and explicit counters-off profile against accepted9cdcea source, with no relaxed numeric tolerance. No native timing or Sound completion claim.','gates':['All named float-field bounds admit exactly endpoints/interior, reject next representable exterior and NaN/Inf, matching baseline return/state bits.','Both profiles return identical output bytes and part-state bytes to baseline after every event/block on same inputs. Diagnostic counters match when enabled.','Counters-off query is0 and nonnullwork rejects with E_UNSUPPORTED before state/output/work mutation, including zero frames.','Invalid profile macro2 fails compilation.'],'profiles':{'diagnostic':'default1','compact':'explicit-D DNBX_DIAGNOSTIC_COUNTERS=0'},'compile_contract':'C11 -O2 -fno-fast-math -ffp-contract=off -DX0X_HOST'}
    # Keep the predeclaration outside the not-yet-created frozen result directory.
    (HERE/'COMPACT-ACCEPTANCE-DECLARATION.json').write_text(json.dumps(declaration,indent=2)+'\n')
    libs,invalid=compile_all()
    result={'created_at_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'candidate_sha256':{p.name:sha(p) for p in (OUT/'snapshot').iterdir()},'baseline_sha256':{p.name:sha(p) for p in (HERE/'frozen_03/snapshot').iterdir()},'harness_sha256':sha(pathlib.Path(__file__)),'declaration_sha256':sha(HERE/'COMPACT-ACCEPTANCE-DECLARATION.json'),'boundary_equivalence':boundaries(libs),'capability_contract':capability_checks(libs),'audio_state_equivalence':compare_audio_state(libs),'invalid_profile_compile_rejected':invalid}
    result['scoped_equivalence_pass']=result['boundary_equivalence']['all_pass'] and result['capability_contract']['all_pass'] and result['audio_state_equivalence']['all_pass'] and invalid
    (OUT/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:result[k] for k in ('candidate_sha256','boundary_equivalence','capability_contract','audio_state_equivalence','invalid_profile_compile_rejected','scoped_equivalence_pass') if k!='boundary_equivalence'}|{'boundary_cases':result['boundary_equivalence']['case_count'],'boundaries_pass':result['boundary_equivalence']['all_pass']},indent=2))

if __name__=='__main__':main()
