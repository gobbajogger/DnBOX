/* Host witness only. This tests shadow rendering and serial mixer commit;
 * it is not a firmware scheduler or inter-core publication implementation. */
#include <stdio.h>
#include <string.h>
#include "drum909.c"
enum { CAP=220 };
typedef struct {d9_smp_t state;unsigned frames;float pcm[CAP];} local_future;
static unsigned seed=0x4ab314au;
static unsigned rng32(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void fill(float a[4][CAP]){
 for(unsigned i=0;i<4;i++)for(unsigned j=0;j<CAP;j++)
  a[i][j]=(float)(int32_t)(rng32()&511u)*0.01f;
}
static void prepare(local_future *future,const d9_smp_t *old,unsigned n)
{
 future->state=*old;
 d9_bus_t d={.y=future->pcm};
 future->frames=(unsigned)old->mute<n?(unsigned)old->mute:n;
 d9_render_smp(&future->state,&d,(int)n);
}
int main(void){
 unsigned passes=0,cancel=0;
 for(unsigned trial=0;trial<3200;trial++){
  drum909_t a={0};drum909_init(&a);
  unsigned idx=trial%4u;
  drum909_trigger(&a,DR_CH+(int)idx,(float)(trial%10u+1u)*0.08f);
  d9_smp_t old=a.smp[idx];
  old.inc=(trial>>3)&3u;old.incf=(trial&64u)?0x80000000u:0;
  if(!old.inc&&!old.incf)old.inc=1;
  old.drive=(trial&1u)?0.5f:0.f;
  if((trial&15u)==0)old.len=2;
  if((trial&7u)==1)old.mute=8;
  const unsigned n=1u+trial%CAP;
  const d9_smp_t untouched=old;
  d9_smp_t reference=old; local_future future={0};
  float x[4][CAP],y[4][CAP],raw[CAP];
  fill(x);memcpy(y,x,sizeof x);
  d9_bus_t bx={.dry=x[0],.dry_r=(trial&1u)?x[1]:0,.rev=(trial&2u)?x[2]:0,.dly=(trial&4u)?x[3]:0,
      .srev=.35f,.sdly=.51f,.pl=.59f,.pr=.21f,.dpl=(trial&8u)?.00003f:0.f,.dpr=(trial&16u)?-.000011f:0.f,.y=raw};
  d9_bus_t by=bx;by.dry=y[0];by.dry_r=bx.dry_r?y[1]:0;by.rev=bx.rev?y[2]:0;by.dly=bx.dly?y[3]:0;by.y=future.pcm;
  d9_render_smp(&reference,&bx,(int)n);
  prepare(&future,&old,n);
  if(memcmp(&old,&untouched,sizeof old))return 1;
  if((trial&1u)==0){
   /* A would-be canceled shadow has not touched buses, live state, or tail.
    * This fresh original fallback proves commit can safely choose neither result. */
   by.y=future.pcm;
   d9_render_smp(&old,&by,(int)n);
   ++cancel;
  }else{
   old=future.state;d9_emit_block(&by,(int)future.frames);
  }
  if(memcmp(&old,&reference,sizeof old)||memcmp(x,y,sizeof x)){
    fprintf(stderr,"trial %u n %u\n",trial,n);return 2;
  }
  ++passes;
 }
 printf("PASS %u same-state/full-PCM host witness cases, %u discarded shadow with original serial retry\n",passes,cancel);
 return 0;
}
