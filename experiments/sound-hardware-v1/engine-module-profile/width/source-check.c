#include "soundpipe.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static uint32_t seed=12321;
static float signed_in(unsigned i) {
    uint32_t v=seed=1664525u*seed+1013904223u;
    if (!(i%19)) return i&1 ? -0.f : 0.f;
    if (i%2205==1) return -0.f;
    return (float)(int32_t)v*0x1p-32f;
}
static int eq(float a,float b){return !memcmp(&a,&b,sizeof a);}
int main(void) {
 sp_data sp={.sr=44100};
 sp_revsc *orig=NULL;
 assert(sp_revsc_create(&orig)==SP_OK&&orig);
 assert(sp_revsc_init(&sp,orig)==SP_OK);
 printf("RevSC auxiliary bytes=%zu host struct bytes=%zu widths=",orig->aux.size,sizeof *orig);
 for(unsigned j=0;j<8;j++)printf("%s%d",j?",":"",orig->delayLines[j].bufferSize);
 puts("");sp_revsc_destroy(&orig);
 static float state[2][2205];
 unsigned pos[2]={0};
 sp_delay *oracle[2];
 for(unsigned j=0;j<2;j++){
  assert(sp_delay_create(oracle+j)==SP_OK&&oracle[j]);
  assert(sp_delay_init(&sp,oracle[j],.05f)==SP_OK&&oracle[j]->bufsize==2205u);
 }
 unsigned n=0;
 for(unsigned i=0;i<700000;i++){
    for(unsigned j=0;j<2;j++){
      if((i%1017==2)&&j==0)continue;
      if((i==12000&&j==0)||(i==111111&&j==1)){
        sp_delay_destroy(&oracle[j]);sp_delay_create(&oracle[j]);sp_delay_init(&sp,oracle[j],.05f);
        memset(state[j],0,sizeof state[j]);pos[j]=0;
      }
      float r=signed_in(i),a=i%11<3?0.f:i%11<5?1.f:((float)(i%127)/127.f);
      float o;
      assert(sp_delay_compute(&sp,oracle[j],&r,&o)==SP_OK);
      float ref=a*o+(1.f-a)*r;
      const unsigned p=pos[j]; const float delayed=state[j][p];state[j][p]=delayed*0.f+r;
      pos[j]=p+1u==2205u?0u:p+1u;
      const float out=a*delayed+(1.f-a)*r;
      assert(eq(o,delayed)&&eq(ref,out)&&eq(((float*)oracle[j]->buf.ptr)[p],state[j][p])&&oracle[j]->bufpos==pos[j]);n++;
    }
 }
 for(unsigned j=0;j<2;j++)sp_delay_destroy(&oracle[j]);
 printf("width: %u signed exact compared frames; two independent owners, width live edits, two clears and sparse span clocks\n",n);
}
