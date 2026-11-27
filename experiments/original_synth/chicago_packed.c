/* SPDX-License-Identifier: GPL-3.0-only */
#include "chicago_packed.h"
#if defined(X0X_CHICAGO_FAST32) && X0X_CHICAGO_FAST32
#include "p_a.h"
#include "p_e.h"
static uint16_t u16(const uint8_t *p){return (uint16_t)p[0]|(uint16_t)((uint16_t)p[1]<<8);}
static uint32_t u32(const uint8_t *p){return (uint32_t)u16(p)|((uint32_t)u16(p+2)<<16);}
static __attribute__((noinline)) uint32_t chicago_fast32_lookup(const unsigned char *t,unsigned idx){
    unsigned i=idx&127,ns=u16(t+2);
    const uint8_t *p=t+4+2*ns+u16(t+4+(idx>>7)*2);
    unsigned degree=p[0]>>5,width=p[0]&31,q=p[9];
    uint32_t base=u32(p+1),lo=u32(p+5);
    p+=10;
    int32_t coefficients[4];
    for(unsigned j=0;j<=degree;j++){
        uint32_t z=0;
        unsigned b,count=0;
        do{b=*p++;z|=(b&127u)<<count;count+=7;}while(b&128u);
        coefficients[j]=(int32_t)((z>>1)^(0u-(z&1u)));
    }
    int32_t v=coefficients[degree];
    for(unsigned j=degree;j>0;j--)v=coefficients[j-1]+((v*(int32_t)i)>>7);
    if(q)v=(v+(int32_t)(1u<<(q-1)))>>q;
    unsigned bit=i*width,shift=bit&7,rem=width;
    uint32_t z=0;
    p+=bit>>3;
    if(rem){
        unsigned take=8-shift;
        if(take>rem)take=rem;
        z=(unsigned)(*p++>>shift)&((1u<<take)-1u);
        rem-=take;
        while(rem){unsigned n=rem<8?rem:8;z|=((uint32_t)*p++&((1u<<n)-1u))<<take;take+=n;rem-=n;}
    }
    return base+lo+(uint32_t)v+z;
}

#elif defined(X0X_CHICAGO_COMPACT_HEADERS)
#include "chicago_compact_private.h"
static int chicago_compact_decode(const uint8_t *p,size_t bytes,uint32_t entries,uint32_t entry,uint32_t prefix,uint32_t *out){
 if(bytes<7)return 0;
 unsigned width=(p[3]&31u);if(width>32)return 0;
 size_t residuals=entries>1?entries-2:0,payload=(residuals*width+7u)/8u;
 if(bytes!=7+payload)return 0;
 uint32_t value=(uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|((uint32_t)p[3]>>5)<<24|prefix;
 uint32_t delta=(uint32_t)p[4]|(uint32_t)p[5]<<8|(uint32_t)p[6]<<16;
 delta=(delta^0x800000u)-0x800000u;
 if(entry)value+=delta;
 size_t bit=0;p+=7;
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

static int chicago_compact_lookup(const original_lossless_table16 *t,uint32_t index,uint32_t prefix,uint32_t *out){
 if(!t||!out||!t->data||!t->offset||!t->count||index>=t->count||t->bytes>UINT16_MAX||t->blocks!=(t->count/32u+(t->count%32u!=0))||t->offset[0]!=0)return 0;
 uint32_t block=index/32u,remaining=t->count-block*32u;
 size_t pos=t->offset[block],end=block+1<t->blocks?t->offset[block+1]:t->bytes;
 if(pos>end||end>t->bytes)return 0;
 return chicago_compact_decode(t->data+pos,end-pos,remaining>32?32:remaining,index%32u,prefix,out);
}

#else
#include "chicago_packed_private.h"
#endif
#include <string.h>
#include <stdint.h>
/* The display-only lookup may run on Core 0 while the Chicago audio owner
 * renders on Core 1. Read the immutable A coefficient without the pair cache. */
int original_chicago_packed_cutoff_a(uint32_t index,float *a){
 if(!a)return 0;
 uint32_t ab;
#if defined(X0X_CHICAGO_FAST32) && X0X_CHICAGO_FAST32
 if(index>16384u)return 0;
 ab=chicago_fast32_lookup(p_a,index);
#elif defined(X0X_CHICAGO_COMPACT_HEADERS)
 if(!chicago_compact_lookup(&original_chicago_packed_a,index,original_chicago_packed_a_prefix,&ab))return 0;
#else
 if(!original_lossless_second16_lookup(&original_chicago_packed_a,index,&ab))return 0;
#endif
 memcpy(a,&ab,4);return 1;
}
int original_chicago_packed_lookup(const void *context,uint32_t index,float *a,float *e){
 if(context||!a||!e||a==e)return 0;
 /* The tables are constant: repeat the last decoded pair for the same index. Two
  * Chicago parts may look up at once on the two cores (DnBOX engine.c): the index is
  * cleared before the pair is written and set after it, and a hit is taken only when
  * the index reads the same before and after the pair (else decode again). */
 static volatile uint32_t last_index=UINT32_MAX,last_a,last_e;
 uint32_t ab,eb,i0=last_index;
 ab=last_a;eb=last_e;
 if(i0!=index||last_index!=i0){
#if defined(X0X_CHICAGO_FAST32) && X0X_CHICAGO_FAST32
  if(index>16384u)return 0;
  ab=chicago_fast32_lookup(p_a,index);eb=chicago_fast32_lookup(p_e,index);
#elif defined(X0X_CHICAGO_COMPACT_HEADERS)
  if(!chicago_compact_lookup(&original_chicago_packed_a,index,original_chicago_packed_a_prefix,&ab)||!chicago_compact_lookup(&original_chicago_packed_e,index,original_chicago_packed_e_prefix,&eb))return 0;
#else
  if(!original_lossless_second16_lookup(&original_chicago_packed_a,index,&ab)||!original_lossless_second16_lookup(&original_chicago_packed_e,index,&eb))return 0;
#endif
  last_index=UINT32_MAX;last_a=ab;last_e=eb;last_index=index;
 }
 memcpy(a,&ab,4);memcpy(e,&eb,4);return 1;
}
