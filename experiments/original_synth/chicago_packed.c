/* SPDX-License-Identifier: GPL-3.0-only */
#include "chicago_packed.h"
#ifdef X0X_CHICAGO_COMPACT_HEADERS
#include "chicago_compact_private.h"
#if X0X_SUPERJ6_FACTORY
/* Factory builds use an additional lossless coding for the tightly clustered
 * second differences. Bounds are checked before every reservoir refill. */
static int chicago_rice_lookup(const uint8_t *p, size_t bytes, unsigned entry,
                              uint32_t prefix, uint32_t *out)
{
 if (bytes < 8u || p[7] >= 32u) return 0;
 unsigned k = p[7] & 15u, mask = (1u << k) - 1u;
 uint32_t value = (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
                  ((uint32_t)p[3] >> 5) << 24 | prefix;
 uint32_t delta = (uint32_t)p[4] | (uint32_t)p[5] << 8 | (uint32_t)p[6] << 16;
 delta = (delta ^ 0x800000u) - 0x800000u;
 if (entry) value += delta;
 const uint8_t *end = p + bytes;
 int bias = 0;
 if (p[7] & 16u) {
  if (bytes < 10u) return 0;
  bias = (int)(((unsigned)p[8] | (unsigned)p[9] << 8) ^ 32768u) - 32768;
  p += 10;
 } else p += 8;
 unsigned available = 0, reservoir = 0;
 for (unsigned i = 1; i < entry; ++i) {
  unsigned q = 0;
  while (!reservoir) {
   q += available;
   if (p == end) return 0;
   reservoir = *p++;
   available = 8;
  }
  unsigned zeros = (unsigned)__builtin_ctz(reservoir);
  q += zeros;
  reservoir >>= zeros + 1;
  available -= zeros + 1;
  while (available < k) {
   if (p == end) return 0;
   reservoir |= (unsigned)*p++ << available;
   available += 8;
  }
  unsigned code = (q << k) | (reservoir & mask);
  delta += ((code >> 1) ^ (0u - (code & 1u))) + (uint32_t)bias;
  value += delta;
  reservoir >>= k;
  available -= k;
 }
 *out = value;
 return 1;
}
#endif
static int chicago_compact_decode(const uint8_t *p,size_t bytes,uint32_t entries,uint32_t entry,uint32_t prefix,uint32_t *out){
 if (!p || !out || bytes < 5 || entry >= entries) return 0;
 unsigned field = p[3] & 31u;
#if X0X_SUPERJ6_FACTORY
 if (field == 31) return chicago_rice_lookup(p, bytes, entry, prefix, out);
#endif
 int biased = field >= 24;
 unsigned width = biased ? field - 24 : field;
 unsigned residuals = entries > 1 ? entries - 2 : 0;
 size_t payload = (residuals * width + 7u) / 8u;
 unsigned bias_bytes = biased ? 2u : 0u;
 if (bytes < 5u + bias_bytes + payload || bytes > 7u + bias_bytes + payload) return 0;
 unsigned slope_bytes = (unsigned)(bytes - 4u - bias_bytes - payload);
 uint32_t value = (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
                  ((uint32_t)p[3] >> 5) << 24 | prefix;
 uint32_t delta = p[4], sign = 0x80u;
 if (slope_bytes >= 2) { delta |= (uint32_t)p[5] << 8; sign = 0x8000u; }
 if (slope_bytes == 3) { delta |= (uint32_t)p[6] << 16; sign = 0x800000u; }
 delta = (delta ^ sign) - sign;
 if (entry) value += delta;
 p += 4u + slope_bytes;
 int bias = 0;
 if (biased) {
  unsigned word = (unsigned)p[0] | (unsigned)p[1] << 8;
  bias = (int)(word ^ 32768u) - 32768;
  p += 2;
 }
 /* Width is at most 23, so byte refills fit a 32-bit reservoir. */
 uint32_t reservoir = 0, mask = (1u << width) - 1u;
 unsigned available = 0;
 for (unsigned i = 1; i < entry; ++i) {
  while (available < width) {
   reservoir |= (uint32_t)*p++ << available;
   available += 8;
  }
  uint32_t code = reservoir & mask;
  reservoir >>= width;
  available -= width;
  delta += biased ? code + (uint32_t)bias : (code >> 1) ^ (0u - (code & 1u));
  value += delta;
 }
 *out = value;
 return 1;
}

static int chicago_compact_lookup(const original_lossless_table16 *t,uint32_t index,uint32_t prefix,uint32_t *out){
 const unsigned block_size=1u<<CHICAGO_PACKED_SHIFT;
 if(!t||!out||!t->data||!t->offset||!t->count||index>=t->count||t->bytes>UINT16_MAX||t->blocks!=(t->count/block_size+(t->count%block_size!=0))||t->offset[0]!=0)return 0;
 uint32_t block=index/block_size,remaining=t->count-block*block_size;
 size_t pos=t->offset[block],end=block+1<t->blocks?t->offset[block+1]:t->bytes;
 if(pos>end||end>t->bytes)return 0;
 return chicago_compact_decode(t->data+pos,end-pos,remaining>block_size?block_size:remaining,index%block_size,prefix,out);
}

#else
#include "chicago_packed_private.h"
#endif
#include <string.h>
#include <stdint.h>
int original_chicago_packed_lookup(const void *context, uint32_t index, float *a, float *e)
{
    uint32_t ab, eb;
    if (context || !a || !e || a == e) return 0;
#ifdef X0X_CHICAGO_COMPACT_HEADERS
    if (!chicago_compact_lookup(&original_chicago_packed_a, index, original_chicago_packed_a_prefix, &ab) ||
        !chicago_compact_lookup(&original_chicago_packed_e, index, original_chicago_packed_e_prefix, &eb)) return 0;
#else
    if (!original_lossless_second16_lookup(&original_chicago_packed_a, index, &ab) ||
        !original_lossless_second16_lookup(&original_chicago_packed_e, index, &eb)) return 0;
#endif
    memcpy(a, &ab, sizeof ab);
    memcpy(e, &eb, sizeof eb);
    return 1;
}

int original_chicago_packed_cached(const void *context, uint32_t index, float *a, float *e)
{
    original_chicago_packed_cache *cache = (original_chicago_packed_cache *)context;
    if (!cache || !a || !e || a == e || index == UINT32_MAX) return 0;
    if (cache->index != index) {
        if (!original_chicago_packed_lookup(NULL, index, &cache->a, &cache->e)) return 0;
        cache->index = index;
    }
    *a = cache->a;
    *e = cache->e;
    return 1;
}
