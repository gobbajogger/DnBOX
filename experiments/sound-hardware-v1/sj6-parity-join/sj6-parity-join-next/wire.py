from pathlib import Path
import sys
p=Path(sys.argv[1])
flag='#if defined(X0X_HOST) && DNBOX_SJ6_JOIN_LAB'
def replace(path, old, new):
    fn=p/path;s=fn.read_text()
    assert s.count(old)==1,(path,old)
    fn.write_text(s.replace(old,new))
replace('dsp/juno6.h','    float current,target,fb,fb_target,wet,wet_target,inv_stride,sum[2],step[3];','''    float current,target,fb,fb_target,wet,wet_target,inv_stride,sum[2],step[3];
'''+flag+'''
    void *lab_fullrate;
    float lab_frames,lab_feedback,lab_dry;
#endif''')
replace('dsp/juno6_delay_native.inc','''    if (!s->delay24.stride || !s->delay24.tap) return;''', flag+'''
    if (s->delay24.lab_fullrate) {
        extern void sj6_params(void *,double,float,float,int);
        extern void sj6_process(void *,size_t,const float *,const float *,float *,float *);
        const float rate=s->core.sample_rate;
        for (unsigned i=0; i<n;) {
            /* Same reached eight-frame cadence as source; callback splits
             * carry through, and owner-private float history is not reset. */
            unsigned k=8u-(s->delay24.tick&7u);
            if(k>n-i)k=n-i;
            sj6_params(s->delay24.lab_fullrate,(double)s->delay24.lab_frames/(double)rate,
                       s->delay24.lab_feedback,s->delay24.lab_dry,0);
            sj6_process(s->delay24.lab_fullrate,k,left+i,right+i,left+i,right+i);
            i+=k;s->delay24.tick+=k;
        }
        return;
    }
#endif
    if (!s->delay24.stride || !s->delay24.tap) return;''')
replace('dsp/juno6_unison_native.inc','        u->step[i]=1.f-fm_exp2f((float)(i+1u)*st*.0833333f);',flag+'''
        u->step[i]=1001.f-fm_exp2f((float)(i+1u)*st*.08333329856395721f);
#else
        u->step[i]=1.f-fm_exp2f((float)(i+1u)*st*.0833333f);
#endif''')
replace('dsp/juno6_unison_native.inc','''            if(h<0.f)h+=1000.f;''',flag+'''
            if(h>=1000.f)h-=1000.f;
#else
            if(h<0.f)h+=1000.f;
#endif''')
replace('app/original_runtime.h','    original_runtime_backend dsp;','''    original_runtime_backend dsp;
'''+flag+'''
    struct RoomOwner *lab_room;
    const struct RoomBlock *lab_room_block;
#endif''')
replace('app/original_runtime.c','#include "../dsp/sj6_native_room.inc"','#include "../dsp/sj6_native_room.inc"\n'+flag+'\n#include "room.h"\n#endif')
key='''#if X0X_SJ6_NATIVE_ROOM
            sj6_crush_process(p,left,right,frames);
            sj6_room_process(&p->dsp.juno.room,left,right,frames);
#endif
#if X0X_SJ6_NATIVE_LIMITER
            sj6_limiter_process(&p->dsp.juno.limiter,left,right,frames);
#endif'''
replace('app/original_runtime.c',key,flag+'''
            if(p->lab_room_block) {
                /* Correct branch order; wet feed is explicitly provided by
                 * the verifier independently after the original wet HP.
                 * Compact Rage here is not an Apple-DSP match claim. */
                sj6_limiter_process(&p->dsp.juno.limiter,left,right,frames);
                compose_room_tick(p->lab_room,p->lab_room_block,left,right,frames);
            } else {
#endif
'''+key+'\n'+flag+'\n            }\n#endif')
