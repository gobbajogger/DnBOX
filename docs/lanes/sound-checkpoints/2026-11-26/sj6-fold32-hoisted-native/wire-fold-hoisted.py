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
    float *cache=v->oscillator_history; /* existing 4 Float32 history words */
    if(step!=cache[0]){
        const sj6_band b=sj6_wavetable_band(step);
        cache[0]=step;
        cache[1]=step*(0x1.f10bbbb0d49f9p-1f + 0x1.ce59426102823p+0f*step*step);
        cache[2]=(float)b.row;cache[3]=b.blend;
    }
    float pdel=p-cache[1];
    if(pdel<0.f)pdel+=1.f;
    const float q1=p+q,q2=q1+q,q3=q2+q,q4=q3+q;
    if(q4<1.f)p=q4; /* monotonic positive phases prove no earlier wrap */
    else for(unsigned j=0;j<4;j++){p+=q;if(p>=1.f)p-=1.f;}
    v->phase=p;
    owner o={{(unsigned)cache[2],cache[3]},shape};
    return shape_lookup(pdel,q,&o);
}
#endif
'''+mark)
only('    for (unsigned l=0;l<DNBX_LAYERS;l++) fp[l]=filter_params(s,l,&linked[l]);', '    for (unsigned l=0;l<DNBX_LAYERS;l++) fp[l]=filter_params(s,l,&linked[l]);\n#if defined(X0X_HOST) && SJ6_WAVE_PROBE\n    int folded[2];\n    for(unsigned l=0;l<2;l++){\n        const dnbx_layer_params *a=&s->layer[l].params;\n        folded[l]=route_shape(a->oscillator_mode,a->waveform)?(int)(a->waveform*3.f):-1;\n    }\n#endif\n')
only('                float x = p->oscillator_mode == DNBX_OSC_SUPERJ6', '#if defined(X0X_HOST) && SJ6_WAVE_PROBE\n                const int fold=folded[l];\n                float x = fold>=0 ? probe_shape(lv,(unsigned)fold) :\n#else\n                float x =\n#endif\n                    p->oscillator_mode == DNBX_OSC_SUPERJ6')
only('                lv->phase+=lv->increment; if (lv->phase>=1) lv->phase-=1;', '#if defined(X0X_HOST) && SJ6_WAVE_PROBE\n                if(fold<0)\n#endif\n                {lv->phase+=lv->increment; if(lv->phase>=1)lv->phase-=1;}\n')
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
