#include "drum909.h"
volatile unsigned char sampler_state[sizeof(d9_smp_t)];
volatile unsigned char terminal_work[4*sizeof(d9_smp_t)+4*256*sizeof(float)+sizeof(uint32_t)];
