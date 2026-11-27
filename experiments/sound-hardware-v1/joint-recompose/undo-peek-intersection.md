# Skip unrelated bytes during Undo preflight

This is independent firmware-wide optimization, not a DSP or audio sound change. Typed undo preflight reads small windows (one-byte controls through small structs) from each existing encoded run. Walk run descriptors in the original order, but process only the intersection of each run and requested window. Offsets, stored run format, late-overwrite precedence, reinstated project values and region initialization are unchanged.

The exact source differential matches every byte in 24,000 ranges (1,529,671 requested bytes), with a second signed fill sentinel for bytes beyond each request. Normal Undo round-trip and 2,107 word-scan entry tests pass. 256 PI32 native isolated emulator peeks cover 1-, 16-, and 32-byte fields, zero divergences. Sum of 24 MHz timer is 377,365 → 23,564 ticks, 61.42 → 3.84 µs mean per peek at 360 MHz for this workload (16.0×). This is not a whole-Undo or audio deadline speedup. Paired joined donor links use the same final APP/RAM extents (Elite/Pro 540/552 B APP free; 3,236 B usable pool); native undo_peek symbol is 140→138 B Elite.

Replay from current joined diagnostic donor. Apply the source patch, run tests/host/undo_test.c and undo_word_scan_test.c. Build peek-check.c with the patched app include and -O2 -fsanitize=undefined; compile peek-native.c with PI32v2 R3, -Os and the standard raw payload start.S / bench.ld. Strict/no-skip RAM report at offset 0x10000 consists of magic 0x554e4450, old ticks, new ticks, and mismatches as little-endian uint32.

### undo-intersection.patch

```diff
--- a/firmware/src/app/undo.c
+++ b/firmware/src/app/undo.c
@@ -231,9 +231,11 @@
 {
     for(uint32_t i=0;i<len;i++)out[i]=u->region[off+i];
     const uint8_t *e=u->pool+u->at[k];unsigned runs=ud_get16(e);e+=8;
+    const uint32_t end=off+len;
     for(unsigned r=0;r<runs;r++) {
         uint32_t at=ud_get16(e),n=ud_get16(e+2);e+=4;
-        for(uint32_t i=0;i<n;i++)if(at+i>=off && at+i<off+len)out[at+i-off]=e[i];
+        const uint32_t lo=at>off?at:off, hi=at+n<end?at+n:end;
+        for(uint32_t i=lo;i<hi;++i)out[i-off]=e[i-at];
         e+=(n+1u)&~1u;
     }
 }
```

### peek-check.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define UNDO_POOL 4096u
#include "undo.c"
static undo_t u;
static uint8_t region[2048],a[128],b[128];
static uint32_t rng=1;
static uint32_t rnd(void){rng=rng*1664525u+1013904223u;return rng>>8;}
static void orig(uint32_t off,uint32_t len)
{
    memcpy(a,region+off,len);
    const uint8_t *e=u.pool;
    unsigned runs=ud_get16(e);e+=8;
    for(unsigned r=0;r<runs;r++){
        uint32_t at=ud_get16(e),n=ud_get16(e+2);e+=4;
        for(uint32_t i=0;i<n;i++)if(at+i>=off&&at+i<off+len)a[at+i-off]=e[i];
        e+=(n+1u)&~1u;
    }
}
static void put16(uint8_t *q,unsigned n){q[0]=n&255;q[1]=n>>8;}
int main(void)
{
    u.region=region;
    unsigned long bytes=0,copied=0,inspected=0;
    for(unsigned k=0;k<24000;k++){
        unsigned off=rnd()%1921,len=rnd()%129,runs=rnd()%25;
        for(unsigned i=0;i<sizeof region;i++)region[i]=(uint8_t)rnd();
        put16(u.pool,runs); uint8_t *e=u.pool+8;
        for(unsigned r=0;r<runs;r++){
            unsigned at=rnd()%1921,n=rnd()%127;
            if((k+r)%13==0)at=off;
            put16(e,at);put16(e+2,n);e+=4;
            for(unsigned i=0;i<n;i++){e[i]=(uint8_t)rnd();copied+=at+i>=off&&at+i<off+len;}
            e+=(n+1u)&~1u; inspected+=n;
        }
        memset(a,0xba,sizeof a);memset(b,0xba,sizeof b);
        orig(off,len);undo_peek(&u,0,off,b,len);
        if(memcmp(a,b,sizeof a))return 1;
        bytes+=len;
    }
    printf("cases=24000 signed_bytes_equal=%lu inspected=%lu intersected=%lu\n",bytes,inspected,copied);
    return 0;
}
```

### peek-native.c

```c
#include <stdint.h>
#include "undo.c"
#include "fm1_time.h"
static undo_t u;
static uint8_t region[2048],a[128],b[128];
static uint32_t rng=1;
static uint32_t rnd(void){rng=rng*1664525u+1013904223u;return rng>>8;}
static __attribute__((noinline)) void orig(uint32_t off,uint32_t len)
{
    for(unsigned i=0;i<len;++i)a[i]=region[off+i];
    const uint8_t *e=u.pool;
    unsigned runs=ud_get16(e);e+=8;
    for(unsigned r=0;r<runs;r++){
        uint32_t at=ud_get16(e),n=ud_get16(e+2);e+=4;
        for(uint32_t i=0;i<n;i++)if(at+i>=off&&at+i<off+len)a[at+i-off]=e[i];
        e+=(n+1u)&~1u;
    }
}
static void put16(uint8_t *q,unsigned n){q[0]=n&255;q[1]=n>>8;}
void bench_start(void){
 volatile uint32_t *o=(void*)0x01c10000;
 unsigned bad=0,old=0,newt=0;
 o[0]=0;fm1_time_init();u.region=region;u.at[0]=0;
 for(unsigned i=0;i<2048;i++)region[i]=(uint8_t)rnd();
 for(unsigned k=0;k<256;k++){
  unsigned off=rnd()%1921,len=(k%3u==0u?1u:(k%3u==1u?16u:32u)),runs=1+rnd()%24;
  put16(u.pool,runs);uint8_t *e=u.pool+8;
  for(unsigned r=0;r<runs;r++){
   unsigned at=rnd()%1921,n=1+rnd()%126;
   if((k+r)%13==0)at=off;
   put16(e,at);put16(e+2,n);e+=4;
   for(unsigned i=0;i<n;i++)e[i]=(uint8_t)rnd();
   e+=(n+1u)&~1u;
  }
  uint32_t t=fm1_ticks();orig(off,len);old+=fm1_ticks()-t;
  t=fm1_ticks();undo_peek(&u,0,off,b,len);newt+=fm1_ticks()-t;
  for(unsigned j=0;j<len;j++)bad+=a[j]!=b[j];
 }
 o[1]=old;o[2]=newt;o[3]=bad;o[0]=0x554e4450;
}
```

### peek-result.txt

```text
cases=24000 signed_bytes_equal=1529671 inspected=18170073 intersected=1477227
```

### native-result.json

```json
{"cases": 256, "baseline_ticks": 377365, "intersection_ticks": 23564, "mismatches": 0, "speedup": 16.014471227295875, "baseline_us_per_peek": 61.420084635416664, "candidate_us_per_peek": 3.8352864583333335}
```
