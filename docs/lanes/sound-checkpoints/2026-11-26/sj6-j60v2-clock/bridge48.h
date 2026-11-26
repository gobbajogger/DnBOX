#ifndef DNBOX_SJ6_J60V2_BRIDGE48_H
#define DNBOX_SJ6_J60V2_BRIDGE48_H
#include <stdint.h>

typedef struct {
    float source[2][64], wet[2][64];
    uint8_t source_next, wet_next, started;
} sj6_bridge48;

/* Coefficients generated offline, then shared across owners. */
typedef struct {
    float up_half[32], down_odd[16], down_center;
} sj6_bridge48_coef;

typedef void (*sj6_bridge48_middle)(void *, float, float, float [2]);

static inline float sj6_bridge48_up_odd(const float a[64], int h,
                                        const sj6_bridge48_coef *c)
{
    float v = 0;
    for (int k = 0; k < 32; ++k)
        v += c->up_half[k] * (a[(h+k)&63] + a[(h+63-k)&63]);
    return v;
}

static inline float sj6_bridge48_down(const float a[64], int g,
                                      const sj6_bridge48_coef *c)
{
    float v = c->down_center * a[(g+31)&63];
    for (int k = 0; k < 16; ++k)
        v += c->down_odd[k] * (a[(g+30-2*k)&63] + a[(g+32+2*k)&63]);
    return v;
}

static inline void sj6_bridge48_publish(sj6_bridge48 *b, const float a[2])
{
    unsigned i = b->wet_next;
    b->wet[0][i] = a[0];
    b->wet[1][i] = a[1];
    b->wet_next = (i+1)&63;
}

/* One active host frame. Caller keeps the exact stop and Mix0 policies
 * outside this leaf and supplies unchanged live middle-kernel state. */
static inline void sj6_bridge48_tick(sj6_bridge48 *b, const sj6_bridge48_coef *c,
                                      sj6_bridge48_middle kernel, void *context,
                                      float l, float r, float out[2])
{
    float a[2];
    unsigned h = b->source_next;
    if (b->started) {
        kernel(context, sj6_bridge48_up_odd(b->source[0],h,c),
                        sj6_bridge48_up_odd(b->source[1],h,c), a);
        sj6_bridge48_publish(b,a);
    }
    b->started = 1;
    b->source[0][h] = l;
    b->source[1][h] = r;
    b->source_next = (h+1)&63;
    unsigned i = (h+32)&63;
    kernel(context,b->source[0][i],b->source[1][i],a);
    sj6_bridge48_publish(b,a);
    out[0] = sj6_bridge48_down(b->wet[0],b->wet_next,c);
    out[1] = sj6_bridge48_down(b->wet[1],b->wet_next,c);
}
#endif
