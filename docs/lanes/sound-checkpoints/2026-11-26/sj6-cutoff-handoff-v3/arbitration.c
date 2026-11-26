#include "juno6.h"
#include <assert.h>
#include <stdio.h>
static juno6_t s;
static float l[256],r[256];
static void init(void){
 juno6_init(&s,44100.f);assert(juno6_set(&s,2,1.f));
 juno6_set_aux_lfo1(&s,8,43);
 juno6_lfo1_delay_set(&s,14);
 assert(juno6_cutoff_note_qualifies(&s));
 juno6_cutoff_first_note(&s);
 assert(s.note_cut_latched);
 assert(juno6_note_on(&s,48,86));
}
static void cycle(unsigned n){
 while(n--)assert(juno6_render_stereo(&s,l,r,256));
}
int main(void){
 init();
 cycle(4);
 float now=s.note_cut_current;
 assert(now<1.f&&now>s.note_cut_target);
 assert(juno6_set_mod_destination(&s,5));
 assert(juno6_set_mod_wheel(&s,90));
 float wheel=s.mod_wheel_cutoff_hz;
 cycle(16);
 assert(s.note_cut_latched && s.note_cut_current==now && s.mod_wheel_cutoff_hz==wheel);
 assert(juno6_set_mod_destination(&s,0));
 cycle(16);
 assert(s.note_cut_latched && s.note_cut_current<now && s.mod_wheel_cutoff_hz!=wheel);
 puts("resident CC1 owns actual cutoff override; note latch stays armed/frozen in this host policy");
 init();
 assert(dnbx_store_raw(&s.core,89u,.6f)==DNBX_RAW_STORED_ONLY);
 juno6_set_panel_lfo(&s,0,44,1);
 const float note_target=s.note_cut_target;
 cycle(4);
 assert(s.note_cut_latched && s.note_cut_target==note_target);
 cycle(200);
 fprintf(stderr,"armed=%u age=%u started=%u amount=%g route=%u oldCut=%g current=%g\n",s.note_cut_latched,s.lfo1_delay_age,s.lfo1_delay_started,s.core.raw[89],s.lfo1_aux_route,s.lfo1_cut_target,s.note_cut_current);
 assert(!s.note_cut_latched && s.lfo1_cut_target!=note_target);
 puts("resident active LFO supersedes typed note target after independently configured startup wait; app precedence not qualified");
 init();
 dnbx_layer_params layer;
 assert(dnbx_get_layer(&s.core,0,&layer)==DNBX_OK);
 layer.waveform=1.f;
 assert(dnbx_set_layer(&s.core,0,&layer)==DNBX_OK);
 assert(dnbx_store_raw(&s.core,210u,1.f)==DNBX_RAW_STORED_ONLY);
 juno6_set_lfo2_pwm(&s,(1u<<SJ6_LFO2_PWM_ROUTE_SHIFT)|50u);
 juno6_set_panel_lfo(&s,1,44,1);
 cycle(32);
 assert(s.lfo2_pwm_index!=0u);
 assert(s.note_cut_latched && s.note_cut_current<.35f);
 assert(s.core.layer[0].params.cutoff_hz==layer.cutoff_hz);
 puts("independent PWM and physical filter reach both actual render clocks; original layer restored");

}
