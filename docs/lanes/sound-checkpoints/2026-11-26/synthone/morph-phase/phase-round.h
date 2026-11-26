/* Round default IEEE-754 binary32 to nearest integer, ties to even, modulo
 * the 24-bit oscillator phase. Finite audio inputs; no libm dependency.
 * Integer bit extraction is unaffected by the configured FPU rounding mode.
 */
#include <stdint.h>
static inline uint32_t morph_round_phase24(float x)
{
    union {float f; uint32_t u;} v = {.f=x};
    const unsigned e = (v.u >> 23) & 255u;
    const uint32_t f = v.u & 0x7fffffu;
    uint32_t y;
    if (e < 126u) return 0;
    if (e == 126u) y = (f != 0);
    else if (e >= 150u) {
        if (e >= 174u) return 0; /* including NaN/Inf: not valid audio inputs */
        y = (0x800000u | f) << (e - 150u);
    } else {
        const unsigned k = 150u - e;
        const uint32_t sig = 0x800000u | f;
        const uint32_t half = 1u << (k - 1u);
        const uint32_t residue = sig & ((half << 1u) - 1u);
        y = sig >> k;
        y += (residue > half) || ((residue == half) && (y & 1u));
    }
    return ((v.u >> 31) ? (0u - y) : y) & 0xffffffu;
}
