#define fm_tanhf wrapped_fast_tanh
#include "fastmath.h"
#undef fm_tanhf
float fm_tanhf(float x){return wrapped_fast_tanh(x);}
