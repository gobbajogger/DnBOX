/* SPDX-License-Identifier: GPL-3.0-only */
/* Bind the real 909 mixer mirror and queue. The native build supplies
 * hardware entropy and the source-owned catalog. */
#ifdef DNR2_NATIVE_RANDOM
#include "dnr2_native_catalog.h"
#endif

static void dnr2_binding_start(void)
{
    if (dnr2_scratch.revision)
        return;
#ifdef DNR2_NATIVE_RANDOM
    uint32_t a = DNR2_NATIVE_RANDOM(0), b = DNR2_NATIVE_RANDOM(1);
    uint32_t c = DNR2_NATIVE_RANDOM(0), d = DNR2_NATIVE_RANDOM(1);
    if (!dnr2_boot_address(&dnr2_scratch, a, b, c, d))
        return;
    dnr2_scratch.control = DNR2_NATIVE_CONTROL;
    dnr2_scratch.catalog_crc = DNR2_NATIVE_CRC;
#else
    uint32_t ticks = DNR2_SCRATCH_CLOCK();
    dnr2_put(dnr2_scratch.unit, 4, (ticks & DN_LIMIT) | 1u);
    dnr2_put(dnr2_scratch.unit_high, 5, ticks);
    dnr2_scratch.control = 1;
    dnr2_scratch.catalog_crc = 0x4bdfe371u;
#endif
    dnr2_scratch.revision = 1;
}

static uint8_t dnr2_scratch_read(void)
{
    return proj.sound.v[T_MIX][PART_909][MX_LEVEL];
}

static int dnr2_scratch_store(uint8_t value)
{
    if (!engine_try_set_remote_909_level(value))
        return 0;
    proj.sound.v[T_MIX][PART_909][MX_LEVEL] = value;
    /* The core publishes the revision once. Main owns the stored mirror. */
    ui.dirty = 1;
    return 1;
}

#define ED_DNR2_STATE dnr2_scratch
#define ED_DNR2_READ dnr2_scratch_read
#define ED_DNR2_WRITE dnr2_scratch_store
#define ED_DNR2_START() dnr2_binding_start()
