#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
int chicago_raw_bits(uint32_t, uint32_t *, uint32_t *);
int compact_lookup(const void *,uint32_t,float *,float *);
int original_chicago_packed_lookup(const void *,uint32_t,float *,float *);
int main(void){
 unsigned n=0;
 for(unsigned pass=0;pass<3;pass++)for(uint32_t k=0;k<16385;k++){
  uint32_t i=(k*(pass==0?1:pass==1?1063:12289))%16385, raw[2];
  float b[2],c[2];
  assert(chicago_raw_bits(i,raw,raw+1));
  assert(compact_lookup(0,i,b,b+1));
  assert(memcmp(raw,b,8)==0);
  for(int j=0;j<2;j++){
   assert(original_chicago_packed_lookup(0,i,c,c+1));
   assert(memcmp(c,raw,8)==0);
   n+=2;
  }
 }
 uint32_t x=1; float a,e;
 assert(!original_chicago_packed_lookup(0,16385,&a,&e));
 assert(!original_chicago_packed_lookup(0,UINT32_MAX,&a,&e));
 assert(!original_chicago_packed_lookup(&x,0,&a,&e));
 assert(!original_chicago_packed_lookup(0,0,&a,&a));
 assert(!original_chicago_packed_lookup(0,0,0,&e));
 assert(!original_chicago_packed_lookup(0,0,&a,0));
 printf("PASS: %u FAST32 exact words against raw original and compact live original; cold/permuted + hits + invalid inputs\n",n);
}
