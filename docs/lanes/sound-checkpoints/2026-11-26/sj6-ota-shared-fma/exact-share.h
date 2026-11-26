#include <stdint.h>
#include <math.h>
#include "fused_reference.h"
typedef union {float f;uint32_t u;} exact_bits;
#ifdef TEST_CALLS
static unsigned exact_compact,exact_fallback,exact_bad;
__attribute__((visibility("default"))) unsigned compact_calls(void){return exact_compact;}
__attribute__((visibility("default"))) unsigned fallback_calls(void){return exact_fallback;}
__attribute__((visibility("default"))) unsigned bad_calls(void){return exact_bad;}
#define UPDATE_EXACT_COUNTER(id) (++exact_##id)
#else
#define UPDATE_EXACT_COUNTER(id) ((void)0)
#endif
static float paired_fma(float a,float b,float c){
 exact_bits A={.f=a},B={.f=b},C={.f=c},D;
 if(original_fma_compact_bits(A.u,B.u,C.u,&D.u)){UPDATE_EXACT_COUNTER(compact);return D.f;}
 if(original_fma_finite_bits_reference(A.u,B.u,C.u,&D.u)){UPDATE_EXACT_COUNTER(fallback);return D.f;}
 UPDATE_EXACT_COUNTER(bad);
 D.u=0x7fc00000u; return D.f;
}
#define fmaf paired_fma
