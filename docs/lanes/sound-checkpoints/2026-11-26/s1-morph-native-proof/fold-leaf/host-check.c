#include <stdint.h>
#include <stdio.h>
#include <string.h>
extern float tick(uint32_t*,uint32_t,float,int), ref_tick(uint32_t*,uint32_t,float,int);
extern uint32_t prepared(float),ref_prepared(float);
extern void pair(uint32_t[2],unsigned,unsigned),ref_pair(uint32_t[2],unsigned,unsigned);
int main(void) {
 for(unsigned r=0;r<52;r++)for(unsigned x=0;x<4096;x++){
  uint32_t u[2],v[2];pair(u,r,x);ref_pair(v,r,x);
  if(memcmp(u,v,sizeof u)){printf("pair %u %u\n",r,x);return 1;}
 }
 uint32_t x=0x86782ec1,p=0;
 for(unsigned i=0;i<262144;i++){
  x^=x<<13;x^=x>>17;x^=x<<5;
  uint32_t q=p;
  float hz=(float)(x>>16)*.4f,u=(float)(x&65535u)/50000.f;
  uint32_t a=prepared(hz),b=ref_prepared(hz);
  const int bandlimited=(i&1u)!=0;
  float y=tick(&p,a,u,bandlimited),z=ref_tick(&q,b,u,bandlimited);
  if(a!=b||p!=q||memcmp(&y,&z,sizeof y)){printf("tick %u\n",i);return 2;}
 }
 puts("PASS 212992 exhaustive folded pairs and 262144 tick PCM words/phase words");
}
