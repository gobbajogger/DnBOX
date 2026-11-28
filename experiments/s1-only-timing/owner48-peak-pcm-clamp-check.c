#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define X0X_COLD
#include "master.c"
static uint32_t bits(float f){uint32_t u;memcpy(&u,&f,4);return u;}
static float flt(uint32_t u){float f;memcpy(&f,&u,4);return f;}
int main(void){
 unsigned n=0;
 for(uint32_t u=bits(0.89f);u<=bits(2.0f);++u){
   float a=flt(u);
   for(int s=0;s<2;++s){
     const float f = soft_ceiling(s?-a:a);
     assert(f>=-1.f && f<=1.f);
     for(int loud=0;loud<2;++loud){
       const float lim=loud?1.f:2.f, k=loud?8388607.f:4194303.f;
       const float old=f>lim?lim:f< -lim?-lim:f;
       assert((int32_t)(old*k)==(int32_t)(f*k));
     }
   }
   ++n;
 }
 assert(soft_ceiling(2.0f)==1.f && soft_ceiling(-2.0f)==-1.f);
 printf("%u knee magnitudes exhaustive (+/-, NORMAL/LOUD)\n", n);
}
