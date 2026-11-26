/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdlib.h>
#include "soundpipe.h"
#include "exact-room.h"
#include "micro.h"
struct room {
    sp_data *sp;
    void *rev;
    sp_buthp *hp[2];
    sp_compressor *comp[3][2];
};
void *exact_room_new(float sr) {
    struct room *r=calloc(1,sizeof *r);
    if(!r || sr!=44100.f)abort();
    sp_create(&r->sp); if(!r->sp)abort();
    r->sp->sr=(int)sr; r->sp->nchan=2;
    micro_config(1.1f,.98f,.15f,1.f);
    if(!(r->rev=micro_new()))abort();
    for(unsigned c=0;c<2;++c){
        if(sp_buthp_create(&r->hp[c])!=SP_OK||!r->hp[c]||sp_buthp_init(r->sp,r->hp[c])!=SP_OK)abort();
        for(unsigned stage=0;stage<3;++stage){
            sp_compressor **comp=&r->comp[stage][c];
            if(sp_compressor_create(comp)!=SP_OK||!*comp||sp_compressor_init(r->sp,*comp)!=SP_OK)abort();
        }
    }
    return r;
}
void exact_room_drop(void *state) {
    struct room *r=state;
    if(!r)return;
    for(unsigned c=0;c<2;++c){
        for(unsigned stage=0;stage<3;++stage)sp_compressor_destroy(&r->comp[stage][c]);
        sp_buthp_destroy(&r->hp[c]);
    }
    micro_drop(r->rev); sp_destroy(&r->sp);free(r);
}
void exact_room_hp(void *s, float frequency, float *l, float *r) {
    struct room *o=s;
    float *samples[]={l,r};
    for(unsigned c=0;c<2;++c){
        float out = 0.f; /* match original output when the HP cutoff is exactly 0 */
        o->hp[c]->freq=frequency;
        sp_buthp_compute(o->sp,o->hp[c],samples[c],&out);
        *samples[c]=out*2.f;
    }
}
void exact_room_comp(void *s,unsigned which,float ratio,float threshold,float attack,
                     float release,float makeup,float*l,float*r){
    struct room *o=s;
    float *samples[]={l,r};
    assert(which<3);
    /* Preserve the source ordering, independent right then left. */
    for(int c=1;c>=0;--c){
        sp_compressor *p=o->comp[which][c];
        *p->atk=attack;*p->rel=release;*p->thresh=threshold;*p->ratio=ratio;
        float out=0.f;
        sp_compressor_compute(o->sp,p,samples[c],&out);
        *samples[c]=out*makeup;
    }
}
void exact_room_return(void *s,float fb,float l,float r,float*wl,float*wr){
    struct room *o=s;
    micro_tick(o->rev,fb,l,r,wl,wr);
}
