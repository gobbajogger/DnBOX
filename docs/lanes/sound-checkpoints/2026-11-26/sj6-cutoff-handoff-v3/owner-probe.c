#include "original_runtime.h"
#include <assert.h>
#include <stdio.h>
static original_runtime_part p;
int main(void){
 juno6_t *s=&p.dsp.juno.synth;
 p.status=ORIGINAL_RUNTIME_OK;
 p.kind=ORIGINAL_SOUND_JUNO;
 juno6_init(s,44100.f);
 assert(original_runtime_param(&p,2,1.f)==ORIGINAL_RUNTIME_OK);
 juno6_set_aux_lfo1(s,8,43);
 juno6_lfo1_delay_set(s,45);
 assert(original_runtime_key(&p,0,48,86)==ORIGINAL_RUNTIME_OK && s->note_cut_latched);
 assert(s->note_cut_target==43.f/127.f);
 assert(original_runtime_param(&p,2,.9f)==ORIGINAL_RUNTIME_OK);
 float shifted=s->note_cut_target;
 assert(shifted<43.f/127.f);
 assert(original_runtime_key(&p,1,60,86)==ORIGINAL_RUNTIME_OK && s->note_cut_target==shifted);
 assert(original_runtime_key(&p,1,64,86)==ORIGINAL_RUNTIME_OK && s->note_cut_target==shifted);
 assert(original_runtime_key(&p,1,60,0)==ORIGINAL_RUNTIME_OK);
 assert(original_runtime_key(&p,1,64,0)==ORIGINAL_RUNTIME_OK);
 assert(original_runtime_key(&p,1,60,86)==ORIGINAL_RUNTIME_OK && s->note_cut_target==shifted);
 assert(original_runtime_key(&p,1,60,0)==ORIGINAL_RUNTIME_OK);
 assert(original_runtime_key(&p,0,48,0)==ORIGINAL_RUNTIME_OK);
 assert(original_runtime_key(&p,0,48,86)==ORIGINAL_RUNTIME_OK && s->note_cut_target==43.f/127.f);
 assert(p.occupied_keys==1);
 assert(original_runtime_key(&p,0,48,86)==ORIGINAL_RUNTIME_OK && p.occupied_keys==1);
 assert(original_runtime_key(&p,0,48,0)==ORIGINAL_RUNTIME_OK && p.occupied_keys==1);
 assert(original_runtime_release_source(&p,0)==ORIGINAL_RUNTIME_OK && p.occupied_keys==0);
 const float native=.3345831036567688f;
 assert(original_runtime_param(&p,JUNO6_APP_PARAM_BASE+180u,native)==ORIGINAL_RUNTIME_OK);
 assert(original_runtime_key(&p,0,48,86)==ORIGINAL_RUNTIME_OK && s->note_cut_target==native);
 assert(original_runtime_cut_source(&p,0)==ORIGINAL_RUNTIME_OK && p.occupied_keys==0);
 assert(original_runtime_key(&p,0,48,0)==ORIGINAL_RUNTIME_OK && p.occupied_keys==0);
 juno6_set_aux_lfo1(s,8,44); /* Explicit panel edit supersedes exact imported value. */
 assert(original_runtime_key(&p,0,51,86)==ORIGINAL_RUNTIME_OK && s->note_cut_target==44.f/127.f);
 assert(original_runtime_all_off(&p)==ORIGINAL_RUNTIME_OK && p.occupied_keys==0);
 puts("exact authored #180 via existing runtime parameter path; counted ownership duplicate/release/cut/all-off PASS");
 puts("actual runtime owner: first note, multiple sources, chord, partial-off retrigger, empty-owner retrigger, relative live #7 write PASS");
}
