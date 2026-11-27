#include <stdlib.h>
#include <math.h>
#include "soundpipe.h"
#include "room.h"
struct RoomOwner{sp_revsc *wet; sp_data clock;};
int sp_auxdata_alloc(sp_auxdata *aux,size_t size){aux->ptr=calloc(1,size);aux->size=size;return SP_OK;}
int sp_auxdata_free(sp_auxdata *aux){free(aux->ptr);aux->ptr=NULL;return SP_OK;}
RoomOwner *compose_room_create(void){
 RoomOwner *x=calloc(1,sizeof *x);if(!x)return NULL;
 x->clock.sr=44100;sp_revsc_create(&x->wet);sp_revsc_init(&x->clock,x->wet);
 x->wet->lpfreq=20000.f;
 return x;
}
void compose_room_destroy(RoomOwner*x){if(x){sp_revsc_destroy(&x->wet);free(x);}}
int compose_room_feedback(RoomOwner*x,float v){
 if(!x||!(v>=0.f&&v<1.f))return 0;
 x->wet->feedback=v;return 1;
}
static int disjoint(const float*x,const float*y,unsigned n){
 uintptr_t a=(uintptr_t)x,b=(uintptr_t)y,s=n*sizeof *x;
 return a>b?a-b>=s:b-a>=s;
}
int compose_room_admits(const RoomOwner*x,const RoomBlock*b,const float*l,const float*r,unsigned n){
 if(!x||!b||!l||!r||!n||n>256||n!=b->frames||!disjoint(l,r,n)||
    b->room_type!=2||b->wet_limiter_bypassed!=1||b->dry_rage_already_applied!=1||
    b->send_open>1||!(b->wet_mix>=0.f&&b->wet_mix<=1.f)||
    !(b->rage_tier_gain==.151f||b->rage_tier_gain==.404f))return 0;
 if(!b->send_open)return 1;
 const float *u=b->after_room_hp_l,*v=b->after_room_hp_r;
 if(!u||!v||!disjoint(u,l,n)||!disjoint(u,r,n)||!disjoint(v,l,n)||!disjoint(v,r,n))return 0;
 for(unsigned i=0;i<n;i++)if(!isfinite(u[i])||!isfinite(v[i]))return 0;
 return 1;
}
void compose_room_tick(RoomOwner*x,const RoomBlock*b,float*l,float*r,unsigned n){
 const float ret=(.05f+.90f*b->wet_mix)*b->rage_tier_gain;
 for(unsigned i=0;i<n;i++){
  float a=b->send_open?b->after_room_hp_l[i]:0.f,z=b->send_open?b->after_room_hp_r[i]:0.f;
  float u,v;sp_revsc_compute(&x->clock,x->wet,&a,&z,&u,&v);
  l[i]+=ret*u;r[i]+=ret*v;
 }
}
