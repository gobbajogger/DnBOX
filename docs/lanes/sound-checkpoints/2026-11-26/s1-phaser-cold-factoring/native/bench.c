#include <string.h>
#include "s1u.h"
#include "../fastmath.h"
#include SRC
#include "fm1_time.h"
#define AT ((volatile uint32_t*)0x01c08000)
#define O ((volatile uint32_t*)0x01c20000)
static uint32_t seed=0x1fa748eu;
static uint32_t rng(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
enum{N=512};
static float x[N],y[N],out[2][N];
static s1u_phaser p;
void bench(void){
 fm1_time_init();AT[0]=0x43554231;
 for(unsigned j=0;j<4;j++){
  memset(&p,0,sizeof p);
  float hz=800,rate=100,mix=.55f,fb=.5f;
  for(unsigned i=0;i<N;i++){
   x[i]=j==1?fc_float(rng()&0x807fffffu):(float)(int32_t)rng()*0x1p-33f;
   y[i]=j==1?fc_float(rng()&0x807fffffu):(float)(int32_t)rng()*0x1p-33f;
  }
  if(j==2){
    float l=0,r=0; s1u_phaser_run(&p,&l,&r,mix,rate,hz,fb,44100.f);
    /* Probe normal-finite cold range instead of burning 120s of silent emulation. */
    p.fRec5[1]=-1.25f;p.fRec6[1]=.04f;
  }
  unsigned t=fm1_ticks();
  for(unsigned i=0;i<N;i++){
    if(j==3){rate=1.f+(float)i*.58f;hz=100.f+(float)i*1.75f;}
    float l=x[i],r=y[i];
    s1u_phaser_run(&p,&l,&r,mix,rate,hz,fb,44100.f);
    out[0][i]=l;out[1][i]=r;
  }
  AT[4+j]=fm1_ticks()-t;
  uint32_t h=2166136261u;unsigned char*pb=(void*)&p;
  for(unsigned i=0;i<sizeof p;i++)h=(h^pb[i])*16777619u;
  AT[8+j]=h;
  for(unsigned i=0;i<N;i++){O[(j*2)*N+i]=fc_bits(out[0][i]);O[(j*2+1)*N+i]=fc_bits(out[1][i]);}
 }
 AT[1]=0x600d600d;for(;;)__asm__ volatile("" ::: "memory");
}
