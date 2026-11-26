#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "app/original_runtime.h"
#include "dsp/fastmath.h"
#include "dsp/juno6_unison_native.inc"
static original_runtime_part own[2];
static juno6_t baseline[2];
static int16_t old[2][2][SJ6_UNISON_HISTORY];
static unsigned long nchecked;
static void cold(unsigned i){
 // Retire-before-publish contract; no execution of installed image in this host witness.
 memset(&own[i],0,sizeof own[i]); memset(&baseline[i],0,sizeof baseline[i]);
 own[i].kind=ORIGINAL_SOUND_JUNO;
}
static void arm(unsigned i, const unsigned char b[4]){
 juno6_unison_config(&baseline[i],b,old[i]);juno6_unison_config(&own[i].dsp.juno.synth,b,own[i].dsp.juno.unison_history);
}
static void audio(unsigned i,unsigned frame,float detune,float stereo){
 float a[137],b[137],c[137],d[137];
 for(unsigned k=0;k<137;k++){a[k]=c[k]=.007f*((int)((frame+k)*113%199)-99);b[k]=d[k]=.004f*((int)((frame+k)*61%157)-78);}
 juno6_unison_process_mod(&baseline[i],a,b,137,detune,stereo);
 juno6_unison_process_mod(&own[i].dsp.juno.synth,c,d,137,detune,stereo);
 assert(!memcmp(a,c,sizeof a)&&!memcmp(b,d,sizeof b));
 nchecked+=137;
}
int main(void){
 const unsigned char a[]={1,71,103,5},b[]={1,77,32,3},dis[]={0,61,127,0};
 cold(0);cold(1); arm(0,a);arm(1,b);
 for(unsigned n=0;n<555;n++){
   if(n==140){arm(0,dis);arm(1,dis);}
   if(n==190){arm(0,b);arm(1,a);}
   if(n==313){
      own[0].kind=ORIGINAL_SOUND_SYNTHONE;
      memset(own[0].dsp.s1_wet_lease.delay,0xa5,sizeof own[0].dsp.s1_wet_lease.delay);
   }
   if(n==351){cold(0);arm(0,b);}
   if(own[0].kind==ORIGINAL_SOUND_JUNO)audio(0,n*137,n%4?.31f:-1.f,n%8?.45f:-1.f);
   audio(1,n*137,n%4?.31f:-1.f,n%8?.45f:-1.f);
 }
 printf("bitwise identical %lu stereo frames, two actual Unison DSP instances per arm; opposite-owner S1 lease write and cold return\n",nchecked);
}
