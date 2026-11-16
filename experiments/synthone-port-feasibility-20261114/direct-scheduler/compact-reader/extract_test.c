// SPDX-License-Identifier: GPL-3.0-only
// Test-only fixture builder. Production changes caught: byte-window truncation,
// overread, width0/32 shifts, bit-order mistakes, predictor/sign corruption.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "compact.c"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static unsigned long checks;
static uint32_t rng=0x54318e79U;
static uint32_t next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static void put32(unsigned char*p,uint32_t w){for(unsigned k=0;k<4;k++)p[k]=(unsigned char)(w>>(8*k));}
int main(void){
 const size_t page=(size_t)sysconf(_SC_PAGESIZE);CHECK(page>=4096);
 unsigned char*map=mmap(0,page*3,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);CHECK(map!=MAP_FAILED);CHECK(!mprotect(map+page,page,PROT_READ|PROT_WRITE));
 for(unsigned bm=0;bm<5;bm++)if(bm!=2)for(unsigned width=0;width<=32;width++){
  if(bm==0&&width!=32)continue;
  const unsigned anchors=bm==0?0:bm==1?1:bm==3?2:3;
  for(unsigned run=0;run<128;run++)for(unsigned short_block=0;short_block<2;short_block++){
   const unsigned count=short_block?1:64,qn=short_block?2049:4096,first=short_block?32:0;
   const size_t data=1368+65*4,nb=2+4*anchors+(count*width+7)/8,total=data+nb;
   unsigned char*b=map+2*page-total;memset(b,0,total);
   put32(b+64,short_block?1:0);put32(b+72,qn);put32(b+80,(uint32_t)total);put32(b+1368+first*4,(uint32_t)data);put32(b+1368+(first+1)*4,(uint32_t)total);
   b[data]=(unsigned char)bm;b[data+1]=(unsigned char)width;
   uint32_t base=next(),step=next(),curve=next();
   if(anchors)put32(b+data+2,base);if(anchors>1)put32(b+data+6,step);if(anchors>2)put32(b+data+10,curve);
   uint32_t codes[64];unsigned char*payload=b+data+2+4*anchors;
   for(unsigned i=0;i<count;i++){
    uint32_t code=next();if(width<32)code&=(UINT32_C(1)<<width)-1U;codes[i]=code;
    for(unsigned k=0;k<width;k++)if((code>>k)&1U)payload[(i*width+k)/8]|=(unsigned char)(1U<<((i*width+k)%8));
   }
   view v={b,total,64,(uint32_t)data,(uint32_t)total};
   for(unsigned i=0;i<count;i++){
    uint32_t got=0xdeadbeefU;const unsigned sample=short_block?2048:i;
    CHECK(physical_word(&v,0,sample,&got));
    uint32_t want;
    if(bm==0)want=codes[i];
    else if(bm==1)want=(uint32_t)((uint64_t)base+codes[i]);
    else {int64_t delta=(codes[i]&1U)?-((int64_t)(codes[i]>>1)+1):(int64_t)(codes[i]>>1);uint64_t pred=(uint64_t)base+(uint64_t)step*i;if(bm==4)pred+=(uint64_t)curve*((uint64_t)i*(i?i-1:0)/2);want=(uint32_t)(pred+(uint64_t)delta);}
    if(got!=want){fprintf(stderr,"mode=%u width=%u run=%u short=%u i=%u got=%08x want=%08x\n",bm,width,run,short_block,i,got,want);return 1;}++checks;
   }
  }
 }
 CHECK(!munmap(map,page*3));printf("{\"checks\":%lu,\"widths\":33,\"short_and_full_blocks\":true,\"guard_page\":true}\n",checks);
}
