#include "fastmath.h"
/* Finite positive sample rates/control values supplied by real typed caller.
 * Separate scope: feedback=0, unchanged conditioner, full low-four selector. */
float main_settle_g(float hz, float fs) {
 float top=fm_clampf(fs*.9f,0.f,30000.f);
 hz=fm_clampf(hz,3.f,top);
#ifdef LOG_ROUNDTRIP
 hz=fm_exp2f(fm_log2f(hz));
#endif
 return fm_tanf((hz*FM_PI)/(fs+fs));
}
