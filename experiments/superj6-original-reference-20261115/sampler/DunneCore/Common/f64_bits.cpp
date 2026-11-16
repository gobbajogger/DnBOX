// One copy of the binary64 elementary functions, so each sampler
// translation unit does not carry its own exp, log, sine and fma.
#define F64_BITS_IMPLEMENTATION
#include "f64_bits.hpp"
