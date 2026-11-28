#include "crc32_nibble.h"
#include <assert.h>
#include <stdio.h>
#include <zlib.h>
static uint32_t rng=1234;
static uint32_t next(void){return rng=rng*1664525u+1013904223u;}
int main(void){
  static uint8_t data[4096];
  for(unsigned i=0;i<sizeof data;i++)data[i]=(uint8_t)(next()>>24);
  assert(x0x_crc32(NULL,0)==0);
  for(unsigned k=0;k<24000;k++){
    unsigned off=(next()>>19)%sizeof data;
    unsigned n=(next()>>19)%(sizeof data-off+1u);
    unsigned split=(next()>>19)%(n+1u);
    uint32_t x=x0x_crc32(data+off,n);
    assert(x==(uint32_t)crc32(0,data+off,n));
    assert(x==~x0x_crc32_upd(x0x_crc32_upd(0xffffffffu,data+off,split),data+off+split,n-split));
  }
  puts("shared and standalone CRC: 24,000 zlib matches and split-stream matches");
}
