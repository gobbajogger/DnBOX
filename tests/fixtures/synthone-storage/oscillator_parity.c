#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "soundpipe.h"
int ref_sp_osc_init(sp_data*,sp_osc*,sp_ftbl*,SPFLOAT);
int ref_sp_osc_compute(sp_data*,sp_osc*,SPFLOAT*,SPFLOAT*);
int ref_sp_fosc_init(sp_data*,sp_fosc*,sp_ftbl*);
int ref_sp_fosc_compute(sp_data*,sp_fosc*,SPFLOAT*,SPFLOAT*);
static uint32_t bits(float x){uint32_t u;memcpy(&u,&x,4);return u;}
int main(int argc,char**argv){assert(argc==2);float raw[4097];FILE*f=fopen(argv[1],"rb");assert(f&&fread(raw,4,4097,f)==4097);fclose(f);unsigned comparisons=0;
 for(unsigned mode=0;mode<2;mode++){
  if(mode){uint32_t q=123;for(unsigned i=0;i<4097;i++){q=q*1664525u+1013904223u;raw[i]=(float)(int32_t)(q>>8)/16777216.f-.5f;}raw[127]=raw[4095]=.25f;}
  sp_data sp={0};sp.sr=48000;
  sp_ftbl a={0},b={0};a.size=b.size=4096;a.lobits=b.lobits=12;a.lomask=b.lomask=4095;a.lodiv=b.lodiv=1.f/4096.f;a.sicvt=b.sicvt=(float)(1.0*SP_FT_MAXLEN/48000);a.tbl=raw;b.tbl=mode?raw:0;
  sp_osc x={0},y={0};sp_fosc u={0},v={0};assert(ref_sp_osc_init(&sp,&x,&a,0)==SP_OK&&sp_osc_init(&sp,&y,&b,0)==SP_OK);assert(ref_sp_fosc_init(&sp,&u,&a)==SP_OK&&sp_fosc_init(&sp,&v,&b)==SP_OK);
  for(unsigned i=0;i<65536;i++){float f1,f2,g1,g2;float hz=(float[]){0,30,440,1100,19999}[(i/128)%5];x.freq=y.freq=hz;u.freq=v.freq=hz;u.mod=v.mod=3;u.car=v.car=2;u.indx=v.indx=2;
   if(i%2048==0){u.mphs=v.mphs=127<<12;u.cphs=v.cphs=4095<<12;}
   assert(ref_sp_osc_compute(&sp,&x,0,&f1)==SP_OK&&sp_osc_compute(&sp,&y,0,&f2)==SP_OK);assert(ref_sp_fosc_compute(&sp,&u,0,&g1)==SP_OK&&sp_fosc_compute(&sp,&v,0,&g2)==SP_OK);
   assert(isfinite(f1)&&isfinite(f2)&&isfinite(g1)&&isfinite(g2));assert(bits(f1)==bits(f2)&&bits(g1)==bits(g2)&&x.lphs==y.lphs&&u.mphs==v.mphs&&u.cphs==v.cphs);comparisons+=2;
  }
 }
 printf("canonical+arbitrary nonnull oscillator outputs=%u phase parity PASS\n",comparisons);return 0;
}
