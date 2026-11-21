/* SPDX-License-Identifier: GPL-3.0-only */
#include "chicago_packed.h"
#include <stddef.h>
#ifdef X0X_CHICAGO_COMPACT_HEADERS
/* Immutable 16,385-entry tables. The generator checks each directory
 * residual and first difference fits its exact representation. */
typedef struct {
 const uint8_t *data;
 const uint16_t *base;
 const uint8_t *stride;
 const int8_t *residual;
 size_t bytes;
} chicago_compact_table;
#include "chicago_compact_private.h"
static int chicago_compact_decode(const uint8_t *p,size_t bytes,uint32_t entries,uint32_t entry,uint32_t prefix,uint32_t *out){
 if(bytes<5)return 0;
 unsigned width=(p[3]&31u);if(width>32)return 0;
 size_t residuals=entries>1?entries-2:0,payload=(residuals*width+7u)/8u;
 if(bytes<5+payload || bytes>7+payload)return 0;
 size_t header=bytes-payload;
 uint32_t value=(uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|((uint32_t)p[3]>>5)<<24|prefix;
 uint32_t delta=p[4],sign=0x80u;
 if(header>=6){delta|=(uint32_t)p[5]<<8;sign=0x8000u;}
 if(header==7){delta|=(uint32_t)p[6]<<16;sign=0x800000u;}
 delta=(delta^sign)-sign;
 if(entry)value+=delta;
 size_t bit=0;p+=header;
 /* At widths up to 24, refilling one byte while fewer than width bits
  * remain never exceeds 31 live bits. Consume each payload byte once, with
  * no speculative read past the validated payload and no 64-bit shifts.
  * Wider generic blocks retain the original fragment decoder below. */
 if(width<=24){
  uint32_t reservoir=0,mask=(1u<<width)-1u;unsigned available=0;
  for(unsigned n=1;n<entry;n++){
   while(available<width){reservoir|=(uint32_t)*p++<<available;available+=8;}
   uint32_t z=reservoir&mask;reservoir>>=width;available-=width;
   delta+=(z>>1)^(0u-(z&1u));value+=delta;
  }
  *out=value;return 1;
 }
 for(unsigned n=1;n<entry;n++){
  uint32_t z=0;unsigned used=0;
  while(used<width){unsigned shift=(unsigned)(bit%8u),take=8-shift;if(take>width-used)take=width-used;
   z|=((uint32_t)(p[bit/8u]>>shift)&((1u<<take)-1u))<<used;used+=take;bit+=take;
  }
  delta+=(z>>1)^(0u-(z&1u));value+=delta;
 }
 *out=value;return 1;
}

static size_t chicago_compact_offset(const chicago_compact_table *t, unsigned block)
{
 unsigned group=block>>4;
 return (size_t)((int)t->base[group]+(int)t->stride[group]*(int)(block&15u)+t->residual[block]);
}

static int chicago_compact_lookup(const chicago_compact_table *t,uint32_t index,uint32_t prefix,uint32_t *out)
{
 if(index>=16385u)return 0;
 unsigned block=index/32u;
 size_t start=chicago_compact_offset(t,block);
 size_t end=block<512u ? chicago_compact_offset(t,block+1u) : t->bytes;
 if(start>end || end>t->bytes)return 0;
 return chicago_compact_decode(t->data+start,end-start,block<512u ? 32u : 1u,index%32u,prefix,out);
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
