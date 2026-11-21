#!/usr/bin/env python3
"""Focused comparison of other public APIs affected by compact range()."""
import ctypes as C, hashlib, json, pathlib, struct
from verify_candidate import Api, bind, raw
from verify_compact_profile import bits, nextbits, value
ROOT=pathlib.Path(__file__).resolve().parent
OUT=ROOT/'compact_profile_review'

def samples(lo,hi):
    return {'low':bits(lo),'low_inside':nextbits(lo,True),'low_outside':nextbits(lo,False),
            'high':bits(hi),'high_inside':nextbits(hi,False),'high_outside':nextbits(hi,True),
            'plus_zero':0,'minus_zero':0x80000000,'plus_inf':0x7f800000,'minus_inf':0xff800000,
            'plus_qnan':0x7fc00001,'minus_qnan':0xffc00001,'plus_snan':0x7f800001,'minus_snan':0xff800001}

def main():
    apis={k:Api(bind(OUT/(k+'.so'))) for k in ('accepted','diagnostic','compact')};checks=[]
    for name,b in samples(8000,192000).items():
        v=value(b);expected=0 if 8000<=v<=192000 else -2;got=[];states=[];unchanged=True
        for api in apis.values():
            s=api.part();before=raw(s);x=C.c_float.from_buffer_copy(struct.pack('<I',b));got.append(api.d.dnbx_init(s,x));states.append(raw(s))
            if expected:unchanged &= before==raw(s)
        checks.append({'api':'dnbx_init','input':name,'bits':f'{b:08x}','expected':expected,'returns':got,'state_equal':len(set(states))==1,'unchanged_if_invalid':unchanged,'pass':got==[expected]*3 and len(set(states))==1 and unchanged})
    for fn in ('dnbx_attack_from_unit','dnbx_lfo_from_unit','dnbx_pwm_from_unit','dnbx_range_from_stop'):
        ss=samples(0,1);ss['mid_stop']=bits(.5)
        for name,b in ss.items():
            v=value(b);ok=(v in (0,.5,1)) if fn=='dnbx_range_from_stop' else 0<=v<=1;expected=0 if ok else -2
            got=[];outs=[];unchanged=True
            for api in apis.values():
                out=C.c_float(123.25);before=raw(out);x=C.c_float.from_buffer_copy(struct.pack('<I',b))
                got.append(getattr(api.d,fn)(x,C.byref(out)));outs.append(raw(out))
                if expected:unchanged &= before==raw(out)
            checks.append({'api':fn,'input':name,'bits':f'{b:08x}','expected':expected,'returns':got,'output_bits_equal':len(set(outs))==1,'unchanged_if_invalid':unchanged,'pass':got==[expected]*3 and len(set(outs))==1 and unchanged})
    for index,(field,lo,hi) in enumerate([('phase',0,1),('increment',.000001,.45),('waveform',0,1),('pulse_width',.03,.5)]):
        flo,fhi=value(bits(lo)),value(bits(hi))
        for name,b in samples(lo,hi).items():
            v=value(b);expected=0 if flo<=v<=fhi and not(index==0 and v==1) else -2;got=[];outs=[];unchanged=True
            for api in apis.values():
                args=[C.c_float(.3),C.c_float(.01),C.c_float(.8),C.c_float(.25)];args[index]=C.c_float.from_buffer_copy(struct.pack('<I',b));out=C.c_float(123.25);before=raw(out)
                got.append(api.d.dnbx_wave_probe_mode(*args,0,C.byref(out)));outs.append(raw(out))
                if expected:unchanged &= before==raw(out)
            checks.append({'api':'dnbx_wave_probe_mode','field':field,'input':name,'bits':f'{b:08x}','expected':expected,'returns':got,'output_bits_equal':len(set(outs))==1,'unchanged_if_invalid':unchanged,'pass':got==[expected]*3 and len(set(outs))==1 and unchanged})
    result={'scope':'Additional affected public range APIs, strict floating point; no timing repeated.','cases':checks,'case_count':len(checks),'all_pass':all(x['pass'] for x in checks),'harness_sha256':hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest()}
    (OUT/'PUBLIC-BOUNDARIES.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({'case_count':len(checks),'all_pass':result['all_pass']},indent=2))
    if not result['all_pass']:raise SystemExit(1)

if __name__=='__main__':main()
