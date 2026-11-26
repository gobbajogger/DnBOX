"""Host-only post-admission source-clock ownership across a live fallback.
Only exact pinned hoisted stage input. No header ABI or sound gain changes."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def only(x,y):
 global s
 assert s.count(x)==1,x
 s=s.replace(x,y,1)
mark='static float probe_shape(dnbx_layer_voice *v,unsigned shape){'
advance='''static inline float source_clock_step(float p,float q){
    const float q1=p+q,q2=q1+q,q3=q2+q,q4=q3+q;
    if(q4<1.f)return q4;
    for(unsigned j=0;j<4;j++){p+=q;if(p>=1.f)p-=1.f;}
    return p;
}
/* Reserved filter-envelope owner byte is not used for typed routing; do not
 * borrow AMP reserved because the sequencer owns it. Fresh voice memset
 * already clears this marker on allocation or steal. */
enum { SJ6_CLOCK_HELD = 0x80 };
'''
only(mark,advance+mark)
only('    const float q1=p+q,q2=q1+q,q3=q2+q,q4=q3+q;\n    if(q4<1.f)p=q4; /* monotonic positive phases prove no earlier wrap */\n    else for(unsigned j=0;j<4;j++){p+=q;if(p>=1.f)p-=1.f;}\n    v->phase=p;', '''    /* Advance once in the shared render-stage owner below. */''')
only('                if(fold<0)\n#endif\n                {lv->phase+=lv->increment; if(lv->phase>=1)lv->phase-=1;}', '''                if(fold>=0 || ((lv->filter.reserved&SJ6_CLOCK_HELD) && p->oscillator_mode==DNBX_OSC_SUPERJ6))
                    lv->phase=source_clock_step(lv->phase,.25f*lv->increment);
                else
#endif
                {lv->phase+=lv->increment; if(lv->phase>=1)lv->phase-=1;}''')
only('        if(reset)memset(v->oscillator_history,0,sizeof v->oscillator_history);', '''        if(reset){
            memset(v->oscillator_history,0,sizeof v->oscillator_history);
            if(p->oscillator_mode==DNBX_OSC_SUPERJ6 && p->waveform>=2.f/3.f && v->amp.stage!=DNBX_ENV_IDLE)
                v->filter.reserved|=SJ6_CLOCK_HELD;
        }''')
p.write_text(s)
