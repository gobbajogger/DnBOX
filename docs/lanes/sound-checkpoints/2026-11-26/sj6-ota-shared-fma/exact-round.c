#include <stdio.h>
#include <assert.h>
#include "exact-share.h"
#undef fmaf
static uint32_t r=UINT32_C(0x78612aac);
static uint32_t rnd(void){r^=r<<13;r^=r>>17;r^=r<<5;return r;}
static void cmp(uint32_t a,uint32_t b,uint32_t c) {
 exact_bits A={.u=a}, B={.u=b}, C={.u=c};
 exact_bits h={.f=fmaf(A.f,B.f,C.f)};
 exact_bits m={.f=paired_fma(A.f,B.f,C.f)};
 assert(h.u==m.u);
}
int main(void){
 static const uint32_t e[]={0u,0x80000000u,1,0x80000001u,0x7fffffu,0x807fffffu,
 0x00800000u,0x80800000u,0x3f000000u,0xbf000000u,0x3f800000u,0xbf800000u,
 0x3f800001u,0x3f7fffffu,0x7f7fffffu,0xff7fffffu,0x01000000u,0x81000000u};
 unsigned n=sizeof(e)/sizeof(*e);
 for(unsigned i=0;i<n;i++)for(unsigned j=0;j<n;j++)for(unsigned k=0;k<n;k++)cmp(e[i],e[j],e[k]);
 for(unsigned i=0;i<100000;i++){
   unsigned u=rnd(),v=rnd(),w=rnd();
   if((u&0x7f800000)==0x7f800000)u^=0x40000000;
   if((v&0x7f800000)==0x7f800000)v^=0x40000000;
   if((w&0x7f800000)==0x7f800000)w^=0x40000000;
   cmp(u,v,w);
 }
 printf("equal_words=%u compact=%u reference=%u rejected=%u\n",n*n*n+100000,compact_calls(),fallback_calls(),bad_calls());
}
