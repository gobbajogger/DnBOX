from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
k='static float factory_oscillator('
assert s.count(k)==1
i=s.index(k)
s=s[:i]+'''#if RAW_SAW_ABLATION
#include "sj6_main_4x.h"
sj6_main_4x eps_saw[2];
typedef struct {unsigned stop;float width;} eps_ctx;
static float eps_shape(float t,float dt,void *opaque)
{
    const eps_ctx *x=opaque;
    return factory_shape(x->stop,t,dt,x->width,0);
}
#endif
'''+s[i:]
a='                if (osc_gain[l]!=1.f) x*=osc_gain[l];'
b='''#if RAW_SAW_ABLATION
                if(i==0 && p->waveform>0.f && p->oscillator_mode==DNBX_OSC_SUPERJ6){
                    eps_ctx params={(unsigned)(p->waveform*3.f),width[l]};
#if RAW_SAW_ABLATION & 1
                    x=sj6_main_4x_sample(&eps_saw[l],lv->increment,eps_shape,&params);
#endif
#if RAW_SAW_ABLATION & 2
                    if(params.stop==2u)x*=1.5309427976608276f; /* recovered constructor, not estimated from capture */
#endif
                }
#endif
'''+a
assert s.count(a)==1
p.write_text(s.replace(a,b))
