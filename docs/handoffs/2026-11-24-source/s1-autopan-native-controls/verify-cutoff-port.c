/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6_cutoff_port.h"
#include <assert.h>
#include <stdio.h>
static uint32_t u(float f){union{float f;uint32_t u;}v={.f=f};return v.u;}
static float f(uint32_t u){union{float f;uint32_t u;}v={.u=u};return v.f;}
static uint32_t seed=0x710b8ae7u;
static uint32_t next(void){seed^=seed<<13; seed^=seed>>17; return seed^=seed<<5;}
static void check(float x,unsigned seg){
 static const uint16_t base[]={20,30,200,600,4100,14400,20000};
 uint32_t a=base[seg],b=base[seg+1]-a;
 assert(u(sj6_cutoff_port_round(x,a,b))==u((float)((double)a+(double)x*b)));
}
int main(void){
 unsigned count=0;
 for(unsigned s=0;s<6;s++){
   for(unsigned i=0;i<524288;i++){check(f(next()%0x3f800010u),s);count++;}
   for(uint32_t i=0x3f7fff00;i<0x3f800100;i++){check(f(i),s);count++;}
 }
 const float hi[]={.1f,.3f,.5f,.7f,.9f};
 for(unsigned i=0;i<=127;i++){
   float x=(float)i/127.f;unsigned s=0;
   while(s<5 && x>hi[s]) ++s;
   float n=(x-(s?hi[s-1]:0))/((s==0 || s==5)?.1f:.2f);
   check(n*n,s);count++;
 }
 printf("%u exact original rounded Build9 cutoff values, including all 128 SMK detents\n",count);
}
