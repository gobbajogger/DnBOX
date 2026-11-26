/* SPDX-License-Identifier: GPL-3.0-only */
#include "exact-aux.h"
#include "soundpipe.h"
#include <stdlib.h>

static sp_data sp;
static sp_ftbl sine_table;
static float table[4097];
struct aux {sp_osc *sub;sp_fosc *fm;};
void *exact_aux_new(void) {
    if(!sine_table.tbl){
        sp.sr=44100;sp.nchan=2;sine_table.tbl=table;
        sp_ftbl_init(&sp,&sine_table,4096);sp_gen_sine(&sp,&sine_table);
        table[4096]=0.f;
    }
    struct aux *a=calloc(1,sizeof *a);
    if(!a)abort();
    sp_osc_create(&a->sub);sp_osc_init(&sp,a->sub,&sine_table,0.f);
    sp_fosc_create(&a->fm);sp_fosc_init(&sp,a->fm,&sine_table);
    return a;
}
void exact_aux_drop(void *state){
    struct aux *a=state;
    if(a){sp_osc_destroy(&a->sub);sp_fosc_destroy(&a->fm);free(a);}
}
float exact_aux_sample(void *state,float subfreq,float fmfreq,float amp,float square,
                       float subvol,float fmidx,float fmvol){
    struct aux *a=state;float subv=0.f,fmv=0.f;
    a->sub->freq=subfreq;a->sub->amp=amp;
    sp_osc_compute(&sp,a->sub,NULL,&subv);
    if(square!=0.f)subv=subv>0.f?subvol:-subvol;
    else subv*=subvol*3.f;
    a->fm->freq=fmfreq;a->fm->amp=amp;a->fm->indx=fmidx;
    sp_fosc_compute(&sp,a->fm,NULL,&fmv);
    fmv*=fmvol;
    return subv+fmv;
}
