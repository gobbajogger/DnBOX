/* Included by the typed SJ6 provider, opt-in pending live LFO arbitration proof.
 * Uses its existing 80 Hz frame clock, not the host application's wall timer. */
#if X0X_SJ6_AUX_LFO_NATIVE
#include "juno6_aux_catalog.h"
#endif
#define SJ6_NOTE_CUT_EPS 0x1.0624dep-11f
static float sj6_note_cut_rebase(float target,float old,float next)
{
    return fm_clampf((target-old)+next,0.f,1.f);
}
int juno6_cutoff_note_qualifies(const juno6_t *s)
{
#if X0X_SJ6_LFO1_DELAY_PORT
    if(!s->typed || juno6_lfo1_delay_seconds(s->lfo1_delay_raw)<.01f)return 0;
#if X0X_SJ6_AUX_LFO_NATIVE
    if(s->lfo1_aux_route)return sj6_aux_destination_id(s->lfo1_aux_route)==7;
#endif
    return s->lfo1_pwm_route==3u;
#else
    (void)s;
    return 0;
#endif
}
void juno6_cutoff_first_note(juno6_t *s)
{
    float target=s->lfo1_pwm_base;
#if X0X_SJ6_AUX_LFO_NATIVE
    if(s->lfo1_aux_route)target=s->lfo1_aux_base*(1.f/127.f);
#endif
    if(!s->note_cut_latched) {
        if(fm_fabsf(target-s->macro[2])<=SJ6_NOTE_CUT_EPS)return;
        s->note_cut_current=s->macro[2];
    }
    s->note_cut_target=target;
    s->note_cut_latched=1;
}
static float sj6_note_cut_tick(juno6_t *s)
{
    const float target=s->note_cut_target;
    const float curr=s->note_cut_current;
    const float value=fm_clampf(curr+(target-curr)*0x1.217c58p-1f,0.f,1.f);
    s->note_cut_current=value;
    if(fm_fabsf(value-target)<=SJ6_NOTE_CUT_EPS) {
        s->note_cut_current=target;
        if(fm_fabsf(s->macro[2]-target)<=SJ6_NOTE_CUT_EPS)
            s->note_cut_latched=0;
    }
    return value;
}
