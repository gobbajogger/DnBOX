#!/usr/bin/env python3
"""Independent, public-API checks; originals are read only as numeric fixtures.

Creates its own frozen review snapshot. Never modifies implementation input.
"""
import argparse, collections, ctypes as C, csv, datetime, hashlib, json, math
import pathlib, shutil, subprocess, sys

HERE = pathlib.Path(__file__).resolve().parent
BASE = HERE.parent
class Env(C.Structure):
    _fields_ = [(n, C.c_float) for n in ('attack_s','hold_s','decay_s','sustain','release_hold_s','release_s')]
class Params(C.Structure):
    _fields_ = [('amp',Env),('filter',Env)] + [(n,C.c_float) for n in ('waveform','pulse_width','gain','sub_gain','cutoff_hz','resonance','filter_env_hz','lfo_hz','lfo_pwm_depth','tune_semitones','pan')] + [('oscillator_mode',C.c_uint32)]
class Snapshot(C.Structure):
    _fields_ = [(n,C.c_float*2) for n in ('amp','filter','phase')] + [(n,C.c_uint8*2) for n in ('amp_stage','filter_stage')] + [(n,C.c_uint8) for n in ('note','gate','age_rank','active')]
WORK_NAMES = ('frames','voice_slots','active_voice_samples','layer_samples','envelope_samples','envelope_ticks','stage_transitions','exp2_calls','log2_calls','small_exp_calls','lfo_samples','blep_calls','sine_calls')
class Work(C.Structure):
    _fields_ = [(n,C.c_uint64) for n in WORK_NAMES] + [('maximum_transition_chain',C.c_uint32),('reserved',C.c_uint32)]

def sha(p): return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
def raw(x): return C.string_at(C.addressof(x),C.sizeof(x))
def finite(xs): return all(math.isfinite(x) for x in xs)
def bind(path):
    d=C.CDLL(str(path))
    def f(n,rest,args):
        fn=getattr(d,n); fn.restype=rest; fn.argtypes=args
    P=C.c_void_p; U=C.c_uint; F=C.c_float; I=C.c_int; FP=C.POINTER(F)
    for n in ('dnbx_part_size','dnbx_layer_params_size','dnbx_work_size'): f(n,C.c_size_t,[])
    f('dnbx_default_layer_params',None,[C.POINTER(Params)])
    f('dnbx_init',I,[P,F]); f('dnbx_set_layer',I,[P,U,C.POINTER(Params)]); f('dnbx_get_layer',I,[P,U,C.POINTER(Params)])
    for n in ('dnbx_note_on',): f(n,I,[P,U,U])
    for n in ('dnbx_note_off','dnbx_set_sustain','dnbx_set_velocity_mode'): f(n,I,[P,U])
    for n in ('dnbx_all_off','dnbx_active_voices'): f(n,I,[P])
    f('dnbx_read_voice',I,[P,U,C.POINTER(Snapshot)])
    f('dnbx_render',I,[P,FP,FP,U,C.POINTER(Work)])
    f('dnbx_store_raw',I,[P,U,F]); f('dnbx_read_raw',I,[P,U,FP])
    for n in ('dnbx_attack_from_unit','dnbx_lfo_from_unit','dnbx_pwm_from_unit','dnbx_range_from_stop'): f(n,I,[F,FP])
    f('dnbx_wave_probe_mode',I,[F,F,F,F,U,FP])
    assert d.dnbx_layer_params_size()==C.sizeof(Params)
    assert d.dnbx_work_size()==C.sizeof(Work)
    return d

class Api:
    def __init__(self,lib): self.d=lib
    def part(self,sr=48000):
        p=C.create_string_buffer(self.d.dnbx_part_size()); assert self.d.dnbx_init(p,sr)==0; return p
    def params(self,amp=None):
        p=Params(); self.d.dnbx_default_layer_params(C.byref(p))
        if amp is not None: p.amp=Env(*amp)
        return p
    def put(self,s,p,layer=0): assert self.d.dnbx_set_layer(s,layer,C.byref(p))==0
    def snap(self,s,i=0):
        x=Snapshot(); assert self.d.dnbx_read_voice(s,i,C.byref(x))==0; return x
    def render(self,s,n,w=None):
        l=(C.c_float*n)();r=(C.c_float*n)()
        assert self.d.dnbx_render(s,l,r,n,C.byref(w) if w is not None else None)==0
        return list(l),list(r)
    def advance(self,s,n):
        while n: q=min(n,1024); self.render(s,q); n-=q
    def stereo(self,s,n,blocks=(1024,),events=()):
        events=sorted(events); pos=0; b=0; e=0; left=[];right=[];w=Work()
        while pos<n:
            while e<len(events) and events[e][0]==pos:
                _,op,args=events[e]; getattr(self.d,op)(s,*args);e+=1
            end=min(n,pos+blocks[b%len(blocks)])
            if e<len(events): end=min(end,events[e][0])
            l,r=self.render(s,end-pos,w);left+=l;right+=r;pos=end;b+=1
        return left,right,w

def error_stats(actual,expected,positions):
    es=[abs(actual[n]-x) for n,x in zip(positions,expected)]
    k=max(range(len(es)),key=es.__getitem__)
    return {'max_abs':max(es),'rms':math.sqrt(sum(e*e for e in es)/len(es)),'comparisons':len(es),'worst_frame':positions[k],'actual_at_worst':actual[positions[k]],'original_at_worst':expected[k],'pass':finite(es) and max(es)<=1e-3}

def linear_contour(cfg,sr):
    a,h,d,s,rh,r=cfg['amp']; a=round(a*sr);h=round(h*sr);d=round(d*sr);rh=round(rh*sr);r=round(r*sr)
    off=cfg['events'][1]['frame']; out=[]
    for n in range(cfg['frames']):
        t=n+1
        if n>=off:
            t=n-off+1
            x=s if t<=rh else max(0,s*(1-(t-rh)/r)) if r else 0
        elif t<=a: x=t/a
        elif t<=a+h: x=1
        elif t<=a+h+d: x=1-(1-s)*(t-a-h)/d
        else: x=s
        out.append(x)
    return out

def envelope_tests(api,outdir):
    d=api.d; fixture=json.loads((BASE/'mapping/results/implementer_brief/recorded_envelope_behavior.json').read_text())
    rows=list(csv.DictReader((BASE/'mapping/results/implementer_brief/amp_observed_levels.csv').open()))
    results=[];traces={}
    for idx in (1,2):
        cfg=next(c['configuration'] for c in fixture['cases'] if c['case_index']==idx)
        s=api.part(); p=api.params(cfg['amp']);p.filter=Env(0,0,0,1,0,16);p.filter_env_hz=0;p.resonance=0
        api.put(s,p); p.gain=0;api.put(s,p,1)
        v=cfg['events'][0]['velocity'];d.dnbx_note_on(s,60,v)
        levels=[];audio=[];off=cfg['events'][1]['frame']
        for n in range(cfg['frames']):
            if n==off: d.dnbx_note_off(s,60)
            l,_=api.render(s,1);audio.append(l[0]);levels.append(float(api.snap(s).amp[0]))
        ref=api.part();p.amp=Env(0,0,0,1,0,16);p.gain=1;api.put(ref,p);p.gain=0;api.put(ref,p,1);d.dnbx_note_on(ref,60,v)
        ref_audio,_,_=api.stereo(ref,cfg['frames'])
        ratio=[audio[n]/ref_audio[n] if abs(ref_audio[n])>1e-6 else None for n in range(cfg['frames'])]
        diff=[abs(ratio[n]-levels[n]) for n in range(cfg['frames']) if ratio[n] is not None]
        chosen=[r for r in rows if int(r['case_index'])==idx]; positions=[];expected=[]
        for row in chosen:
            for endpoint in ('first','last'):
                positions.append(int(row[endpoint+'_observed_frame']));expected.append(float(row[endpoint+'_observed_level']))
        stat=error_stats(levels,expected,positions)
        stat.update(case_index=idx,case_name=cfg['name'],ratio_crosscheck_max_abs=max(diff),ratio_crosscheck_samples=len(diff),ratio_crosscheck_pass=max(diff)<=2e-6)
        shifted=levels[16:]+[levels[-1]]*16
        stat['negative_wrong_time_grid']=error_stats(shifted,expected,positions)
        stat['negative_linear_envelope']=error_stats(linear_contour(cfg,48000),expected,positions)
        results.append(stat);traces[str(idx)]=levels
        with (outdir/f'envelope_case{idx}.csv').open('w',newline='') as f:
            w=csv.writer(f);w.writerow(['frame','candidate_amp_snapshot','candidate_audio','constant_amp_audio','candidate_inferred_amp'])
            w.writerows((n,levels[n],audio[n],ref_audio[n],ratio[n]) for n in range(len(levels)))
    return results

def velocity_tests(api):
    def audio(v,mode):
        s=api.part();p=api.params([0,0,0,.7,0,.1]);p.filter_env_hz=0;api.put(s,p);p.gain=0;api.put(s,p,1)
        api.d.dnbx_set_velocity_mode(s,mode);api.d.dnbx_note_on(s,60,v)
        return api.stereo(s,4096)[0]
    hi=audio(127,0);lo=audio(1,0);wrong=audio(1,1)
    ok=[abs(lo[n]*127/hi[n]-1) for n in range(len(hi)) if abs(hi[n])>1e-6]
    bad=[abs(wrong[n]*127/hi[n]-1) for n in range(len(hi)) if abs(hi[n])>1e-6]
    return {'normalized_linear_ratio_error':max(ok),'linear_pass':max(ok)<=1e-4,'samples':len(ok),'negative_squared_ratio_error':max(bad),'negative_squared_rejected':max(bad)>1e-4}

def lifecycle_tests(api):
    d=api.d;res={}
    # Real irregular events, including simultaneous notes, retrigger, sustain, velocity0.
    events=[(0,'dnbx_note_on',(60,100)),(31,'dnbx_note_on',(64,90)),(1001,'dnbx_note_off',(60,)),(1177,'dnbx_set_sustain',(1,)),(1201,'dnbx_note_off',(64,)),(2123,'dnbx_note_on',(60,64)),(2347,'dnbx_note_on',(67,127)),(3451,'dnbx_set_sustain',(0,)),(5003,'dnbx_note_off',(60,)),(5131,'dnbx_note_on',(67,0))]
    renders=[]
    for blocks in ((1,),(16,),(64,),(1,3,5,7,13,16),(1024,)):
        s=api.part();l,r,w=api.stereo(s,16384,blocks,events);renders.append((blocks,l,r,raw(s),raw(w)))
    res['block_partition']={'schedules':[list(x[0]) for x in renders],'frames':16384,'bit_identical_output_state_counters':all(x[1:]==renders[0][1:] for x in renders[1:])}
    # Separate instances: adversarial interleaving does not alter a baseline instance.
    base=api.part();other=api.part();d.dnbx_note_on(base,55,117); baseline=api.stereo(base,4096)[0]
    s=api.part();d.dnbx_note_on(s,55,117);got=[]
    for i in range(64):
        d.dnbx_note_on(other,35+i%80,1+i%127);d.dnbx_set_sustain(other,i%2);d.dnbx_store_raw(other,i%218,i*1.32)
        api.render(other,31+i%7);l,_=api.render(s,64);got+=l
    res['part_isolation']={'pass':baseline==got,'tested_interleaves':64}
    # Independent layer/amp/filter trajectories; setting unrelated layer must not alter layer0 state.
    s=api.part();p=api.params([.01,0,.02,.8,0,.1]);p.filter=Env(.03,0,.1,.2,0,.2);api.put(s,p)
    p=api.params([.1,0,.3,.4,0,.4]);p.filter=Env(.2,0,.4,.6,0,.6);api.put(s,p,1)
    d.dnbx_note_on(s,60,100);api.advance(s,240);x=api.snap(s)
    res['layer_envelope_independence']={'amp':list(x.amp),'filter':list(x.filter),'pass':len(set(list(x.amp)+list(x.filter)))==4}
    # Repeated pedal events must not reset a release already running.
    a=api.part();b=api.part();
    for s in (a,b): d.dnbx_note_on(s,60,100);api.advance(s,4096);d.dnbx_note_off(s,60);api.advance(s,800)
    d.dnbx_set_sustain(b,1);d.dnbx_set_sustain(b,0)
    aa=api.stereo(a,8192);bb=api.stereo(b,8192)
    res['pedal_up_no_restart']={'pass':aa[:2]==bb[:2] and raw(a)==raw(b)}
    # Pedal holds a released gate, then eventually frees it.
    s=api.part();d.dnbx_note_on(s,60,100);api.advance(s,4096);d.dnbx_set_sustain(s,1);d.dnbx_note_off(s,60);api.advance(s,20000)
    held=d.dnbx_active_voices(s);d.dnbx_set_sustain(s,0);api.advance(s,20000)
    res['pedal_lifecycle']={'held_active':held,'after_release_active':d.dnbx_active_voices(s),'pass':held==1 and d.dnbx_active_voices(s)==0}
    # Recycle low-numbered voices until rank saturation would hide an older high-numbered voice.
    s=api.part();p=api.params([0,0,0,1,0,0]);p.filter=Env(0,0,0,1,0,0);api.put(s,p);api.put(s,p,1)
    for note in range(60,66): d.dnbx_note_on(s,note,100)
    log=[]
    for note in range(60,65):
        d.dnbx_note_off(s,note);api.render(s,1);new=note+10;slot=d.dnbx_note_on(s,new,100);log.append({'off':note,'on':new,'slot':slot})
    current=70
    for note in range(80,90):
        d.dnbx_note_off(s,current);api.render(s,1);slot=d.dnbx_note_on(s,note,100);log.append({'off':current,'on':note,'slot':slot});current=note
    before=[{'slot':i,'note':api.snap(s,i).note,'rank':api.snap(s,i).age_rank} for i in range(6)]
    slot=d.dnbx_note_on(s,100,100)
    res['oldest_held_after_retirement']={'events':log,'before':before,'oldest_note_expected':65,'expected_slot':5,'chosen_slot':slot,'pass':slot==5}
    # Held same-note must use its existing slot rather than consume an extra one.
    s=api.part();slot=d.dnbx_note_on(s,60,100);api.advance(s,120);slot2=d.dnbx_note_on(s,60,90)
    res['same_held_note']={'slot1':slot,'slot2':slot2,'active':d.dnbx_active_voices(s),'pass':slot==slot2 and d.dnbx_active_voices(s)==1}
    # all_off is repeatable and returns silence without dropping raw controls/params.
    d.dnbx_store_raw(s,0,1.32);d.dnbx_all_off(s);before=raw(s);d.dnbx_all_off(s);l,r=api.render(s,64)
    value=C.c_float();d.dnbx_read_raw(s,0,C.byref(value))
    res['all_off']={'pass':before==raw(s) and not any(l+r) and value.value==C.c_float(1.32).value}
    return res

def error_tests(api):
    d=api.d;s=api.part();d.dnbx_note_on(s,60,100);api.advance(s,17);results=[]
    def check(name,fn,expected,out=None):
        before=raw(s);out_before=raw(out) if out is not None else None;rc=fn()
        results.append({'name':name,'return_code':rc,'expected':expected,'state_unchanged':raw(s)==before,'output_unchanged':out is None or raw(out)==out_before,'pass':rc==expected and raw(s)==before and (out is None or raw(out)==out_before)})
    p=api.params();
    for attr in ('attack_s','hold_s','decay_s','sustain','release_hold_s','release_s'):
        for bad in (float('nan'),float('inf'),float('-inf'),-1,17):
            q=Params.from_buffer_copy(raw(p));setattr(q.amp,attr,bad)
            check(f'amp_{attr}_{bad}',lambda q=q:d.dnbx_set_layer(s,0,C.byref(q)),-2)
    for attr in ('waveform','pulse_width','gain','sub_gain','cutoff_hz','resonance','filter_env_hz','lfo_hz','lfo_pwm_depth','tune_semitones','pan'):
        for bad in (float('nan'),float('inf'),float('-inf')):
            q=Params.from_buffer_copy(raw(p));setattr(q,attr,bad)
            check(f'layer_{attr}_{bad}',lambda q=q:d.dnbx_set_layer(s,0,C.byref(q)),-2)
    check('bad_note_on',lambda:d.dnbx_note_on(s,128,1),-2)
    check('bad_velocity',lambda:d.dnbx_note_on(s,60,128),-2)
    check('bad_note_off',lambda:d.dnbx_note_off(s,128),-2)
    check('bad_sustain',lambda:d.dnbx_set_sustain(s,2),-2)
    check('bad_velocity_mode',lambda:d.dnbx_set_velocity_mode(s,2),-2)
    check('bad_layer',lambda:d.dnbx_set_layer(s,2,C.byref(p)),-1)
    for v in (float('nan'),float('inf'),float('-inf')): check('bad_raw_'+str(v),lambda v=v:d.dnbx_store_raw(s,1,v),-2)
    check('bad_raw_id',lambda:d.dnbx_store_raw(s,218,0),-2)
    buf=(C.c_float*2048)(*[123.25]*2048); ptr=C.cast(C.byref(buf,1024*4),C.POINTER(C.c_float))
    check('oversize_render',lambda:d.dnbx_render(s,buf,ptr,1025,None),-2,buf)
    check('null_render',lambda:d.dnbx_render(s,None,ptr,64,None),-1,buf)
    check('equal_output_buffers',lambda:d.dnbx_render(s,buf,buf,64,None),-1,buf)
    check('zero_render_null',lambda:d.dnbx_render(s,None,None,0,None),0,buf)
    for sr in (0,7999,192001,float('nan'),float('inf')):check('bad_init_'+str(sr),lambda sr=sr:d.dnbx_init(s,sr),-2)
    out=C.c_float(123.25)
    check('raw_absent',lambda:d.dnbx_read_raw(s,217,C.byref(out)),-4,out)
    # Exact binary32 admission, preserving present zero and values outside0..1.
    values=[(0,0.),(1,-0.),(2,1.32),(3,1.001),(4,70.),(5,9.),(6,-.008403362),(207,4.),(212,0.),(214,1.),(217,3.4028234663852886e38)]
    admissions=[]
    for idx,v in values:
        x=C.c_float(v);rc=d.dnbx_store_raw(s,idx,x);y=C.c_float();got=d.dnbx_read_raw(s,idx,C.byref(y));exp=2 if idx==207 else 3 if 212<=idx<=214 else 1
        admissions.append({'id':idx,'status':rc,'bits_preserved':raw(x)==raw(y),'pass':rc==exp and got==0 and raw(x)==raw(y)})
    return {'validated_error_paths':results,'raw_admission':admissions,'all_pass':all(x['pass'] for x in results+admissions)}

def finite_tests(api):
    d=api.d;cases=[]
    for sr in (8000,44100,48000,192000):
      for mode in (0,1):
       for wave in (0,.16666667,1/3,.5,2/3,.83333333,1):
        s=api.part(sr);p=api.params([0,0,0,1,0,16]);p.filter=Env(0,0,0,1,0,16);p.waveform=wave;p.pulse_width=.03;p.gain=4;p.sub_gain=.5;p.cutoff_hz=20000;p.resonance=1;p.filter_env_hz=20000;p.lfo_hz=16;p.lfo_pwm_depth=.47;p.tune_semitones=48;p.pan=-1;p.oscillator_mode=mode
        api.put(s,p);api.put(s,p,1)
        for n in (0,48,60,90,120,127): d.dnbx_note_on(s,n,127)
        l,r,w=api.stereo(s,4096);peak=max(map(abs,l+r));env=[v for i in range(6) for v in list(api.snap(s,i).amp)+list(api.snap(s,i).filter)]
        cases.append({'sr':sr,'mode':mode,'wave':wave,'frames':4096,'peak':peak,'finite':finite(l+r+env),'bounded':peak<=32+1e-5,'max_transition_chain':w.maximum_transition_chain})
    # Negative cutoff excursion, one-sample stages, zero stages, minimum note.
    for sr in (8000,44100,48000,192000):
      s=api.part(sr);p=api.params([1/sr,1/sr,1/sr,0,1/sr,1/sr]);p.filter=Env(0,0,0,0,0,0);p.filter_env_hz=-20000;p.cutoff_hz=20;p.resonance=1;p.tune_semitones=-48;api.put(s,p);api.put(s,p,1);d.dnbx_note_on(s,0,1)
      l,r,w=api.stereo(s,64);d.dnbx_note_off(s,0);ll,rr,ww=api.stereo(s,64)
      cases.append({'sr':sr,'variant':'one_sample_and_zero','frames':128,'peak':max(map(abs,l+r+ll+rr)),'finite':finite(l+r+ll+rr),'bounded':max(map(abs,l+r+ll+rr))<=32+1e-5,'retired':d.dnbx_active_voices(s)==0,'max_transition_chain':max(w.maximum_transition_chain,ww.maximum_transition_chain)})
    return {'cases':cases,'total_frames':sum(x['frames'] for x in cases),'all_pass':all(x['finite'] and x['bounded'] and x.get('retired',True) for x in cases),'peak':max(x['peak'] for x in cases)}

def long_duration_tests(api):
    # Full maximum duration at the largest supported sample rate; exact endpoint checks.
    d=api.d;s=api.part(192000);p=api.params([16,0,0,1,0,16]);p.filter=Env(0,0,0,1,0,16);api.put(s,p);p.gain=0;api.put(s,p,1)
    d.dnbx_note_on(s,60,127);n=16*192000;previous=0.;monotonic=True;peak=0.;remain=n-1
    while remain:
        take=min(remain,1024);l,r=api.render(s,take);x=api.snap(s).amp[0];monotonic &= x>=previous;previous=x;peak=max(peak,max(map(abs,l+r)));remain-=take
    before=api.snap(s).amp[0];api.render(s,1);end=api.snap(s).amp[0]
    api.advance(s,32);d.dnbx_note_off(s,60);remaining=n-1;previous=1.;release_monotonic=True
    while remaining:
        take=min(remaining,1024);l,r=api.render(s,take);x=api.snap(s).amp[0];release_monotonic &= x<=previous;previous=x;peak=max(peak,max(map(abs,l+r)));remaining-=take
    release_before=api.snap(s).amp[0];api.render(s,1);release_end=api.snap(s).amp[0];api.render(s,1)
    return {'sample_rate':192000,'duration_frames_each':n,'total_rendered_frames':2*n+33,'attack_penultimate':before,'attack_endpoint':end,'release_penultimate':release_before,'release_endpoint':release_end,'attack_monotonic_at_block_endpoints':monotonic,'release_monotonic_at_block_endpoints':release_monotonic,'peak':peak,'retired':d.dnbx_active_voices(s)==0,'pass':0<=before<1 and end==1 and release_before>0 and release_end==0 and monotonic and release_monotonic and d.dnbx_active_voices(s)==0 and peak<=32+1e-5}

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--label',required=True);parser.add_argument('--source',type=pathlib.Path,default=BASE/'hardware_candidate/src');args=parser.parse_args()
    out=HERE/args.label;out.mkdir();snap=out/'snapshot';snap.mkdir()
    for name in ('dnbx_core.c','dnbx_core.h','fastmath.h'):shutil.copy2(args.source/name,snap/name)
    command=['cc','-std=c11','-Wall','-Wextra','-Werror','-O2','-ffp-contract=off','-DX0X_HOST','-fPIC','-shared',str(snap/'dnbx_core.c'),'-o',str(out/'libcandidate.so')]
    cp=subprocess.run(command,text=True,capture_output=True);(out/'build.log').write_text(cp.stdout+cp.stderr)
    (out/'COMMANDS.json').write_text(json.dumps({'build':command,'invocation':sys.argv},indent=2)+'\n');cp.check_returncode()
    d=bind(out/'libcandidate.so');api=Api(d)
    result={'created_at_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'snapshot_sha256':{p.name:sha(p) for p in snap.iterdir()},'harness_sha256':sha(__file__),'declaration_sha256':sha(HERE/'ACCEPTANCE-DECLARATION.json'),'compiler':subprocess.check_output(['cc','--version'],text=True).splitlines()[0],'floating_contract':'off; host X0X_HOST','envelope':envelope_tests(api,out),'velocity':velocity_tests(api),'lifecycle':lifecycle_tests(api),'errors':error_tests(api),'finite':finite_tests(api),'long_durations':long_duration_tests(api)}
    result['scoped_acceptance_pass']=all(x['pass'] and x['ratio_crosscheck_pass'] and not x['negative_wrong_time_grid']['pass'] and not x['negative_linear_envelope']['pass'] for x in result['envelope']) and result['velocity']['linear_pass'] and result['velocity']['negative_squared_rejected'] and all(x.get('pass',x.get('bit_identical_output_state_counters',False)) for x in result['lifecycle'].values()) and result['errors']['all_pass'] and result['finite']['all_pass'] and result['long_durations']['pass']
    (out/'RESULT.json').write_text(json.dumps(result,indent=2,allow_nan=False)+'\n')
    print(json.dumps({k:result[k] for k in ('snapshot_sha256','envelope','velocity','lifecycle','scoped_acceptance_pass')},indent=2))

if __name__=='__main__': main()
