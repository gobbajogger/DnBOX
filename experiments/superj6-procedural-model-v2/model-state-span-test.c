/* Synthetic render-state overlap regression; no original payload. */
#define main existing_protocol_suite
#include "model-unit-test.c"
#undef main
#include <stddef.h>
int main(void) {
 union { max_align_t align; unsigned char bytes[530]; } input;
 unsigned char saved[530];
 Sj6MathModel m, before;
 Sj6MathState state;
 int16_t table[512], old_table[512];
 float expected, actual;
 synthetic(input.bytes, sizeof input.bytes);
 expect("open synthetic model", sj6_math_open(&m,input.bytes,sizeof input.bytes)==0);
 expect("build synthetic cycle", sj6_math_build(&m,table,512)==0);
 expected=sj6_math_at(&m,0,table,0,0);
 sj6_math_reset(&state);
 expect("valid state preserves sample",sj6_math_at(&m,&state,table,0,0)==expected);
 memcpy(saved,input.bytes,sizeof saved);
 actual=sj6_math_at(&m,(Sj6MathState*)input.bytes,table,0,0);
 expect("input-overlap rejects",actual==0.f);
 expect("input-overlap does not mutate",memcmp(saved,input.bytes,sizeof saved)==0);
 memcpy(input.bytes,saved,sizeof saved);
 before=m;
 actual=sj6_math_at(&m,(Sj6MathState*)&m.frames,table,0,0);
 expect("view-overlap rejects",actual==0.f);
 expect("view-overlap does not mutate",memcmp(&m,&before,sizeof m)==0);
 m=before;
 memcpy(old_table,table,sizeof table);
 actual=sj6_math_at(&m,(Sj6MathState*)table,table,0,0);
 expect("cycle-overlap rejects",actual==0.f);
 expect("cycle-overlap does not mutate",memcmp(old_table,table,sizeof table)==0);
 memcpy(table,old_table,sizeof table);
 { unsigned i;
   for(i=0;i<sizeof input.bytes;i+=2){
     actual=sj6_math_at(&m,(Sj6MathState*)(input.bytes+i),table,0,0);
     expect("each aligned borrowed-input overlap rejects",actual==0.f);
     expect("each borrowed-input overlap preserves bytes",memcmp(saved,input.bytes,sizeof saved)==0);
   }
   for(i=0;i<sizeof m;i+=2){
     actual=sj6_math_at(&m,(Sj6MathState*)((unsigned char*)&m+i),table,0,0);
     expect("each aligned view overlap rejects",actual==0.f);
     expect("each view overlap preserves bytes",memcmp(&m,&before,sizeof m)==0);
   }
   for(i=0;i<512;i++){
     actual=sj6_math_at(&m,(Sj6MathState*)(table+i),table,0,0);
     expect("each cycle overlap rejects",actual==0.f);
     expect("each cycle overlap preserves bytes",memcmp(old_table,table,sizeof table)==0);
   }
 }
 expect("misaligned state rejects",sj6_math_at(&m,(Sj6MathState*)(input.bytes+1),table,0,0)==0.f);
 expect("wrapping state rejects",sj6_math_at(&m,(Sj6MathState*)(UINTPTR_MAX-1),table,0,0)==0.f);
 expect("wrapping cycle rejects",sj6_math_at(&m,&state,(int16_t*)(UINTPTR_MAX-1),0,0)==0.f);
 expect("misaligned cycle rejects",sj6_math_at(&m,&state,(int16_t*)(input.bytes+1),0,0)==0.f);
 expect("endpoint never touches invalid state",sj6_math_at(&m,(Sj6MathState*)UINTPTR_MAX,table,m.frames,0)==0.f);
 { struct {int16_t cycle[512];Sj6MathState adjacent;} x;
   memcpy(x.cycle,old_table,sizeof x.cycle);sj6_math_reset(&x.adjacent);
   expect("state adjacent to cycle accepted",sj6_math_at(&m,&x.adjacent,x.cycle,0,0)==expected);
 }
 { struct {Sj6MathModel view;Sj6MathState adjacent;} x;
   x.view=m;sj6_math_reset(&x.adjacent);
   expect("state adjacent to view accepted",sj6_math_at(&x.view,&x.adjacent,table,0,0)==expected);
 }
 printf("%u render-state span checks, %u failed\n",checks,failed);
 return !!failed;
}
