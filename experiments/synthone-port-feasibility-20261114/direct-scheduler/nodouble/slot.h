#pragma once
/* One .pool slot per Soundpipe constructor. Matches firmware/src/app/plat.h.
 * The compact engine embeds its DSP objects and does not call these
 * constructors. The slot removes malloc/free from the object file.
 * Measured host sizeof is in POOL-SIZES.txt next to this header. */
#ifndef X0X_POOL
#define X0X_POOL __attribute__((section(".pool")))
#endif
#define S1_POOLED(type, create_name, destroy_name, tag) \
    static type s1_pool_##tag X0X_POOL; \
    static unsigned char s1_pool_used_##tag; \
    int create_name(type **p) { \
        if (!p || s1_pool_used_##tag) return SP_NOT_OK; \
        s1_pool_used_##tag = 1; \
        *p = &s1_pool_##tag; \
        return SP_OK; \
    } \
    int destroy_name(type **p) { \
        if (p && *p == &s1_pool_##tag) { s1_pool_used_##tag = 0; *p = 0; } \
        return SP_OK; \
    }
