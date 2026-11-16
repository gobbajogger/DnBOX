/* SPDX-License-Identifier: GPL-3.0-only */
/* Parametric resynthesis of a recorded zone. No sample memory.
 *
 * Each record is a handful of partials, an attack/decay envelope, a slow
 * sustain drift and a noise floor. The private 12-record SuperJ6 set is not
 * in this tree; the tables that are linked are labelled public stand-ins.
 * Analytic saw/triangle/sine/pulse/noise oscillators are a different path
 * and are not replaced here.
 *
 * Integer phase and a shared sine table. No libm, no heap, no static locals.
 */
#ifndef SJ6_RESYNTH_H
#define SJ6_RESYNTH_H
#include <stdint.h>

enum { SJ6_PARTIALS = 8, SJ6_SINE_N = 256 };

typedef struct Sj6Voice {
    char name[20];
    uint16_t attack;          /* samples, at least 1 */
    uint16_t decay;
    uint16_t drift_len;       /* 0 = no drift */
    int16_t drift_depth;      /* Q15, added around unity */
    int16_t noise_peak;
    int16_t noise_sustain;
    uint32_t noise_seed;
    uint32_t inc[SJ6_PARTIALS];
    int16_t peak[SJ6_PARTIALS];
    int16_t sustain[SJ6_PARTIALS];
} Sj6Voice;

typedef struct Sj6State {
    uint32_t phase[SJ6_PARTIALS];
    uint32_t drift;
    uint32_t noise;
} Sj6State;

typedef struct Sj6Cost {
    uint64_t muls, adds, shifts, branches;
} Sj6Cost;

void sj6_state_init(Sj6State *s, uint32_t noise_seed);
/* Writes n samples. Full scale is 32767. Deterministic for a given voice and state. */
void sj6_render(const Sj6Voice *v, Sj6State *s, int32_t *out, int n, Sj6Cost *cost);
/* Referenced by the opt-in firmware link so the voice table is not dropped. */
void sj6_resynth_keep(void);
unsigned sj6_standin_count(void);
const Sj6Voice *sj6_standin(unsigned index);

#endif
