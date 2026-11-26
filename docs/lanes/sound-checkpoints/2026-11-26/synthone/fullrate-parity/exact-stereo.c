/* SPDX-License-Identifier: GPL-3.0-only */
#include "exact-stereo.h"
#include "soundpipe.h"
#include <stdlib.h>
struct stereo{
    sp_data *sp;
    sp_ftbl *sine;
    sp_osc *osc;
    sp_pan2 *pan;
    sp_phaser *ph;
};
void *exact_stereo_new(void){
    struct stereo *s=calloc(1,sizeof *s);
    if(!s)abort();
    sp_create(&s->sp);if(!s->sp)abort();
    s->sp->sr=44100;s->sp->nchan=2;
    sp_ftbl_create(s->sp,&s->sine,4096);sp_gen_sine(s->sp,s->sine);
    sp_osc_create(&s->osc);sp_osc_init(s->sp,s->osc,s->sine,0.f);
    sp_pan2_create(&s->pan);sp_pan2_init(s->sp,s->pan);
    sp_phaser_create(&s->ph);sp_phaser_init(s->sp,s->ph);
    *s->ph->MinNotch1Freq=100.f; *s->ph->MaxNotch1Freq=800.f;
    *s->ph->Notch_width=1000.f; *s->ph->NotchFreq=1.5f;
    *s->ph->VibratoMode=1.f; *s->ph->lfobpm=30.f;
    return s;
}
void exact_stereo_drop(void *ptr){
    struct stereo *s=ptr;if(!s)return;
    sp_osc_destroy(&s->osc);sp_pan2_destroy(&s->pan);sp_phaser_destroy(&s->ph);
    sp_ftbl_destroy(&s->sine);sp_destroy(&s->sp);free(s);
}
void exact_stereo(void *ptr,float mono,float pan_hz,float pan_depth,float ph_mix,
                  float ph_bpm,float ph_notch,float ph_fb,float*l,float*r){
    struct stereo *s=ptr;
    s->osc->freq=pan_hz;s->osc->amp=pan_depth;
    float p=0.f;
    sp_osc_compute(s->sp,s->osc,NULL,&p);s->pan->pan=p;
    sp_pan2_compute(s->sp,s->pan,&mono,l,r);
    *s->ph->Notch_width=ph_notch;*s->ph->feedback_gain=ph_fb;*s->ph->lfobpm=ph_bpm;
    if(ph_mix!=0.f){
        float pl=*l,pr=*r,op_l=0.f,op_r=0.f;
        const float dry=1.f-ph_mix;
        sp_phaser_compute(s->sp,s->ph,&pl,&pr,&op_l,&op_r);
        *l=dry*pl+(1.f-dry)*op_l;
        *r=dry*pr+(1.f-dry)*op_r;
    }
}
