/* SPDX-License-Identifier: GPL-3.0-only */
#include "exact-stereo.h"
#include "soundpipe.h"
#include <stdlib.h>
void ph_native_run(void*,float*,float*,float,float,float,float,float);
struct stereo{
    sp_data *sp;
    sp_ftbl *sine;
    sp_osc *osc;
    sp_pan2 *pan;
    void *ph;
};
void *exact_stereo_new(void){
    struct stereo *s=calloc(1,sizeof *s);
    if(!s)abort();
    sp_create(&s->sp);if(!s->sp)abort();
    s->sp->sr=44100;s->sp->nchan=2;
    sp_ftbl_create(s->sp,&s->sine,4096);sp_gen_sine(s->sp,s->sine);
    sp_osc_create(&s->osc);sp_osc_init(s->sp,s->osc,s->sine,0.f);
    sp_pan2_create(&s->pan);sp_pan2_init(s->sp,s->pan);
    s->ph=calloc(1,224);if(!s->ph)abort();
    return s;
}
void exact_stereo_drop(void *ptr){
    struct stereo *s=ptr;if(!s)return;
    sp_osc_destroy(&s->osc);sp_pan2_destroy(&s->pan);free(s->ph);
    sp_ftbl_destroy(&s->sine);sp_destroy(&s->sp);free(s);
}
void exact_stereo(void *ptr,float mono,float pan_hz,float pan_depth,float ph_mix,
                  float ph_bpm,float ph_notch,float ph_fb,float*l,float*r){
    struct stereo *s=ptr;
    s->osc->freq=pan_hz;s->osc->amp=pan_depth;
    float p=0.f;
    sp_osc_compute(s->sp,s->osc,NULL,&p);s->pan->pan=p;
    sp_pan2_compute(s->sp,s->pan,&mono,l,r);
    ph_native_run(s->ph,l,r,ph_mix,ph_bpm,ph_notch,ph_fb,44100.f);
}
