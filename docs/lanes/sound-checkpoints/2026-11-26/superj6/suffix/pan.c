/* Scoped settled stage, not the lost full live_stage.c.
 * Source: current official ARM64 sp_panst_init and sp_panst_compute.
 * For supported settled raw rate/depth zero only, pan is identically +0
 * regardless of LFO phase. Source uses Float32(theta) then double
 * sin/cos and double products, rounding once to Float32 output. */
#include <stdlib.h>
typedef struct { int started; } pan_zero;
void *pan_live_new(unsigned rate,float raw_rate,float depth)
{
 if ((rate!=44100 && rate!=48000)||raw_rate!=0.f||depth!=0.f) return NULL;
 return calloc(1,sizeof(pan_zero));
}
void pan_live_active(void *u,int active) {((pan_zero *)u)->started=!!active;}
void pan_live_free(void *u) {free(u);}
void pan_live_run(void *u,const float *x,float *y,unsigned n)
{
 const pan_zero *s=u;
 /* Refuse an unadmitted use, rather than treating it as an effect bypass.
  * The original settled-pan.c calls active(true) before every render. */
 if (!s || !s->started || !x || !y || !n || n>128) abort();
 for (unsigned i=0;i<n;i++){
  y[i]=(float)((double)x[i]*0x1.6a09e5e333598p-1);
  y[n+i]=(float)((double)x[n+i]*0x1.6a09e6ecb41fep-1);
 }
}
