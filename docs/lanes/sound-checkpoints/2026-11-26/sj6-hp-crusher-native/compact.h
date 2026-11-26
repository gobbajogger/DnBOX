#ifndef DNBOX_SJ6_44K_ZERO_LEAF
#define DNBOX_SJ6_44K_ZERO_LEAF
/* Active Apple-style HP 10 Hz/0dB plus amount-zero distortion DC response,
 * scoped to the verified 44.1 kHz profile. Caller owns a separate initialized
 * state for each live audio channel; unrelated controls must not select it. */
typedef struct {
    float hp_x, hp_dx, hp_y, hp_dy, crusher_x, crusher_y;
} sj6_zero_fx_channel;
static inline float sj6_hp10_44100(sj6_zero_fx_channel *s, float x)
{
    const float dx=x-s->hp_x;
    const float dy=0x1.ffa2ap-1f*(dx-s->hp_dx)-0x1.10427p-19f*s->hp_y+0x1.ff4564p-1f*s->hp_dy;
    s->hp_y+=dy;
    s->hp_x=x;
    s->hp_dx=dx;
    s->hp_dy=dy;
    return s->hp_y;
}
static inline float sj6_zero_crusher_44100(sj6_zero_fx_channel *s,float x)
{
    const float y=(x-s->crusher_x)+0x1.fd70a4p-1f*s->crusher_y;
    s->crusher_x=x;
    s->crusher_y=y;
    return y;
}
#endif
