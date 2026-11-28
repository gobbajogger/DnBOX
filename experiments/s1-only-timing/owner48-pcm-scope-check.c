#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define N 256u
static uint32_t rng=0x0faf3edf;
static uint32_t rnd(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static float input(void){return (float)(int32_t)rnd()*(1.f/2147483648.f);}
int main(void){
 float l[N],r[N]; int32_t a[2*N],b[2*N]; int16_t sa[N],sb[N];
 for(unsigned k=0;k<100000;k++){
  uint32_t n=rnd()%257,w=rnd(),w2=w;
  const float z=k&1u?8388608.f:4194304.f,lim=k&1u?1.f:2.f;
  memset(a,0,sizeof a);memset(b,0,sizeof b);memset(sa,0,sizeof sa);memset(sb,0,sizeof sb);
  for(unsigned i=0;i<n;i++){
   l[i]=i%19?input():(k&4?1.f:-0.f);r[i]=i%23?input():(k&4?-1.f:0.f);
   float x=l[i]>lim?lim:l[i]<-lim?-lim:l[i];
   float y=r[i]>lim?lim:r[i]<-lim?-lim:r[i];
   a[2*i]=(int32_t)(x*z);a[2*i+1]=(int32_t)(y*z);
  }
  for(unsigned i=1;i<n;i+=2)sa[w++&(N-1)]=(int16_t)((l[i]+r[i])*16383.5f);
  unsigned i=0;
  for(;i+1u<n;i+=2u){
   float l0=l[i],r0=r[i],l1=l[i+1u],r1=r[i+1u];
   b[2*i]=(int32_t)(l0*z);b[2*i+1]=(int32_t)(r0*z);
   b[2*i+2]=(int32_t)(l1*z);b[2*i+3]=(int32_t)(r1*z);
   sb[w2++&(N-1)]=(int16_t)((l1+r1)*16383.5f);
  }
  if(i<n){b[2*i]=(int32_t)(l[i]*z);b[2*i+1]=(int32_t)(r[i]*z);}
  if(w!=w2||memcmp(a,b,sizeof a)||memcmp(sa,sb,sizeof sa)) return 1;
 }
 puts("100000 exact bounded stereo PCM + wrapped oscilloscope states");
}
