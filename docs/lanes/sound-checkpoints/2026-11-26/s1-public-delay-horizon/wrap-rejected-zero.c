#include "baseline-delay.h"
#include "exact-delay.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
static uint32_t prng=0x120d3829u;
static float rand11(void){
 prng^=prng<<13;prng^=prng>>17;prng^=prng<<5;return (float)(int32_t)prng/2147483648.f;
}
static void test(unsigned fs){
 dnbox_s1_exact_delay *a=dnbox_s1_delay_new(fs);source_s1_exact_delay*b=source_s1_delay_new(fs);
 unsigned diff=0;float max=0.f;
 for(unsigned j=0;j<40*fs;j++){
  float w=(float)j/(float)fs;
  float t=w<12.f?2.5f:w<15.f?(float)(w-12.f)*.8f:w<23.f?2.4999998f:w<26.f?0.f:w<29.f?1.f/16.f:1.24f+rand11()*1.21f;
  assert(t>=0&&t<=2.5f);
  float feedback=w<12.f?.667f:w<20.f?.982f:w<29.f?.25f:.7f;
  float mix=w<5.f?0.f:w<12.f?.83f:w<28.f?.48f:1.f;
  float E=w<13.f?0.f:w<14.f?w-13.f:w<27.f?1.f:w<28.f?28.f-w:0.f;
  float l=rand11()*.0625f+(j%(fs/30u)==0?.125f:0.f),r=rand11()*.03125f+(j%(fs/41u)==0?-.25f:0.f);
  if(j==9*fs+11||j==24*fs+17){dnbox_s1_delay_reset(a); source_s1_delay_reset(b);}
  float al,ar,bl,br;
  dnbox_s1_delay_insert(a,t,feedback,((j/4096u)%3u)?0.f:1.f,7200.f,0.4f,mix,E,l,r,&al,&ar);
  source_s1_delay_insert(b,t,feedback,((j/4096u)%3u)?0.f:1.f,7200.f,0.4f,mix,E,l,r,&bl,&br);
  assert(isfinite(al)&&isfinite(ar)&&isfinite(bl)&&isfinite(br));
  if((memcmp(&al,&bl,4)||memcmp(&ar,&br,4)) &&diff<10)fprintf(stderr,"first Fs=%u frame=%u t=%.9g L %a %a R %a %a E %.7g\n",fs,j,t,al,bl,ar,br,E);
  diff+=(memcmp(&al,&bl,4)!=0)+(memcmp(&ar,&br,4)!=0);max=fmaxf(max,fmaxf(fabsf(al-bl),fabsf(ar-br)));
 }
 printf("Fs=%u frames=%u old B=%zu cap B=%zu diff=%u max=%g\n",fs,40*fs,source_s1_delay_history_bytes(b),dnbox_s1_delay_history_bytes(a),diff,max);
 assert(!diff);
 dnbox_s1_delay_free(a);source_s1_delay_free(b);
}
int main(void){test(44100);test(48000);return 0;}
