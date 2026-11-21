/* SPDX-License-Identifier: GPL-3.0-only */
/* The 909 delay retains the most recent block as signed 1/2048 words. Older
 * blocks store 64 ten-bit mantissas and a common scale in sixteenths. */
enum { FX10_BLOCK = 64 };
typedef struct {
    uint16_t low_block, high_block;
    int16_t words[FX10_BLOCK];
} fx10_cache_t;

static int32_t fx10_block(const fx10_cache_t *cache)
{
    return (int32_t)((uint32_t)cache->low_block | ((uint32_t)cache->high_block << 16));
}

static unsigned fx10_scale(unsigned code)
{
    if (code > 223) return 223 + (code - 223) * 32;
    return code < 16 ? 16 : code;
}

static int fx10_decode(const uint8_t *bits, const uint8_t *scales, unsigned index)
{
    unsigned at = 10 * index / 8, shift = 10 * index % 8;
    unsigned pair = bits[at] | ((unsigned)bits[at + 1] << 8);
    int mantissa = (int)(((pair >> shift) & 1023) ^ 512) - 512;
    int mag = mantissa < 0 ? -mantissa : mantissa;
    mag = (mag * (int)fx10_scale(scales[index / FX10_BLOCK]) + 8) / 16;
    if (mag > 32767) mag = 32767;
    return mantissa < 0 ? -mag : mag;
}

__attribute__((noinline)) static int fx10_read(const int16_t *store, int frames, int index)
{
    const fx10_cache_t *cache = (const fx10_cache_t *)store;
    if (fx10_block(cache) == index / FX10_BLOCK) return cache->words[index % FX10_BLOCK];
    const uint8_t *bits = (const uint8_t *)(cache + 1);
    return fx10_decode(bits, bits + FX10_DATA_BYTES(frames), (unsigned)index);
}

static void fx10_flush(fx10_cache_t *cache, uint8_t *bits, uint8_t *scales, unsigned frames, int block)
{
    if (block < 0) return;
    unsigned base = (unsigned)block * FX10_BLOCK;
    unsigned count = frames - base;
    if (count > FX10_BLOCK) count = FX10_BLOCK;
    unsigned peak = 0;
    for (unsigned i = 0; i < count; ++i) {
        int v = cache->words[i];
        unsigned mag = (unsigned)(v < 0 ? -v : v);
        if (mag > peak) peak = mag;
    }
    unsigned scale = (peak * 16 + 510) / 511;
    if (scale < 16) scale = 16;
    unsigned code = scale > 223 ? 223 + (scale - 223 + 31) / 32 : scale;
    scales[block] = (uint8_t)code;
    scale = fx10_scale(code);
    for (unsigned i = 0; i < count; ++i) {
        int word = cache->words[i];
        unsigned mag = (unsigned)(word < 0 ? -word : word);
        int q = (int)((mag * 16 + scale / 2) / scale);
        unsigned pos = (base + i) * 10, at = pos / 8, shift = pos % 8;
        unsigned mask = 1023u << shift;
        unsigned value = ((unsigned)(word < 0 ? -q : q) & 1023u) << shift;
        bits[at] = (uint8_t)((bits[at] & ~mask) | value);
        bits[at + 1] = (uint8_t)((bits[at + 1] & ~(mask >> 8)) | (value >> 8));
    }
}

__attribute__((noinline)) static void fx10_write(int16_t *store, int frames, int index, int value)
{
    fx10_cache_t *cache = (fx10_cache_t *)store;
    int block = index / FX10_BLOCK;
    int previous = fx10_block(cache);
    if (block != previous) {
        uint8_t *bits = (uint8_t *)(cache + 1);
        uint8_t *scales = bits + FX10_DATA_BYTES(frames);
        fx10_flush(cache, bits, scales, (unsigned)frames, previous);
        int base = block * FX10_BLOCK;
        int count = frames - base;
        if (count > FX10_BLOCK) count = FX10_BLOCK;
        for (int i = 0; i < count; ++i)
            cache->words[i] = (int16_t)fx10_decode(bits, scales, (unsigned)(base + i));
        cache->low_block = (uint16_t)block;
        cache->high_block = (uint16_t)((unsigned)block >> 16);
    }
    cache->words[index % FX10_BLOCK] = (int16_t)value;
}

