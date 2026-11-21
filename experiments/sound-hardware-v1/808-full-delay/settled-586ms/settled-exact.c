/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fxbus.h"
void ref_delay(fxbus_t *, const float *, float *, float *, int, float);
static fxbus_t a,b;
static int16_t ar[FX_808_DELAY_WORDS],br[FX_808_DELAY_WORDS];
static float in[256],al[256],bl[256],ax[256],bx[256];
static unsigned seq=39183;
static unsigned rnd(void){return seq=1664525*seq+1013904223;}
int main(void){
 unsigned count=0;
 fxbus_init_808_fx(&a,ar);
 for(unsigned batch=0;batch<5500;batch++){
  unsigned n=1+rnd()%256;
  for(unsigned i=0;i<256;i++){in[i]=(int32_t)rnd()*0x1p-32f;al[i]=bl[i]=(int32_t)rnd()*0x1p-33f;ax[i]=bx[i]=(int32_t)rnd()*0x1p-33f;}
  for(unsigned i=0;i<FX_808_DELAY_WORDS;i++)ar[i]=(int16_t)rnd();
  unsigned w=n%3==0?FX_808_DELAY_FRAMES-1-rnd()%n:n%3==1?rnd()%n:rnd()%FX_808_DELAY_FRAMES;
  a.w=(int32_t)w;a.dl_quiet=0;
  float d=(float)(16384+rnd()%(FX_808_DELAY_FRAMES-16384-256))+(rnd()%512)*0x1p-9f;
  a.dcur=d;a.dl_time_ms=d/44.1f;
  a.lp=(int32_t)rnd()*0x1p-33f;a.dl_fdbk=(rnd()%128)/127.f;a.dl_tc=(rnd()%128)/127.f;
  b=a;b.dbuf=br;memcpy(br,ar,sizeof ar);
  ref_delay(&a,n%5?in:0,al,n%7?ax:0,n,0.77f);fxbus_808_delay(&b,n%5?in:0,bl,n%7?bx:0,n,0.77f);
  assert(!memcmp(al,bl,sizeof al)&&!memcmp(ax,bx,sizeof ax));
  assert(!memcmp(ar,br,sizeof ar));b.dbuf=ar;assert(!memcmp(&a,&b,sizeof a));count++;
 }
 printf("Exact wet PCM, both channels, filter state, cursors, and every delay word on %u settled blocks\n",count);
}
