/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef X0X_GFINAL_CHANCE_PROBE
#define X0X_GFINAL_CHANCE_PROBE 0
#endif
#if X0X_GFINAL_CHANCE_PROBE
#include "gfinal/ui_chance_exact.h"
enum { CHANCE_NOT_TARGET, CHANCE_DRAWN, CHANCE_LIGHT_PENDING,
       CHANCE_NATIVE_LAW_PENDING, CHANCE_RENDER_ERROR };
static uint8_t chance_knob_status[4];
static int chance_native_knob(pref_t r,const x0x_param_t *d,int val,int cx)
{
    uint8_t prepared;
    uint16_t pixels[26];
    int row,x;
    if(r.kind!=R_BRKSET || r.a || r.b || r.c!=BRK_BCHANCE)
        return CHANCE_NOT_TARGET;
    if(ui_theme_light) return CHANCE_LIGHT_PENDING;
    if(!d || d->max!=100 || ui_chance_prepare_native(val,&prepared))
        return CHANCE_NATIVE_LAW_PENDING;
    cv_rect(cx-20,26,40,24,0x10c3);
    for(row=0;row<19;++row){
        if(ui_chance_exact_row(prepared,row,pixels))return CHANCE_RENDER_ERROR;
        for(x=0;x<26;++x)cv_pset(cx-13+x,28+row,pixels[x]);
    }
    return CHANCE_DRAWN;
}
#endif
