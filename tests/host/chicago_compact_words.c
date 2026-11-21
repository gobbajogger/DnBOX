/* SPDX-License-Identifier: GPL-3.0-only */
#include "chicago_packed.h"
#include "chicago_packed_private.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    for (unsigned i=0; i<16385; ++i) {
        uint32_t a, e, wa, we;
        float x, y;
        assert(original_lossless_second16_lookup(&original_chicago_packed_a,i,&a));
        assert(original_lossless_second16_lookup(&original_chicago_packed_e,i,&e));
        assert(original_chicago_packed_lookup(NULL,i,&x,&y));
        memcpy(&wa,&x,sizeof wa);
        memcpy(&we,&y,sizeof we);
        assert(wa==a && we==e);
    }
    float x,y;
    assert(!original_chicago_packed_lookup(NULL,16385,&x,&y));
    puts("32770/32770 Chicago source words exact, including every directory boundary.");
    return 0;
}
