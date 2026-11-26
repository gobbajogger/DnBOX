from pathlib import Path
from hashlib import sha256
import sys
b=Path(sys.argv[1]).read_bytes()
assert sha256(b).hexdigest()=="c071c4b804d2053717da5a74dd1fceb63c21773cfecd73685369596de99cf675"
s=b.decode()
def replace(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
replace('#include <stdlib.h>','#include <stdlib.h>\n#include <stdint.h>')
replace('\tFAUSTFLOAT fHslider3;\n} compressor;', '''\tFAUSTFLOAT fHslider3;
#if DNBOX_S1_COMP_COEF_CACHE
    uint32_t coef_atk, coef_ratio, coef_rel;
    float coef_slow[4];
    uint32_t coef_valid;
#endif
} compressor;
#if DNBOX_S1_COMP_COEF_CACHE
static uint32_t coef_word(float v) { union {float f; uint32_t u;} b={.f=v}; return b.u; }
#endif''')
replace('\tdsp->fSamplingFreq = samplingFreq;', '''\tdsp->fSamplingFreq = samplingFreq;
#if DNBOX_S1_COMP_COEF_CACHE
    dsp->coef_valid=0;
#endif''')
start='\tfloat fSlow0 = (float)dsp->fHslider0;'
end='\tfloat fSlow5 = (float)dsp->fHslider3;'
i,j=s.index(start),s.index(end); original=s[i:j]
cache='''#if DNBOX_S1_COMP_COEF_CACHE
    const float atk=(float)dsp->fHslider0, ratio=(float)dsp->fHslider1, rel=(float)dsp->fHslider2;
    const uint32_t ka=coef_word(atk), kr=coef_word(ratio), kl=coef_word(rel);
    if (!dsp->coef_valid || ka!=dsp->coef_atk || kr!=dsp->coef_ratio || kl!=dsp->coef_rel) {
        dsp->coef_slow[0]=exp((0.f-(dsp->fConst1/atk)));
        dsp->coef_slow[1]=((1.f-dsp->coef_slow[0])*((1.f/ratio)-1.f));
        dsp->coef_slow[2]=exp((0.f-(dsp->fConst2/atk)));
        dsp->coef_slow[3]=exp((0.f-(dsp->fConst2/rel)));
        dsp->coef_atk=ka; dsp->coef_ratio=kr; dsp->coef_rel=kl;
        dsp->coef_valid=1;
    }
    const float fSlow1=dsp->coef_slow[0], fSlow2=dsp->coef_slow[1];
    const float fSlow3=dsp->coef_slow[2], fSlow4=dsp->coef_slow[3];
#else
'''
s=s[:i]+cache+original+'#endif\n'+s[j:]
old=next(l for l in s.splitlines() if 'dsp->fRec0[0] = (' in l)
replace(old,'''#if DNBOX_S1_COMP_ONE_LOG
            const double over = (20.f * log10(dsp->fRec1[0])) - fSlow5;
            dsp->fRec0[0] = ((fSlow1 * dsp->fRec0[1]) + (fSlow2 * max(over, 0.f)));
#else
'''+old+'\n#endif')
Path(sys.argv[2]).write_text(s)
