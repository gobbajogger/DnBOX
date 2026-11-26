"""Actual donor harness, deliberately never compiled unless host+variant."""
from pathlib import Path
import sys
src=Path(sys.argv[1]); s=src.read_text()
def only(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
mark='static float factory_oscillator('
model=Path(__file__).with_name('stage.c').read_text().split('/* Normal is deliberately separate')[0]
model=model.replace('#include "halfband.h"','').replace('anchor-generated.private.h','fold-data.h').replace('TBL[s][row][p]','TBL[TBL_MAP[s][row]][p]').replace('TBL[s][row][q]','TBL[TBL_MAP[s][row]][q]')
only(mark, '#if defined(X0X_HOST) && SJ6_WAVE_PROBE\n'+model+'''
static int route_shape(unsigned mode,float shape){
  return mode==DNBX_OSC_SUPERJ6 && shape<2.f/3.f;
}
static float probe_shape(dnbx_layer_voice *v,unsigned shape){
    const float step=v->increment,q=.25f*step;
    float p=v->phase;
    /* Delay's first and third coefficients derived from the exact physical filter. */
    float pdel=p-step*(0x1.f10bbbb0d49f9p-1f + 0x1.ce59426102823p+0f*step*step);
    if(pdel<0.f)pdel+=1.f;
    for(unsigned j=0;j<4;j++){p+=q;if(p>=1.f)p-=1.f;}
    v->phase=p;
    owner o={sj6_wavetable_band(step),shape};
    return shape_lookup(pdel,q,&o);
}
#endif
'''+mark)
only('    if (stop == 0) return .5f * shape','''#if defined(X0X_HOST) && SJ6_WAVE_PROBE
    if(stop<2)return probe_shape(voice,stop);
#endif
    if (stop == 0) return .5f * shape''')
only('                lv->phase+=lv->increment; if (lv->phase>=1) lv->phase-=1;', '''#if defined(X0X_HOST) && SJ6_WAVE_PROBE
                if(!route_shape(p->oscillator_mode,p->waveform))
#endif
                {lv->phase+=lv->increment; if(lv->phase>=1)lv->phase-=1;}
''')
only('    s->layer[layer].params=*p;', '''#if defined(X0X_HOST) && SJ6_WAVE_PROBE
    int reset=route_shape(p->oscillator_mode,p->waveform)!=route_shape(s->layer[layer].params.oscillator_mode,s->layer[layer].params.waveform);
#endif
    s->layer[layer].params=*p;''')
only('        /* A running timed segment retains its cached endpoint/rate. */', '''#if defined(X0X_HOST) && SJ6_WAVE_PROBE
        if(reset)memset(v->oscillator_history,0,sizeof v->oscillator_history);
#endif
        /* A running timed segment retains its cached endpoint/rate. */''')
only('                if (osc_gain[l]!=1.f) x*=osc_gain[l];','''                if (osc_gain[l]!=1.f) x*=osc_gain[l];
                extern float carrier_tap[2][128], raw_tap[2][128];
                carrier_tap[l][k]+=velocity*amp*x;
                raw_tap[l][k]+=x;
''')
src.write_text(s)
