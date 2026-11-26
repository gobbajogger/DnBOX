from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
mark='static float factory_oscillator('
i=s.index(mark)
s=s[:i]+'''#if defined(X0X_HOST) && DNBX_WAVE_FAMILY
#include "family.h"
#endif
'''+s[i:]
a='    float t = voice->phase, dt = voice->increment;'
b=a+'''
#if defined(X0X_HOST) && DNBX_WAVE_FAMILY
    if(stop<2u)
        return sj6_family(stop,t,dt);
#endif'''
assert s.count(a)==1;s=s.replace(a,b)
a='                lv->phase+=lv->increment; if (lv->phase>=1) lv->phase-=1;'
b='''
#if defined(X0X_HOST) && DNBX_WAVE_FAMILY
                if(p->oscillator_mode==DNBX_OSC_SUPERJ6 && p->waveform<2.f/3.f){
                    const float q=0.25f*lv->increment;
                    for(unsigned quarter=0;quarter<4;quarter++){
                        lv->phase+=q;
                        if(lv->phase>=1.f)lv->phase-=1.f;
                    }
                } else
#endif
                {lv->phase+=lv->increment; if (lv->phase>=1) lv->phase-=1;}
'''
assert s.count(a)==1;p.write_text(s.replace(a,b))
