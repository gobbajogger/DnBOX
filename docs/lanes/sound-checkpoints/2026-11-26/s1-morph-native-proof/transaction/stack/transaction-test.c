#include <stdio.h>
#include <string.h>
#include "drum909.c"
void plat_cpu2_sync(void) {}
void engine_d9_noop(void) {}
static unsigned r=0x42ad3b37u;
static unsigned rng(void){r^=r<<13;r^=r>>17;r^=r<<5;return r;}
int main(void){
  static drum909_t a,b;
  unsigned ran=0,fallback=0;
  for(unsigned i=0;i<16000;i++){
    memset(&a,0,sizeof a); drum909_init(&a);
    for(int voice=0;voice<DR_NUM;++voice)
      if(rng()&1) drum909_trigger(&a,voice,(rng()&255)*0.003f);
    unsigned voice=DR_CH+i%4, n=1+i%D9_FUTURE_MAX, mask=rng()&0x7ff;
    drum909_trigger(&a,voice,0.71f);
    d9_smp_t *s=&a.smp[voice-DR_CH];
    if(!s->playing) continue;
    s->inc=(i>>3)&3; s->incf=(i&64)?0x80000000u:0;
    if(!s->inc&&!s->incf)s->inc=1;
    s->drive=(i&1)?0.5f:0.f;
    if((i&15)==0)s->len=2;
    if((i&7)==1)s->mute=8;
    if(i&2) { a.pan_l[voice]=0.513f; a.pan_r[voice]=0.347f; }
    b=a;
    d9_future_t f={.voice=*s};
    float au[4][256]={{0}},bu[4][256]={{0}};
    d9_future_t *volatile ready=0;
    if(i&4) { drum909_future_run(&f,n); ready=&f; ++ran; }
    else { ++fallback; }
    drum909_render_st_cleared(&a,au[0],au[1],au[2],au[3],n,mask);
    drum909_render_st_future(&b,bu[0],bu[1],bu[2],bu[3],n,mask,voice,&ready);
    if(!(i&4)) drum909_future_run(&f,n); /* ignored late publish */
    if(memcmp(&a,&b,sizeof a)||memcmp(au,bu,sizeof au)){
      fprintf(stderr,"divergence at %u note %u n=%u committed=%u\n",i,voice,n,(i&4)!=0);return 1;
    }
  }
  printf("PASS %u published/%u unavailable futures, complete bank owner state plus all 4 PCM buses\n",ran,fallback);
}
