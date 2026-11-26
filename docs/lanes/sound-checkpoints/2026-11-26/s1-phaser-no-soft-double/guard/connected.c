#include "s1c.h"
#include "s1c_front.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
static s1c s;
static _Alignas(8) unsigned char room[4096],pool[4096];
static uint16_t widen[2205];
static float l[64],r[64];
int main(void){
 static const unsigned rates[]={43,42,97};
 for(unsigned k=0;k<3;k++){
  s1c_init(&s,44100.f,0);
  for(unsigned i=0;i<150;i++)assert(s1c_front_set(&s,i,s1c_front_init(i)));
  s1c_attach(&s,room,widen,pool,sizeof pool);
  /* actual typed manual/restore domain code: position43=>code43 (>INIT pos5) */
  const float rate=1.f+(299.f/127.f)*(float)rates[k];
  float lo,hi; s1c_front_bounds(144,&lo,&hi);
  assert(s1c_front_set(&s,144,lo));
  assert(s1c_front_set(&s,116,.55f));assert(s1c_front_set(&s,117,rate));
  assert(s1c_front_set(&s,118,.5f));assert(s1c_front_set(&s,119,800.f));
  s1c_start(&s,60,96);
  unsigned found=0;
  for(unsigned b=0;b<230000;b++){
   s1c_render(&s,l,r,64);
   const float z=6.28319f*100.f,span=.5f*(6.28319f*800.f-z),c=(1.f/44100.f)*5.0625f;
   float al=c*(span*(1.f-s.ph.fRec5[0])+z), ar=c*(span*(1.f-s.ph.fRec6[0])+z);
   if(fabsf(al)>.625f||fabsf(ar)>.625f){
    printf("pos %u rate %a port %a physical rate %a frame %u left %a right %a\n",rates[k],(double)rate,(double)lo,(double)s.ph.fHslider6,(b+1)*64,(double)al,(double)ar);
    found=1;break;
   }
  }
  assert(found);
 }
}
