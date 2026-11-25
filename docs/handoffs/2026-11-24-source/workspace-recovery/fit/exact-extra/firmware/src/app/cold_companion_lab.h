/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "../crc32_nibble.h"

#define COLD_LAB_HEAD 32u
static int cold_lab_valid(const uint8_t *data, size_t mapped,
                          const uint8_t expected[COLD_LAB_HEAD])
{
    uint32_t bytes, crc;
    if (!data || mapped < COLD_LAB_HEAD ||
        memcmp(data, expected, COLD_LAB_HEAD))
        return 0;
    memcpy(&bytes, expected + 12, 4);
    memcpy(&crc, expected + 16, 4);
    return bytes == mapped - COLD_LAB_HEAD &&
        x0x_crc32(data + COLD_LAB_HEAD, bytes) == crc;
}
