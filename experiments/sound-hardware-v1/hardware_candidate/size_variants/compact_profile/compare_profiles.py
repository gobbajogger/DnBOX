#!/usr/bin/env python3
"""Exact ABI, boundary admission, audio and state comparison of local variants."""
import ctypes as c
import hashlib
import json
import math
import struct
from pathlib import Path

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]

class Envelope(c.Structure):
    _fields_=[(n,c.c_float) for n in ("attack_s","hold_s","decay_s","sustain","release_hold_s","release_s")]
class Parameters(c.Structure):
    _fields_=[("amp",Envelope),("filter",Envelope)]+[(n,c.c_float) for n in
              ("waveform","pulse_width","gain","sub_gain","cutoff_hz","resonance","filter_env_hz","lfo_hz","lfo_pwm_depth","tune_semitones","pan")]+[("oscillator_mode",c.c_uint32)]

def load(path):
    lib=c.CDLL(str(path.resolve()))
    lib.dnbx_part_size.restype=c.c_size_t
    lib.dnbx_layer_params_size.restype=c.c_size_t
    lib.dnbx_work_size.restype=c.c_size_t
    for name,args in {
        "dnbx_init":[c.c_void_p,c.c_float],"dnbx_set_layer":[c.c_void_p,c.c_uint,c.c_void_p],
        "dnbx_note_on":[c.c_void_p,c.c_uint,c.c_uint],"dnbx_note_off":[c.c_void_p,c.c_uint],
        "dnbx_set_sustain":[c.c_void_p,c.c_uint],"dnbx_set_velocity_mode":[c.c_void_p,c.c_uint],
        "dnbx_store_raw":[c.c_void_p,c.c_uint,c.c_float],"dnbx_all_off":[c.c_void_p],
        "dnbx_render":[c.c_void_p,c.c_void_p,c.c_void_p,c.c_uint,c.c_void_p],
        "dnbx_default_layer_params":[c.c_void_p]}.items():
        getattr(lib,name).argtypes=args
        getattr(lib,name).restype=c.c_int
    return lib

def state(lib):
    s=(c.c_uint32*(lib.dnbx_part_size()//4))()
    assert lib.dnbx_init(s,44100)==0
    return s

def near_float(value,up):
    value=c.c_float(value).value
    if value==0:
        bits=1 if up else 0x80000001
    else:
        bits=struct.unpack("I",struct.pack("f",value))[0]
        bits+=1 if (value>0)==up else -1
    return struct.unpack("f",struct.pack("I",bits))[0]

def main():
    libs=[load(ROOT/"build/libdnbx.so"),load(HERE/"libdiagnostic.so"),load(HERE/"libcompact.so")]
    assert all(x.dnbx_part_size()==2264 and x.dnbx_layer_params_size()==96 for x in libs)
    assert libs[1].dnbx_diagnostic_counters_enabled()==1
    assert libs[2].dnbx_diagnostic_counters_enabled()==0
    base=Parameters();libs[0].dnbx_default_layer_params(c.byref(base))
    bounds=[(0,16),(0,16),(0,16),(0,1),(0,16),(0,16)]*2+[(0,1),(.03,.5),(0,4),(0,.5),(20,20000),(0,1),(-20000,20000),(0,16),(0,.47),(-48,48),(-1,1)]
    count=0
    for field,(lo,hi) in enumerate(bounds):
        for value in (lo,hi,near_float(lo,False),near_float(hi,True),(lo+hi)*.5,0,-0.0,math.nan,math.inf,-math.inf):
            p=Parameters.from_buffer_copy(base)
            c.c_float.from_address(c.addressof(p)+field*4).value=value
            ss=[state(lib) for lib in libs]
            results=[lib.dnbx_set_layer(s,0,c.byref(p)) for lib,s in zip(libs,ss)]
            assert results[0]==results[1]==results[2],(field,value,results)
            assert bytes(ss[0])==bytes(ss[1])==bytes(ss[2]),(field,value,"state")
            count+=1
    for mode in (0,1,2,0xffffffff):
        p=Parameters.from_buffer_copy(base);p.oscillator_mode=mode
        ss=[state(lib) for lib in libs]
        results=[lib.dnbx_set_layer(s,0,c.byref(p)) for lib,s in zip(libs,ss)]
        assert results[0]==results[1]==results[2]
        assert bytes(ss[0])==bytes(ss[1])==bytes(ss[2]);count+=1

    # A nonnull work argument must fail atomically in compact profile.
    s=state(libs[2]);before=bytes(s)
    work=(c.c_uint64*(libs[2].dnbx_work_size()//8))()
    l=(c.c_float*4)(123,123,123,123);r=(c.c_float*4)(456,456,456,456)
    for frames in (0,4):
        assert libs[2].dnbx_render(s,l,r,frames,work)==-5
        assert bytes(s)==before and list(l)==[123]*4 and list(r)==[456]*4 and not any(work)

    ss=[[state(lib),state(lib)] for lib in libs]
    hashes=[hashlib.sha256() for lib in libs]
    p=Parameters.from_buffer_copy(base)
    p.amp.attack_s=.01013;p.amp.hold_s=0;p.amp.decay_s=.05;p.amp.release_hold_s=.0037;p.amp.release_s=.1
    p.filter.attack_s=.02017;p.filter.decay_s=.04;p.lfo_hz=16;p.lfo_pwm_depth=.47;p.waveform=.85
    for lib,parts in zip(libs,ss):
        for part in parts:
            for layer in (0,1): assert lib.dnbx_set_layer(part,layer,c.byref(p))==0
            for voice in range(6): assert lib.dnbx_note_on(part,60+voice,127-voice*12)==voice
    events=[1001,2123,3007,5003,6001,7003,8191,9001,12001,12002,12999,16384]
    blocks=[1,3,5,7,13,16,64,127,256];frame=bi=ei=0;renders=0
    while frame<16384:
        if frame==events[ei]:
            for lib,parts in zip(libs,ss):
                for part in parts:
                    if frame==1001: assert lib.dnbx_note_off(part,60)==0
                    elif frame==2123: assert lib.dnbx_note_on(part,99,64)>=0
                    elif frame==3007: assert lib.dnbx_set_sustain(part,1)==0
                    elif frame==5003: assert lib.dnbx_note_off(part,61)==0
                    elif frame==6001: assert lib.dnbx_set_sustain(part,0)==0
                    elif frame==7003:
                        q=Parameters.from_buffer_copy(p);q.oscillator_mode=1;q.tune_semitones=12;q.gain=1.32
                        assert lib.dnbx_set_layer(part,1,c.byref(q))==0
                    elif frame==8191: assert lib.dnbx_set_velocity_mode(part,1)==0
                    elif frame==9001: assert lib.dnbx_store_raw(part,185,70)==1
                    elif frame==12001: assert lib.dnbx_all_off(part)==0
                    elif frame==12002: assert lib.dnbx_note_on(part,127,127)>=0
                    elif frame==12999: assert lib.dnbx_note_on(part,0,1)>=0
            ei+=1
        take=min(blocks[bi%len(blocks)],events[ei]-frame);bi+=1
        outputs=[]
        for lib,parts,digest in zip(libs,ss,hashes):
            chunk=b""
            for part in parts:
                l=(c.c_float*take)();r=(c.c_float*take)()
                assert lib.dnbx_render(part,l,r,take,None)==0
                chunk+=bytes(l)+bytes(r)
            outputs.append(chunk);digest.update(chunk)
        assert outputs[0]==outputs[1]==outputs[2],("audio",frame)
        assert bytes(ss[0][0])==bytes(ss[1][0])==bytes(ss[2][0]),("state0",frame)
        assert bytes(ss[0][1])==bytes(ss[1][1])==bytes(ss[2][1]),("state1",frame)
        frame+=take;renders+=1
    result={"status":"PASS","typed_boundary_admission_cases":count,
            "rendered_frames_per_part":frame,"parts":2,"irregular_render_calls_per_part":renders,
            "audio_and_state_bitwise_equal_all_calls":True,"work_null_for_audio_comparison":True,
            "audio_digest_all_profiles":[h.hexdigest() for h in hashes],
            "diagnostic_capability_on":1,"diagnostic_capability_off":0,
            "nonnull_work_disabled_rejected_atomically":True,
            "baseline_core_sha256":hashlib.sha256((ROOT/"src/dnbx_core.c").read_bytes()).hexdigest(),
            "variant_core_sha256":hashlib.sha256((HERE/"dnbx_core.c").read_bytes()).hexdigest(),
            "variant_header_sha256":hashlib.sha256((HERE/"dnbx_core.h").read_bytes()).hexdigest()}
    (HERE/"EQUIVALENCE.json").write_text(json.dumps(result,indent=2)+"\n")
    print(json.dumps(result,indent=2))

if __name__=="__main__":main()
