#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sub_owner.h"
#include "warm_amp.h"
#include "source_callback_clock.h"
struct event {unsigned time,key,velocity;};
#include "case.h"
typedef struct {uint64_t commit;float ending;unsigned key;} outcome;
static outcome run(int use_split,int without_chord)
{
    sj6_warm_amp sub_amp;
    sj6_amp_config_panel(&sub_amp,44100,capture_amp[0][0],capture_amp[0][1],capture_amp[0][2],capture_amp[0][3]);
    dnbox_sub_owner s={0};
    unsigned cursor=0,event=0;float stock_phase=0.f;
    outcome o={0};
    for(unsigned b=0;b<sizeof capture_blocks/sizeof *capture_blocks;b++){
        unsigned n=capture_blocks[b];
        while(event<sizeof capture_events/sizeof *capture_events && capture_events[event].time==cursor){
            const struct event q=capture_events[event++];
            const unsigned midi=q.key-12;
            if(without_chord && cursor>=49459)continue;
            if(q.velocity){
                int active=sub_amp.stage!=0;
                dnbox_sub_offer(&s,midi,active);
                if(active)sj6_amp_restart(&sub_amp);else sj6_amp_on(&sub_amp);
            } else if(s.active_key==midi || (s.pending&&s.pending_key==midi)){
                sj6_amp_off(&sub_amp);dnbox_sub_cancel(&s);
            }
        }
        const sj6_source_cb schedule={cursor,n};
        for(unsigned k=0;k<n;){
            unsigned z=use_split?3+k%7:n;if(z>n-k)z=n-k;
            for(unsigned a=0;a<z;a++){
                const unsigned f=cursor+k+a;
                const unsigned prep=sj6_source_prep_frames(schedule,f);
                if(!sub_amp.stage)continue;
                if(prep && sj6_amp_prep(&sub_amp,prep)){
                    assert(dnbox_sub_commit(&s)); o.commit=f;
                    assert(s.phase==0.f && s.active_key==55);
                    assert(stock_phase!=0.f);
                }
                if(!without_chord && f>=49459 && f<49875) {
                    assert(s.active_key==36 && s.pending_key==55 && s.pending);
                }
                if(f<49459)assert(s.phase==stock_phase);
                /* Constant independent marker step. This is not the real
                 * changing source glide, asset phase or playback cursor. */
                const float step=.0007f;
                dnbox_sub_advance(&s,step);
                stock_phase+=step;if(stock_phase>=1.f)stock_phase-=1.f;
                (void)sj6_amp_sample(&sub_amp);
            }
            k+=z;
        }
        cursor+=n;
    }
    assert(cursor==148684);o.ending=s.phase;o.key=s.active_key;return o;
}
int main(void)
{
    outcome a=run(0,0),b=run(1,0),c=run(1,1);
    assert(a.commit==49875&&b.commit==a.commit&&a.ending==b.ending&&a.key==b.key);
    assert(!c.commit&&c.key==36);
    dnbox_sub_owner v={0};
    dnbox_sub_offer(&v,36,0);dnbox_sub_advance(&v,.125f);
    dnbox_sub_offer(&v,48,1);dnbox_sub_offer(&v,52,1);
    assert(v.active_key==36&&v.pending_key==52&&v.phase==.125f);
    dnbox_sub_cancel(&v); assert(!dnbox_sub_commit(&v)&&v.phase==.125f);
    dnbox_sub_offer(&v,72,0);assert(v.active_key==72&&v.phase==0.f&&!v.pending);
    printf("Sub source clock: fixed old owner until %llu, phase reset, subdivisions/cold/cancel guards. state bytes=%zu replacement cursor delta=%zu independent host AMP bytes=%zu\n",(unsigned long long)a.commit,sizeof v,sizeof v-sizeof(float),sizeof(sj6_warm_amp));
    return 0;
}
