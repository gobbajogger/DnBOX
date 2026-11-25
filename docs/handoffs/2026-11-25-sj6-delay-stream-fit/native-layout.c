#include "dsp/juno6.h"
uint8_t baseline_state[sizeof(((juno6_t*)0)->delay24)];
uint8_t baseline_ring[2u*SJ6_NATIVE_DELAY_CAP*sizeof(int16_t)];
