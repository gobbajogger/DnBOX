#include "firmware/src/app/dnr2_scalar.h"
/* Force each wire path through LTO without a test adapter or chosen catalog
 * being folded to a constant. Size is a bound for this implementation only. */
static int accept(uint8_t value) { return value != 0; }
int dnr2_scalar_native_fit(dnr2_scalar *state, const uint8_t *input, uint32_t length,
                           uint32_t now, uint8_t value, uint8_t *reply)
{
    return dnr2_scalar_frame(state, input, length, now, value, accept, reply);
}
