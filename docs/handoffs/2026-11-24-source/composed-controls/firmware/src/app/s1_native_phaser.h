/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
enum { S1_PHASE_BYTE=48, S1_PHASE_MAX=127, S1_PHASE_COUNT=4, S1_PHASE_FIRST=116 };
_Static_assert(S1_PHASE_BYTE>=NPARAMS_MAX && S1_PHASE_BYTE+S1_PHASE_COUNT<=2u*NPARAMS_MAX,
               "Synth One Phaser belongs in the second typed owner row");
enum { S1_ENV_LFO_BYTE=52, S1_ENV_LFO_MAX=3, S1_ENV_LFO_PARAM=56 };
_Static_assert(S1_ENV_LFO_BYTE==S1_PHASE_BYTE+S1_PHASE_COUNT &&
               S1_ENV_LFO_BYTE<2u*NPARAMS_MAX, "Envelope LFO has a distinct owner byte");
static inline const uint8_t *s1_env_lfo_read(const sound_t *s,unsigned owner)
{
    return &s->v[T_303][8u+2u*owner+S1_ENV_LFO_BYTE/NPARAMS_MAX][S1_ENV_LFO_BYTE%NPARAMS_MAX];
}
static inline uint8_t *s1_env_lfo_write(sound_t *s,unsigned owner)
{
    return &s->v[T_303][8u+2u*owner+S1_ENV_LFO_BYTE/NPARAMS_MAX][S1_ENV_LFO_BYTE%NPARAMS_MAX];
}
enum { S1_DELAY_RES_BYTE=62, S1_DELAY_RES_MAX=127, S1_DELAY_RES_PARAM=138 };
_Static_assert(S1_DELAY_RES_BYTE<2u*NPARAMS_MAX, "Delay input resonance owner byte");
static inline const uint8_t *s1_delay_res_read(const sound_t *s,unsigned owner)
{
    return &s->v[T_303][8u+2u*owner+S1_DELAY_RES_BYTE/NPARAMS_MAX][S1_DELAY_RES_BYTE%NPARAMS_MAX];
}
static inline uint8_t *s1_delay_res_write(sound_t *s,unsigned owner)
{
    return &s->v[T_303][8u+2u*owner+S1_DELAY_RES_BYTE/NPARAMS_MAX][S1_DELAY_RES_BYTE%NPARAMS_MAX];
}
static inline unsigned s1_phase_init(unsigned k)
{
    static const uint8_t p[S1_PHASE_COUNT]={0,5,0,99};
    return p[k];
}
static inline unsigned s1_phase_encode(unsigned k,unsigned pos)
{
    const unsigned init=s1_phase_init(k);
    return pos==init ? 0u : pos+(pos<init);
}
static inline unsigned s1_phase_position(unsigned k,unsigned code)
{
    const unsigned init=s1_phase_init(k);
    return code ? code-(code<=init) : init;
}
static inline uint8_t *s1_phase_write(sound_t *s,unsigned owner)
{
    return &s->v[T_303][8u+2u*owner+S1_PHASE_BYTE/NPARAMS_MAX][S1_PHASE_BYTE%NPARAMS_MAX];
}
static inline const uint8_t *s1_phase_read(const sound_t *s,unsigned owner)
{
    return &s->v[T_303][8u+2u*owner+S1_PHASE_BYTE/NPARAMS_MAX][S1_PHASE_BYTE%NPARAMS_MAX];
}
static inline float s1_phase_value(unsigned k,unsigned code)
{
    /* Original kernel units: wet ratio, cycles/min, feedback fraction, sweep width Hz. */
    static const float lo[S1_PHASE_COUNT]={0.f,1.f,0.f,100.f};
    static const float span[S1_PHASE_COUNT]={1.f/127.f,299.f/127.f,.8f/127.f,900.f/127.f};
    static const float defaults[S1_PHASE_COUNT]={0.f,12.f,0.f,800.f};
    return code ? lo[k]+span[k]*(float)s1_phase_position(k,code) : defaults[k];
}
