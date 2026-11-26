#ifndef DNBOX_SJ6_J60V2_PHASE_CLOCK_H
#define DNBOX_SJ6_J60V2_PHASE_CLOCK_H
#include <stdint.h>

/* Host diagnostic: advance the original rational converter phase with no
 * division, modulo, or growing absolute frame counter per output sample. */
typedef struct {
    uint32_t input_index;
    uint16_t residual, denominator;
    uint16_t input_step, residual_step;
    uint16_t phase, subphase;
    uint16_t phase_step, subphase_step;
} sj6_phase_clock;

static inline sj6_phase_clock sj6_phase_init(uint16_t numerator, uint16_t denominator)
{
    const uint16_t ds = numerator % denominator;
    return (sj6_phase_clock){
        .denominator = denominator, .input_step = numerator / denominator,
        .residual_step = ds, .phase_step = ((uint32_t)ds << 8) / denominator,
        .subphase_step = ((uint32_t)ds << 8) % denominator
    };
}

static inline void sj6_phase_next(sj6_phase_clock *c)
{
    uint16_t frac = c->subphase + c->subphase_step;
    uint16_t phase = c->phase + c->phase_step;
    if (frac >= c->denominator) {
        frac -= c->denominator;
        ++phase;
    }
    const uint16_t carry = phase >> 8;
    c->input_index += c->input_step + carry;
    c->phase = phase & 255;
    c->subphase = frac;
    c->residual += c->residual_step;
    if (carry) c->residual -= c->denominator;
}
#endif
