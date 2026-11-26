#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "soundpipe.h"
#include "exact-delay.h"
#ifndef ROOMHIST_WIDTH
#error select declared width
#endif
int main(void){
 for(unsigned rate=44100;rate<=48000;rate+=3900){
   sp_data *sp=0; sp_create(&sp); assert(sp);sp->sr=rate;
   sp_revsc *rv[3]={0}; for(int i=0;i<3;i++){
     assert(sp_revsc_create(&rv[i])==SP_OK);assert(sp_revsc_init(sp,rv[i])==SP_OK);
     rv[i]->feedback=.98f; rv[i]->lpfreq=.5f*rate;
   }
   assert(rv[0]->aux.size==rv[1]->aux.size && rv[0]->aux.size==rv[2]->aux.size &&
          rv[0]->aux.ptr!=rv[1]->aux.ptr && rv[0]->aux.ptr!=rv[2]->aux.ptr);
   size_t total=0,original=0;
   dnbox_s1_exact_delay *c=dnbox_s1_delay_new(rate);assert(c);
   sp_vdelay *v; assert(sp_vdelay_create(&v)==SP_OK);
   assert(sp_vdelay_init(sp,v,10.f)==SP_OK);
   original=4*v->buf.size;total=dnbox_s1_delay_history_bytes(c);
   float x,y,z,t;
   for(unsigned i=0;i<2*rate;i++){
      float l=i%127u==0?.20f:0.f,r=i%193u==0?-.33f:0.f;
      assert(sp_revsc_compute(sp,rv[0],&l,&r,&x,&y)==SP_OK);
      float l2=i%23u==0?-.17f:0.f,r2=i%59u==0?.09f:0.f;
      assert(sp_revsc_compute(sp,rv[1],&l2,&r2,&z,&t)==SP_OK);
      assert(sp_revsc_compute(sp,rv[2],&l,&r,&z,&t)==SP_OK);
      assert(memcmp(&x,&z,4)==0&&memcmp(&y,&t,4)==0);
   }
   printf("{\"rate\":%u,\"roomRingRequested\":%zu,\"roomObject\":%zu,\"fixtureStatesCompared\":%u,\"delayRequestedStock\":%zu,\"delayRequestedExactShort\":%zu,\"widen50msPhysicalBytes\":%u,\"lineSamples\":[",rate,rv[0]->aux.size,sizeof *rv[0],rate*2,original,total,4*(rate/20));
   for(int i=0;i<8;i++)printf("%s%d",i?",":"",rv[0]->delayLines[i].bufferSize);
   printf("],\"width\":%d,\"hostIntegration44kOnly\":true}\n",ROOMHIST_WIDTH);
   for(int i=0;i<3;i++)sp_revsc_destroy(&rv[i]);sp_vdelay_destroy(&v);sp_destroy(&sp);dnbox_s1_delay_free(c);
 }
}
