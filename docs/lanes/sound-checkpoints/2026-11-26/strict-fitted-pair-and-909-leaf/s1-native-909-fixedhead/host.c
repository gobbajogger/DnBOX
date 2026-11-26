#include <stdio.h>
#include <string.h>
#include "drum909.h"
extern void ref_drum909_init(drum909_t*);
extern void ref_drum909_trigger(drum909_t*,int,float);
extern void ref_drum909_render_st_cleared(drum909_t*,float*,float*,float*,float*,int,uint16_t);
static unsigned r=0x6b3f9d21;
static unsigned rng(void){r^=r<<13;r^=r>>17;r^=r<<5;return r;}
int main(void){
 unsigned pass=0;
 for(unsigned i=0;i<8000;i++){
  drum909_t a={0},b={0}; drum909_init(&a);ref_drum909_init(&b);
  for(int k=0;k<DR_NUM;k++)if((rng()&7)<5){
    float g=(rng()&127)/127.f;
    drum909_trigger(&a,k,g);ref_drum909_trigger(&b,k,g);
  }
  for(unsigned k=0;k<4;k++){
   d9_smp_t *s=a.smp+k,*q=b.smp+k;
   s->inc=q->inc=(i&1)?1:2; s->incf=q->incf=(i&3)?0:0x40000000u;
   s->drive=q->drive=(i&8)?0.5f:0;
   if((i&31)==0&&s->playing)s->len=q->len=2;
  }
  for(int block=0;block<7;block++){
   float x[4][256]={{0}},y[4][256]={{0}};
   unsigned n=1+(rng()&255),mask=rng()&2047;
   drum909_render_st_cleared(&a,x[0],x[1],x[2],x[3],n,mask);
   ref_drum909_render_st_cleared(&b,y[0],y[1],y[2],y[3],n,mask);
   drum909_t c=a,d=b;
   for(int k=0;k<4;k++)c.smp[k].adpcm=d.smp[k].adpcm=0;
   if(memcmp(&c,&d,sizeof c)||memcmp(x,y,sizeof x)){
     fprintf(stderr,"diff %u block %d %u\n",i,block,n);return 1;
   }
   ++pass;
  }
 }
 printf("PASS %u complete-state/four-bus serial exact spans\n",pass);
}
