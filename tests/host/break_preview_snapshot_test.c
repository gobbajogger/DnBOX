/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define X0X_IMA_TABLES_DEFINE
#include "dsp/ima_adpcm.h"
#include "dsp/breaks.h"
static breaks_t *interrupted;
static unsigned decodes;
static const uint8_t *original;
static int guarded_decode(const uint8_t *data,uint32_t i,int32_t *p,int32_t *idx){
 decodes++;
 if(interrupted && data!=original){breaks_t *b=interrupted;interrupted=0;unsigned before=decodes;breaks_preview_restore(b);assert(decodes==before);assert(breaks_bank(b,0)->adpcm==original);breaks_slice_on(b,1);}
 return ima_decode(data,i,p,idx);
}
#define ima_decode guarded_decode
#include "dsp/breaks.c"
int main(void){
 static uint8_t a[4096],c[4096];memset(a,0x12,sizeof a);memset(c,0x45,sizeof c);original=a;
 breaks_t b;breaks_init(&b);brk_loop_t base={a,8192,22050,1,8},candidate={c,8192,22050,1,1};
 breaks_set_loop(&b,0,&base);breaks_set_loop(&b,1,&base);const brk_bank_t *saved=breaks_bank(&b,0);
 assert(breaks_preview_begin(&b));interrupted=&b;breaks_set_loop(&b,0,&candidate);
 assert(!interrupted);assert(!b.preview_pin);assert(breaks_bank(&b,0)==saved);assert(breaks_count(&b,0)==8);assert(b.slices&2);assert(!b.pend_on[0]);
 breaks_slice_hold(&b,1,0);breaks_sync_input(&b);assert(breaks_preview_begin(&b));breaks_set_loop(&b,0,&candidate);assert(breaks_count(&b,0)==1);
 unsigned before=decodes;breaks_preview_restore(&b);assert(decodes==before);assert(breaks_bank(&b,0)==saved);assert(breaks_count(&b,0)==8);
 /* A request canceled before DSP entry must not become a normal publication,
  * whether held-count deferral, same-loop clearing, invalid input, or decoding. */
 for(int mode=0;mode<4;mode++){
  assert(breaks_preview_begin(&b));int epoch=b.preview_epoch;
  breaks_preview_restore(&b);breaks_slice_on(&b,1);
  brk_loop_t requested=mode==1?base:candidate;
  if(mode==2) requested.adpcm=0;
  if(mode==3){breaks_slice_hold(&b,1,0);breaks_sync_input(&b);}
  b.pend[0]=base;b.pend_on[0]=1;
  unsigned count=decodes;
  breaks_set_loop_request(&b,0,&requested,epoch);
  assert(decodes==count);assert(b.pend_on[0]);assert(b.pend[0].adpcm==a);
  assert(breaks_bank(&b,0)==saved);assert(breaks_count(&b,0)==8);
  b.pend_on[0]=0;breaks_slice_hold(&b,1,0);breaks_sync_input(&b);
 }
 assert(breaks_preview_begin(&b));breaks_set_loop(&b,0,&candidate);
 assert(breaks_bank(&b,0)!=saved);assert(saved->valid);
 breaks_drop_range(&b,a,sizeof a);assert(!saved->valid);
 breaks_preview_restore(&b);assert(breaks_bank(&b,0)==saved);assert(!saved->valid);
 puts("BREAK snapshot interruption and constant-work restoration PASS");return 0;
}
