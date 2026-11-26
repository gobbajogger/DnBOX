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

/* Standalone fixture identity and generation, not a new product format. */
typedef struct { uint8_t key[16]; unsigned generation; } identity;
int late_reject(const stage *x,const uint8_t key[16],const stage *candidate,
                const uint8_t replacement[16],uint64_t *hash) {
 owner a,b,rejected={0}; uint8_t initialfx[S1C_ARENA_BYTES];
 identity published={.generation=37}, before;
 if(!cold_stage(&a,x)||!cold_stage(&b,x))return -1;
 memcpy(published.key,key,16); before=published;
 if(!memcmp(key,replacement,16))return -2;
 memcpy(initialfx,a.fx,sizeof initialfx);
 if(a.part.arena==b.part.arena || a.part.e.pool==b.part.e.pool)return -3;
 if(!s1_part_note(&a.part,48,86)||!s1_part_note(&b.part,48,86))return -4;
 if(candidate->n!=144||x->n!=144)return -5;
 stage bad=*candidate; bad.ids[143]=255;
 uint64_t h=0xcbf29ce484222325ULL;
 for(unsigned i=0;i<192;i++){
  if(i==64){
   if(!s1_part_active(&a.part)||!memcmp(initialfx,a.fx,sizeof initialfx))return -6;
   if(!cold_stage(&rejected,candidate))return -7;
   if(cold_stage(&rejected,&bad)){
    memcpy(published.key,replacement,16); published.generation++;
    return -8;
   }
   if(memcmp(published.key,before.key,16)||published.generation!=before.generation)return -9;
   if(memcmp(a.fx,b.fx,sizeof a.fx))return -10;
  }
  if(i==88){if(!s1_part_note(&a.part,55,101)||!s1_part_note(&b.part,55,101))return -11;}
  if(i==128){if(!s1_part_note(&a.part,48,0)||!s1_part_note(&b.part,48,0))return -12;}
  if(i==142){if(!s1_part_note(&a.part,55,0)||!s1_part_note(&b.part,55,0))return -13;}
  unsigned n=1+(i*47u)%128u;
  float al[128],ar[128],bl[128],br[128];
  s1_part_render(&a.part,al,ar,n);
  s1_part_render(&b.part,bl,br,n);
  if(memcmp(al,bl,n*sizeof(float))||memcmp(ar,br,n*sizeof(float)))return -14;
  if(i>=64)for(unsigned j=0;j<n;j++){
   uint32_t l,r;
   if(!isfinite(al[j])||!isfinite(ar[j]))return -15;
   memcpy(&l,al+j,4);memcpy(&r,ar+j,4);
   h=(h^l)*1099511628211ULL;h=(h^r)*1099511628211ULL;
  }
 }
 if(memcmp(a.fx,b.fx,sizeof a.fx)||memcmp(published.key,before.key,16)||published.generation!=before.generation)return -16;
 *hash=h;return 1;
}

/* Host-only validated, canonical ratios conveyed outside DNL2. No project
 * pointer is available here; only the new owner may be modified. */
int sidecar_pitch(const stage *st,const uint8_t id[16],const uint8_t side_id[16],
                  unsigned variant,const uint32_t *ratios,unsigned count,float hz[128]) {
 if(!st||st->n!=144||memcmp(id,side_id,16)||variant>2)return 0;
 if(variant==2){
  if(!ratios||!count||count>128)return 0;
  for(unsigned j=0;j<count;j++)
   if(ratios[j]<0x3f800000u||ratios[j]>=0x40000000u||(j&&ratios[j]<ratios[j-1]))return 0;
 } else if(count)return 0;
 for(unsigned n=0;n<128;n++){
  owner q;
  if(!cold_stage(&q,st))return 0;
  float a4=q.part.e.t[143];
  /* Simulate inherited project scale in the detached owner, not persisted settings. */
  q.part.e.tuning_index=S1C_SCALE_USER;
  q.part.e.user_npo=4;
  static const unsigned char inherited[]={0,0,16,0,0,32,0,0,64,0,0,96};
  memcpy(q.part.e.user_scale,inherited,sizeof inherited);
  if(variant==1)q.part.e.tuning_index=0;  /* explicit equal temperament only in this owner */
  else if(variant==2){
   for(unsigned j=0;j<count;j++)for(unsigned k=0;k<3;k++)
    q.part.e.user_scale[j*3+k]=(uint8_t)((ratios[j]&0x7fffffu)>>(k*8));
   q.part.e.user_npo=count;
  }
  if(q.part.e.t[143]!=a4)return 0;
  s1c_start(&q.part.e,n,86);
  if(q.part.e.v[0].root!=n||!isfinite(q.part.e.v[0].freq))return 0;
  hz[n]=q.part.e.v[0].freq;
 }
 return 1;
}
