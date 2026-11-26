#ifndef SJ6_CRUSH_BASE_H
#define SJ6_CRUSH_BASE_H
#include "fastmath.h"
#include "unit_control.h"
static inline uint32_t sj6_base108(unsigned q) {
    const float x=unit_control(q);
    fm_bits_t b;
    b.f=(100.f*x)*fm_sqrtf(x);
    return b.u;
}
static inline uint32_t sj6_base109(unsigned q) {
    const float x=unit_control(q);
    fm_bits_t b;
    b.f=6.06f*fm_powf(x,1.23f);
    return b.u;
}
#endif
