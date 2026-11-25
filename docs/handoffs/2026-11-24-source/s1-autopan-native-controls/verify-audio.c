/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1u.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/* Frozen receiver arithmetic from the prior exact 893 host source.*/
static void prior(float *ph,float y,float rate,float amount,float sr,float *l,float *r){
  const uint32_t mask=0xffffffu;
  const uint32_t a=(uint32_t)(*ph*16777216.f)&mask;
  float pan=s1u_sin(a<<8)*amount;
  uint32_t inc=(uint32_t)lrintf(rate*(float)(16777216.0/sr));
  *ph=(float)((a+inc)&mask)*(1.f/16777216.f);
  uint32_t pa=(uint32_t)((1.f+pan)*.125f*4294967296.f);
  *l=y*s1u_sin(pa+0x40000000u);*r=y*s1u_sin(pa);
}
static uint32_t seed=0x52413c91u;
static uint32_t step(void){seed^=seed<<13; seed^=seed>>17; seed^=seed<<5; return seed;}
int main(void){
 const float sample_rates[]={11025.f,22050.f,44100.f,48000.f};
 for(unsigned s=0;s<4;s++){
  float a=.97344f,b=a;
  for(unsigned i=0;i<65536;i++){
   const uint32_t h=step();
   const float y=(float)(int16_t)h*(1.f/32768.f),r=(float)(h&4095u)*(10.f/4096.f);
   const float amt=(float)((h>>12)&511u)*(1.f/511.f);
   float x[2],z[2];
   prior(&a,y,r,amt,sample_rates[s],&x[0],&x[1]);
   s1u_autopan_authored(&b,y,r,amt,sample_rates[s],&z[0],&z[1]);
   assert(!memcmp(x,z,sizeof x));assert(!memcmp(&a,&b,sizeof a));
  }
 }
 puts("262144 stereo frames plus live phase bit-identical to the prior receiver, including 65536 at native 44100 Hz");
}
