/* Independent dispatch clock: preserve the original source quantum when an
 * internal compact engine subdivides one external render callback. */
#ifndef DNBX_SJ6_SOURCE_CALLBACK_CLOCK_H
#define DNBX_SJ6_SOURCE_CALLBACK_CLOCK_H
#include <stdint.h>
typedef struct { uint64_t first; unsigned span; } sj6_source_cb;
/* Returns the number of source AMP interpolation frames only on a physical
   source prep boundary, or zero otherwise. Never reads compact subblock size. */
static inline unsigned sj6_source_prep_frames(sj6_source_cb c,uint64_t frame){
    if(frame<c.first) return 0u;
    uint64_t offset=frame-c.first;
    if(offset>=c.span || offset%16u) return 0u;
    const unsigned rem=c.span-(unsigned)offset;
    return rem<16u?rem:16u;
}
#endif
