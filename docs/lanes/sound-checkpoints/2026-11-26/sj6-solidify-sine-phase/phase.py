from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
orig='static float factory_oscillator('
lead='''#if DNBX_HOST_SINE_PHASE == 2
#include "sj6_main_4x.h"
static sj6_main_4x sine_owner[DNBX_VOICES][DNBX_LAYERS];
static inline void sine_reset(unsigned i) {
    memset(sine_owner[i],0,sizeof sine_owner[i]);
}
static float sine_cos(float p,float step,void *context) {
    (void)step; (void)context;
    return .5f*cosf(p*FM_TWO_PI);
}
#endif
'''
assert s.count(orig)==1
s=s.replace(orig,lead+orig)
a='if (!retrigger) memset(v,0,sizeof *v);'
assert s.count(a)==1
s=s.replace(a,'''if (!retrigger) {
#if DNBX_HOST_SINE_PHASE == 2
        sine_reset(chosen);
#endif
        memset(v,0,sizeof *v);
    }''')
a='if(!voice_active(v))memset(v,0,sizeof *v);'
assert s.count(a)==1
s=s.replace(a,'''if(!voice_active(v)) {
#if DNBX_HOST_SINE_PHASE == 2
        sine_reset(0);
#endif
        memset(v,0,sizeof *v);
    }''')
a='    memset(s->voice,0,sizeof s->voice); s->damper=0;'
assert s.count(a)==1
s=s.replace(a,a+'''
#if DNBX_HOST_SINE_PHASE == 2
    memset(sine_owner,0,sizeof sine_owner);
#endif''')
a='''                if (osc_gain[l]!=1.f) x*=osc_gain[l];'''
assert s.count(a)==1
s=s.replace(a,'''#if DNBX_HOST_SINE_PHASE
                if (s->superj6_amp_range && p->oscillator_mode==DNBX_OSC_SUPERJ6 && p->waveform==0.f) {
#if DNBX_HOST_SINE_PHASE == 2
                    x=sj6_main_4x_sample(&sine_owner[i][l],lv->increment,sine_cos,0);
#else
                    /* Exact quadrature origin, retain compact phase clock. */
                    float t=lv->phase+.25f;
                    if(t>=1.f)t-=1.f;
                    x=.5f*shape(0,t,lv->increment,width[l],DNBX_OSC_STOCK_BLEP,w);
#endif
                }
#endif
'''+a)
p.write_text(s)
