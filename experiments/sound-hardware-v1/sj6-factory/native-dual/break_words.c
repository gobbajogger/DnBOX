#include <stdint.h>
#include "break_retrigger_lut.h"
#ifdef HOST_CHECK
#include <stdio.h>
#else
#define OUTPUT ((volatile uint32_t *)0x01c10000)
#endif
__attribute__((noinline)) static uint32_t word(unsigned length, unsigned pot)
{
    union { float f; uint32_t u; } data = {brk_retrigger_probability(length, pot)};
    return data.u;
}
int main(void)
{
    unsigned count = 0;
    for (unsigned l = 0; l < 6; ++l) {
        for (unsigned p = 0; p < 257; ++p) {
            const uint32_t raw = word(l, p == 256 ? UINT32_MAX : p);
#ifdef HOST_CHECK
            fwrite(&raw, sizeof raw, 1, stdout);
#else
            OUTPUT[count + 1] = raw;
#endif
            ++count;
        }
    }
#ifndef HOST_CHECK
    OUTPUT[0] = count;
#endif
    return 0;
}
