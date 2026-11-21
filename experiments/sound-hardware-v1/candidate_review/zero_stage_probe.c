#include "dnbx_core.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
extern void review_reset(void);
extern void review_get_trace(dnbx_work *,uint64_t *);
int main(void) {
    dnbx_part s[2];dnbx_layer_params p;float l[256],r[256];
    dnbx_default_layer_params(&p);
    p.amp=(dnbx_envelope_params){0,0,0,1,0,0};p.filter=p.amp;
    for(unsigned i=0;i<2;i++) {
        dnbx_init(&s[i],44100);
        for(unsigned j=0;j<2;j++) dnbx_set_layer(&s[i],j,&p);
        dnbx_note_on(&s[i],60,127);
    }
    printf("{\"scenarios\":[");
    for(unsigned off=0;off<2;off++) {
        dnbx_work public={0},actual={0};uint64_t math_calls[2];review_reset();
        for(unsigned i=0;i<2;i++) {
            if(off)dnbx_note_off(&s[i],60);else dnbx_note_on(&s[i],60,127);
            for(unsigned j=0;j<2;j++) dnbx_render(&s[i],l,r,256,&public);
        }
        review_get_trace(&actual,math_calls);
        if(off)printf(",");
        printf("{\"events\":\"two_zero_stage_%s\",\"public_stage_transitions\":%"PRIu64",\"actual_stage_transitions\":%"PRIu64",\"public_exp2\":%"PRIu64",\"actual_direct_exp2\":%"PRIu64",\"active_voices_after\":%d}",off?"note_offs":"held_retriggers",public.stage_transitions,actual.stage_transitions,public.exp2_calls,math_calls[0],dnbx_active_voices(&s[0])+dnbx_active_voices(&s[1]));
    }
    printf("]}\n");return 0;
}
