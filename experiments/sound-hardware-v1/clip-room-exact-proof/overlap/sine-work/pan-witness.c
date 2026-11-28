#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
typedef void (*panfn)(float*,float,float,float,float,float*,float*);
void s1u_autopan(float*,float,float,float,float,float*,float*);
void new_s1u_autopan(float*,float,float,float,float,float*,float*);
void s1u_autopan_authored(float*,float,float,float,float,float*,float*);
void new_s1u_autopan_authored(float*,float,float,float,float,float*,float*);
static uint32_t rng=287409u;
static uint32_t next(void){return rng=rng*1664525u+1013904223u;}
int main(void){
 const float rates[]={44100,48000,32000,22050};
 uint64_t n=0;
 for (int j=0;j<1;++j){
   panfn a=s1u_autopan,b=new_s1u_autopan;
   for (int k=0;k<128;++k){
    float p[3]={(float)(next()&0xffffffu)*0x1p-24f,0,0},q[3]={p[0],0,0};
    for(int f=0;f<8192;++f){
      float sr=rates[k%4], v=(float)(next()&255u)*0.031f;
      float amount=(f/32)%5==0?(float)(next()&4095u)*0x1p-12f:((f&1)?-0.0f:0.0f);
      float y=(float)(int32_t)next()*0x1p-28f;
      if(f%128==0)y=copysignf(0.0f,y);
      a(&p[0],y,v,amount,sr,&p[1],&p[2]); b(&q[0],y,v,amount,sr,&q[1],&q[2]);
      assert(memcmp(p,q,sizeof p)==0);++n;
    }
   }
 }
 printf("PASS %llu phase and stereo comparisons, all Float32 bits\n",(unsigned long long)n);
}
