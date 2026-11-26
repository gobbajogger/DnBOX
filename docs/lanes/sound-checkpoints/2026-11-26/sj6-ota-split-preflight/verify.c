#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
extern float sj6_ota_stage(float,float*,float), fallback_shared(float,float*,float);
static uint32_t seed=20;
static uint32_t rng(void){seed ^= seed<<13; seed ^= seed>>17; seed ^= seed<<5; return seed;}
int main(void){
 float a[4]={0,-0.f,0x1p-149f,-0x1p-149f},b[4]={0,-0.f,0x1p-149f,-0x1p-149f};
 for(unsigned i=0;i<65536;i++){
   float x=((int32_t)rng())*0x1p-26f,g=(rng()&0xffu)*(.3f/255.f),y=x;
   if(i<16){x=y=(i&2)?-0.f:0; if(i%4==0)a[0]=b[0]=x;}
   for(unsigned j=0;j<4;j++){x=sj6_ota_stage(x,a+j,g); y=fallback_shared(y,b+j,g);assert(!memcmp(&x,&y,4));}
   assert(!memcmp(a,b,sizeof a));
 }
 puts("262144 signed output words and 262144 state words bit-identical");
}
