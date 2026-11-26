#include "juno6.h"
#include <assert.h>
#include <stdio.h>
static juno6_t s;
static float l[128],r[128];
int main(void){
 juno6_init(&s,44100.f);
 assert(juno6_set(&s,2,.633f));
 juno6_set_aux_lfo1(&s,3,71); /* #129 Harmonic, not #7 physical cutoff. */
 juno6_lfo1_delay_set(&s,45);
#if X0X_SJ6_NOTE_CUTOFF_LATCH
 assert(!juno6_cutoff_note_qualifies(&s));
#endif
 for(unsigned b=0;b<40;b++){
   if(b==0)assert(juno6_note_on(&s,60,86));
   if(b==6)assert(juno6_note_on(&s,65,101));
   if(b==9)assert(juno6_set(&s,2,.45f));
   if(b==12)juno6_note_off(&s,60);
   if(b==22)juno6_note_off(&s,65);
   assert(juno6_render_stereo(&s,l,r,128));
   assert(fwrite(l,sizeof l,1,stdout)==1 && fwrite(r,sizeof r,1,stdout)==1);
 }
}
