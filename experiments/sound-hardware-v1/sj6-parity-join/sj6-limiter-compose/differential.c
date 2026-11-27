#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "sj6_native_limiter.h"
#include "sj6_native_room.h"
#include "fastmath.h"
#include "leaves.h"
int main(void){
 enum { N=16773 };
 static float ol[N],orr[N];
 unsigned changed=0;
 for(unsigned mode=0;mode<3;++mode){
  sj6_room_t probe={0},shadow={0};
  sj6_limiter_t lim={.15f,14.f,.037f,1.113f,1.f,(uint8_t)(mode!=0)}, ref=lim;
  probe.armed=2;probe.type=1;probe.enable=1;probe.fb=.67f;
  probe.hp_coeff=.012f;probe.mix=.41f;
  shadow=probe;
  for(unsigned i=0;i<N;++i){
   if(i==4357)probe.enable=shadow.enable=0; /* retained tail */
   if(i==7811)probe.enable=shadow.enable=1;
   float l=((int)(i*14771u%65536)-32768)*(1.f/43560.f);
   float r=((int)(i*7169u%65536)-32768)*(1.f/65321.f);
   if(i>14000 || i%67==0)l=r=i&1?-0.f:0.f;
   if(mode==2){l*=.5f;r*=-.25f;}
   float dryl=l,dryr=r,rooml=l,roomr=r;
   old_limit(&ref,&dryl,&dryr,1);
   old_room(&shadow,&rooml,&roomr,1);
   float t=(float)shadow.tick*(1.f/SJ6_ROOM_STRIDE);
   float eL=dryl+(shadow.last[0]+t*(shadow.wet[0]-shadow.last[0]))*shadow.mix;
   float eR=dryr+(shadow.last[1]+t*(shadow.wet[1]-shadow.last[1]))*shadow.mix;
   sj6_room_process(&probe,&l,&r,1,lim.armed?&lim:0);
   assert(memcmp(&eL,&l,4)==0&&memcmp(&eR,&r,4)==0);
   assert(memcmp(&probe,&shadow,sizeof probe)==0&&memcmp(&lim,&ref,sizeof lim)==0);
   if(!mode)assert(memcmp(&l,&rooml,4)==0&&memcmp(&r,&roomr,4)==0);
   changed+=(memcmp(&rooml,&l,4)!=0||memcmp(&roomr,&r,4)!=0);
   ol[i]=l;orr[i]=r;
  }
 }
 assert(changed>15000);
 printf("%u exact signed graph frames, %u changed from uncompressed dry+wet, unchanged room and dry-limiter histories\n",3u*N,changed);
}
