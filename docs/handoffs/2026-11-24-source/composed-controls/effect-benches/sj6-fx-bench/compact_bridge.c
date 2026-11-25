/* SPDX-License-Identifier: GPL-3.0-only
 * Isolated calls into the supplied pinned firmware kernels. No approximation
 * of firmware effects here. No production state, device, or flash access. */
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "dsp/fastmath.h"
#include "dsp/juno6.h"
#include "app/original_runtime.h"
#include "dsp/juno6_j60_native.inc"
#include "dsp/juno6_delay_native.inc"
#include "dsp/sj6_native_room.inc"

enum { RATE = 44100, CONFIG_SETTLE = 10240 };
enum { FX_DELAY, FX_J60_I, FX_J60_II, FX_J60_BOTH, FX_ROOM_I, FX_ROOM_II, FX_WIDEN };

int sj6_fx_render(unsigned effect, unsigned enabled, float *planes, unsigned frames)
{
    if (!planes || enabled > 1u || !frames) return -1;
    float *l=planes, *r=planes+frames;
    if (effect==FX_DELAY) {
        juno6_t s={0};
        int16_t ring[SJ6_NATIVE_DELAY_CAP*2u]={0};
        uint8_t code[4]={2u|enabled,45u,64u,127u}; /* .125 whole-note at 120 BPM: 250 ms */
        s.typed=1;
        s.sample_rate=(float)RATE;
        juno6_delay_native_arm(&s,ring);
        juno6_delay_native_config(&s,code,120.f);
        if (s.delay24.clock_refused) return -2;
        /* Integer control warm-up matches the settled original oracle, but
         * does not overwrite or tune any compact DSP state. */
        for (unsigned n=0;n<CONFIG_SETTLE;n+=64u) {
            float z[2][64]={{0}};
            juno6_delay24_process(&s,z[0],z[1],64u);
        }
        juno6_delay24_process(&s,l,r,frames);
        return 0;
    }
    if (effect>=FX_J60_I && effect<=FX_J60_BOTH) {
        juno6_chorus_t c={0};
        c.sample_rate=(float)RATE;
        c.factory_mode=(uint8_t)(enabled ? JUNO6_CHORUS_OFF+effect : JUNO6_CHORUS_OFF);
        const uint8_t code[4]={1,0,0,0}; /* original unity rate/depth and mix */
        juno6_j60_config(&c,code);
        juno6_j60_process(&c,l,r,frames);
        return 0;
    }
    if (effect==FX_ROOM_I || effect==FX_ROOM_II || effect==FX_WIDEN) {
        original_runtime_part p={0};
        p.kind=ORIGINAL_SOUND_JUNO;
        p.status=ORIGINAL_RUNTIME_OK;
        const uint8_t code[SJ6_ROOM_COUNT]={
            [SJ6_ROOM_ENABLE]=(uint8_t)(enabled && effect!=FX_WIDEN),
            [SJ6_ROOM_TYPE]=(uint8_t)(effect==FX_ROOM_II),
            [SJ6_ROOM_SIZE]=100,[SJ6_ROOM_LOCUT]=40,[SJ6_ROOM_MIX]=127,
            [SJ6_ROOM_WIDEN]=(uint8_t)(enabled && effect==FX_WIDEN)
        };
        original_runtime_sj6_room(&p,code);
        sj6_room_process(&p.dsp.juno.room,l,r,frames);
        return 0;
    }
    return -3;
}
