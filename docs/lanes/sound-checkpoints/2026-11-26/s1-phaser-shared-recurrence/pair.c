#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "s1u.h"
void old_run(s1u_phaser*,float*,float*,float,float,float,float,float);
void new_run(s1u_phaser*,float*,float*,float,float,float,float,float);
static uint32_t rng=0x893a0du;
static uint32_t randword(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static float fword(uint32_t x){float y;memcpy(&y,&x,4);return y;}
int main(void){
 s1u_phaser a[2]={{0}},b[2]={{0}};
 unsigned crossed=0;unsigned active=0;
 for(unsigned i=0;i<6100000;i++)for(unsigned o=0;o<2;o++){
   /* owner0 real valid panel rate detent43 continuous >119.57s; owner1 independent held/release/automation.
    * Valid signed tiny/zero samples introduced only in independent owner1. */
   float l=(float)(int32_t)randword()*0x1p-32f,r=(float)(int32_t)randword()*0x1p-32f;
   if(o&&(i&8192u)){l=fword(randword()&0x807fffffu);r=fword((i&1u)<<31);}
   float p=o?(i&2048u?.73f:0.f):.55f;
   float bpm=o?1.f+(299.f/127.f)*(float)((i>>15)&127u):1.f+299.f/127.f*43.f;
   float hz=o?100.f+900.f/127.f*(float)((i>>16)&127u):800.f;
   float fb=o?0.8f/127.f*(float)((i>>14)&127u):.5f;
   float u=l,v=r;
   old_run(a+o,&l,&r,p,bpm,hz,fb,o?48000.f:44100.f);
   new_run(b+o,&u,&v,p,bpm,hz,fb,o?48000.f:44100.f);
   if(memcmp(&l,&u,4)||memcmp(&r,&v,4)||memcmp(a+o,b+o,sizeof *a)){fprintf(stderr,"Mismatch at %u owner%u\n",i,o);return 1;}
   if(!o&&!crossed){
      float z=6.28319f*100.f,k=.5f*(6.28319f*800.f-z),w=(1.f/44100.f)*5.0625f;
      if(fabsf(w*(k*(1.f-a[o].fRec5[0])+z))>.625f||fabsf(w*(k*(1.f-a[o].fRec6[0])+z))>.625f)crossed=i;
   }
   active+=p!=0.f;
 }
 assert(crossed>0);
 printf("12200000 stereo owner calls, %u active, source and shared signed PCM and all %zu state bytes identical; cold range crossed at %u\n",active,sizeof *a,crossed);
}
