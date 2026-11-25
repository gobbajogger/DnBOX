/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
/* Build9 KoFMInstrument.lfoDestinations contains exactly 71 UInt8 entries.
 * Legacy carrier 1..6 remains decodable; newly authored selectors use
 * original index + 7, including indices whose receiver is not yet admitted. */
enum { SJ6_AUX_ORIGINAL_FIRST=7, SJ6_AUX_ORIGINAL_COUNT=71, SJ6_AUX_ORIGINAL_LAST=77 };
#if X0X_SJ6_AUX_SEQ_NATIVE
static inline int sj6_aux_seq_id(unsigned id)
{ return id==65u || id==66u || (id>=143u && id<=145u) || (id>=147u && id<=162u); }
#endif
static inline int sj6_aux_destination_id(unsigned route)
{
    static const uint8_t ids[]={
        0,78,79,129,1,3,57,
        207,7,36,66,65,23,22,108,37,26,27,25,132,133,135,136,134,131,35,124,207,
        125,74,72,1,126,75,73,3,89,88,97,96,101,100,0,201,39,8,143,144,202,38,
        103,104,60,61,14,15,13,145,147,148,149,150,151,152,153,154,155,156,
        157,158,159,160,161,162,207,17,78,79
    };
    return route && route<sizeof ids && ids[route]!=207u ? ids[route] : -1;
}
