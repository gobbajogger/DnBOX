#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drum909.h"
extern volatile uint32_t d9_verify_good, d9_verify_missed, d9_verify_state, d9_verify_pcm;
void plat_cpu2_sync(void){}
extern void stripped_drum909_render_st_cleared(drum909_t*,float*,float*,float*,float*,int,uint16_t);
static uint32_t r=0x9f8c4891u;
static uint32_t rnd(void){r^=r<<13;r^=r>>17;r^=r<<5;return r;}
int main(void){
 for(unsigned z=0;z<7000;z++){
   drum909_t a={0},b={0}; drum909_init(&a); drum909_init(&b);
   for(unsigned i=0;i<DR_NUM;i++) if (rnd()%6!=0u || i==DR_RD){
     const float amp=(rnd()%126+1)* (1.f/127.f);
     drum909_trigger(&a,i,amp);drum909_trigger(&b,i,amp);
   }
   for(unsigned o=DR_CH;o<=DR_RD;o++){
     unsigned k=o-DR_CH;
     a.pan_l[o]=b.pan_l[o]=(float)(rnd()&127)*(.5f/127.f)+.5f;
     a.send_rev[o]=b.send_rev[o]=(float)(rnd()&127)*(1.f/127.f);
     a.send_dly[o]=b.send_dly[o]=(float)(rnd()&127)*(1.f/127.f);
     a.smp[k].inc=b.smp[k].inc=1+(rnd()&1);
     a.smp[k].incf=b.smp[k].incf=rnd();
   }
   for(unsigned n=0;n<8;n++){
     int count=1+(int)(rnd()%D9_FUTURE_MAX);
     unsigned w=DR_RD;while(w>=DR_CH && (!a.smp[w-DR_CH].playing || a.smp[w-DR_CH].mute<=0)) --w;
     d9_future_t f;d9_future_t *volatile ready=NULL; unsigned mask=rnd()&0x7ff;
     if(w>=DR_CH && n%5!=1){
       f.voice=a.smp[w-DR_CH]; drum909_future_run(&f,count);ready=&f;
     }
     float u[4][256]={{0}},v[4][256]={{0}};
     drum909_render_st_future(&a,u[0],u[1],u[2],u[3],count,(uint16_t)mask,w,&ready);
     stripped_drum909_render_st_cleared(&b,v[0],v[1],v[2],v[3],count,(uint16_t)mask);
     if(memcmp(&a,&b,sizeof a)||memcmp(u,v,sizeof u)) {fprintf(stderr,"case %u span %u mismatch\n",z,n);return 1;}
   }
 }
 if(!d9_verify_good || !d9_verify_missed || d9_verify_state || d9_verify_pcm) return 2;
 printf("PASS 56000 full-state/four-bus spans, duplicate ready %u missed %u, 0 mismatches\n",(unsigned)d9_verify_good,(unsigned)d9_verify_missed);
}
