#include "juno6.h"
#include <assert.h>
#include <stdio.h>
static juno6_t s;
static float l[256],r[256];
int main(void){
 juno6_init(&s,44100.f);
 assert(juno6_set(&s,2,1.f));
 juno6_set_aux_lfo1(&s,8,43);
 juno6_lfo1_delay_set(&s,0);
 assert(!juno6_cutoff_note_qualifies(&s));
 juno6_lfo1_delay_set(&s,45);
 juno6_set_aux_lfo1(&s,3,43);
 assert(!juno6_cutoff_note_qualifies(&s));
 juno6_set_aux_lfo1(&s,8,43);
 assert(juno6_cutoff_note_qualifies(&s));
 float prev=s.core.layer[0].params.cutoff_hz;
 juno6_cutoff_first_note(&s);
 assert(s.note_cut_latched && s.note_cut_current==1.f && s.note_cut_target==43.f/127.f);
 assert(juno6_note_on(&s,48,86));
 for(unsigned t=0;t<30;++t) assert(juno6_render_stereo(&s,l,r,256));
 assert(s.note_cut_latched && s.note_cut_current<.35f);
 assert(s.core.layer[0].params.cutoff_hz==prev && s.core.layer[1].params.cutoff_hz==prev);
 float applied=s.note_cut_current;
 assert(juno6_set(&s,2,43.f/127.f));
 assert(s.note_cut_target==0.f && s.note_cut_current==applied && s.macro[2]==43.f/127.f);
 for(unsigned t=0;t<30;++t) assert(juno6_render_stereo(&s,l,r,256));
 assert(s.note_cut_latched && s.note_cut_current<.001f);
 float read=0;
 assert(dnbx_read_raw(&s.core,7,&read)==DNBX_OK && read==s.macro[2]);
 juno6_init(&s,44100.f); juno6_set_aux_lfo1(&s,8,43);juno6_lfo1_delay_set(&s,45);
 assert(juno6_set(&s,2,43.f/127.f));
 juno6_cutoff_first_note(&s);
 assert(!s.note_cut_latched);
 puts("typed renderer: exact guard, target/applied/public separation, per-layer restore, relative rollback, authored no-op PASS");
}
