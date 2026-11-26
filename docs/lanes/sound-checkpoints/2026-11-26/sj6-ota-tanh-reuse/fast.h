#include <math.h>
#if EXTERNAL_TANH
/* Prospective same existing body exported once; not current ELF linkage. */
extern float fm_tanhf(float);
#else
#include "fastmath.h"
#endif
#ifdef TEST_CALLS
static float max_tanh_argument;
__attribute__((visibility("default"))) float reached_tanh_peak(void){return max_tanh_argument;}
static float monitored_tanh(float x){if(fabsf(x)>max_tanh_argument)max_tanh_argument=fabsf(x);return fm_tanhf(x);}
#define tanhf monitored_tanh
#else
#define tanhf fm_tanhf
#endif
