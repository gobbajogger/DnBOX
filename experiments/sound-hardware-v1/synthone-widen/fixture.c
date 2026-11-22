/* Adapt the bounded DnBOX history to one instance in the original test graph. */
#include <stdlib.h>
#include "soundpipe.h"
#include "widen.h"

static sp_delay *candidate;
void use_bounded_widen(sp_delay *p)
{
    /* Only used by this serial two-renderer comparison fixture. */
    if (!p || p->bufsize != 2205 || p->bufpos || p->feedback != 0) abort();
    size_t bytes = (TEST_HISTORY_BITS / 8) * S1_WIDEN_FRAMES;
    void *words = calloc(bytes, 1);
    if (!words) abort();
    free(p->buf.ptr);
    p->buf.ptr = words;
    p->buf.size = bytes;
    candidate = p;
}
int __real_sp_delay_compute(sp_data *, sp_delay *, SPFLOAT *, SPFLOAT *);
int __wrap_sp_delay_compute(sp_data *sp, sp_delay *p, SPFLOAT *input, SPFLOAT *out)
{
    if (p != candidate) return __real_sp_delay_compute(sp, p, input, out);
    unsigned pos = p->bufpos;
#if TEST_HISTORY_BITS == 16
    /* Original rejected two-byte experiment. Preserve sign and exponent. */
    uint16_t *history = p->buf.ptr;
    uint32_t input_bits, prior = (uint32_t)history[pos] << 16;
    memcpy(&input_bits, input, sizeof input_bits);
    history[pos] = (uint16_t)(input_bits >> 16);
    memcpy(out, &prior, sizeof prior);
    pos = pos + 1u == S1_WIDEN_FRAMES ? 0u : pos + 1u;
#elif TEST_HISTORY_BITS == 24
    uint8_t *storage = p->buf.ptr;
    *out = s1_widen_tap(storage, 1365, storage + 4095, &pos, *input);
#else
#error unsupported history width
#endif
    p->bufpos = pos;
    return SP_OK;
}
