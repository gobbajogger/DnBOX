#include <assert.h>
#include <stdint.h>
#include <stdio.h>
float s1u_sin(uint32_t);
static void check(uint32_t p) {
  assert((s1u_sin(p) > 0.f) == (p != 0u && p < UINT32_C(0x80000000)));
}
int main(void) {
 uint32_t p = 1;
 for (uint32_t i=0;i<256;i++) {
   uint32_t h=i<<24;
   check(h);check(h|1);check(h|2);check(h|0x7fffff);check(h|0xfffffe);check(h|0xffffff);
 }
 for(unsigned i=0;i<1000000;i++){p=p*1664525u+1013904223u;check(p);}
 puts("1,001,536 phase signs matched.");
}
