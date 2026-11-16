/* Synthetic admission spans only. No recorded/model payload. */
#define main retained_synthetic_main
#include "model-unit-test.c"
#undef main
#include <stddef.h>
int main(void){
 union {max_align_t alignment;unsigned char bytes[700];} storage;
 unsigned char before[700];Sj6MathModel m,old;
 synthetic(storage.bytes,530);memcpy(before,storage.bytes,sizeof before);
 expect("output inside model harmonic payload rejected",sj6_math_open((Sj6MathModel*)(storage.bytes+80),storage.bytes,530)!=0);
 expect("alias rejection preserves all model bytes",memcmp(storage.bytes,before,sizeof before)==0);
 synthetic(storage.bytes,530);memcpy(before,storage.bytes,sizeof before);
 expect("output inside header rejected",sj6_math_open((Sj6MathModel*)storage.bytes,storage.bytes,530)!=0);
 expect("header alias rejection preserves all bytes",memcmp(storage.bytes,before,sizeof before)==0);
 synthetic(storage.bytes,530);memset(&m,0xa5,sizeof m);old=m;
 expect("misaligned output rejected",sj6_math_open((Sj6MathModel*)(storage.bytes+1),storage.bytes,530)!=0);
 expect("wrapping output span rejected",sj6_math_open((Sj6MathModel*)(UINTPTR_MAX-sizeof(Sj6MathModel)+2),storage.bytes,530)!=0);
 expect("wrapping input span rejected",sj6_math_open(&m,(const void*)(UINTPTR_MAX-100),530)!=0);
 expect("wrapping input preserves output",memcmp(&m,&old,sizeof m)==0);
 expect("ordinary disjoint model still opens",sj6_math_open(&m,storage.bytes,530)==0);
 printf("%u synthetic span checks, %u failed\n",checks,failed);return!!failed;
}
