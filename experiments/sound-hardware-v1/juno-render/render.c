/* SPDX-License-Identifier: GPL-3.0-only */
/* Boot a single shipping Juno renderer without the rest of the instrument. */
#include <stddef.h>
#include <stdint.h>
#include "juno6.h"

void *memset(void *dest, int value, size_t bytes)
{
    unsigned char *p = dest;
    while (bytes--) *p++ = (unsigned char)value;
    return dest;
}

void *memcpy(void *dest, const void *src, size_t bytes)
{
    unsigned char *p = dest;
    const unsigned char *q = src;
    while (bytes--) *p++ = *q++;
    return dest;
}

static juno6_t synth;
static juno6_chorus_t chorus;
static float left[128], right[128];

void bench(void)
{
    juno6_init(&synth, 44100);
    juno6_chorus_init(&chorus, 44100);
    for (int i = 0; i < VOICES; ++i) juno6_note_on(&synth, 32 + 7 * i, 99);
    for (int block = 0; block < 10; ++block) {
        juno6_render(&synth, left, 128);
        memcpy(right, left, sizeof left);
        juno6_chorus_process(&chorus, left, right, 128);
    }
    uint32_t hash = 0;
    for (int i = 0; i < 128; ++i) {
        union {uint32_t word; float value;} l = {.value = left[i]}, r = {.value = right[i]};
        hash = hash * 16777619 ^ l.word;
        hash = hash * 16777619 ^ r.word;
    }
    *(volatile uint32_t *)0x1c07f00 = hash;
}
