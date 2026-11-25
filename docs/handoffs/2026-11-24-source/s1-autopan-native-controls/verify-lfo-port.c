/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6_lfo_double_port.h"
#include <stdio.h>
#include <assert.h>
static float fl(uint32_t u){union{uint32_t u;float f;}v={.u=u};return v.f;}
static uint32_t fbits(float f){union{float f;uint32_t u;}v={.f=f};return v.u;}
static uint32_t seed=0x211678b1;
static uint32_t rng(void){seed^=seed<<13;seed^=seed>>17;return seed^=seed<<5;}
static void same(float b,float a,int i){
 double r=(double)b+((double)i/64.0)*(double)a*.5;
 const float f=(float)(r<0?0:r>1?1:r);
 assert(fbits(sj6_lfo_port(b,a,i))==fbits(f));
}
int main(void){
 unsigned count=0;
 for(unsigned b=0;b<128;b++)for(unsigned a=0;a<128;a++)for(int x=-64;x<=64;x++){same((float)b/127.f,(float)a/127.f,x);count++;}
 for(unsigned i=0;i<2000000;i++){float b=fl(rng()%0x3f800001u),a=fl(rng()%0x3f800001u);same(b,a,(int)(rng()%129)-64);count++;}
 printf("%u original double-addition outcomes matched bit-for-bit, all physical dial pairs and triangle samples\n",count);
}
