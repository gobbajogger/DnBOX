#define X0X_IMA_TABLES_DEFINE 1
#include "dsp/ima_adpcm.h"
#include <stdint.h>
#include <math.h>
#include <memory.h>
#include "part.inc"
#define emit dnl_emit
#include "select.c"
#undef emit
typedef struct { s1_dry_part part; _Alignas(8) uint8_t fx[S1C_ARENA_BYTES]; } owner;
static owner o[2];
static unsigned active=0;
static uint64_t go(owner *own) {
 float l[128],r[128]; uint64_t h=0xcbf29ce484222325ULL;
 if(!s1_part_note(&own->part,48,86))return 0;
 for(unsigned b=0;b<32;b++){
  if(b==16&&!s1_part_note(&own->part,48,0))return 0;
  s1_part_render(&own->part,l,r,128);
  for(unsigned i=0;i<128;i++){
   if(!isfinite(l[i])||!isfinite(r[i]))return 0;
   uint32_t a,c; memcpy(&a,l+i,4);memcpy(&c,r+i,4);
   h=(h^a)*1099511628211ULL;h=(h^c)*1099511628211ULL;
  }
 }
 return h;
}
unsigned active_owner(void){return active;}
static int cold_stage(owner *q,const stage *x) {
 memset(q,0,sizeof *q);
 s1_part_init(&q->part);
 s1_part_bind(&q->part,q->fx);
 load_init(&q->part.e);
 for(unsigned i=0;i<x->n;i++){
  float f;memcpy(&f,x->words+i,4);
  if(!s1c_front_set(&q->part.e,x->ids[i],f))return 0;
 }
 s1_finish(&q->part,S1_PART_INIT); /* external stable identity held separately; never stale legacy index */
 return 1;
}
int staged_adopt(const stage *x,uint64_t *h) {
 if(!x||x->n!=144||!h)return 0;
 owner probe;
 if(!cold_stage(&probe,x))return 0;
 *h=go(&probe);
 if(!*h||!cold_stage(o+(active^1u),x))return 0;
 active^=1u;return 1;
}
uint64_t legacy_holdout(const uint8_t *p,unsigned n,unsigned program) {
 owner q={0};
 if(!s1_part_set_pack(p,n))return 0;
 s1_part_init(&q.part);s1_part_bind(&q.part,q.fx);
 if(!s1_part_program(&q.part,program)){s1_part_set_pack(0,0);return 0;}
 s1_part_set_pack(0,0);
 return go(&q);
}
