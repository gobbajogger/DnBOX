/* SPDX-License-Identifier: GPL-3.0-only */
/* Native build.py accepts the environment switch; host amalgamated callers
 * use X0X_HOST_DEFS=-DX0X_GFINAL_CHANCE_PROBE=0 for an explicit opt-out. */
#ifndef X0X_GFINAL_CHANCE_PROBE
#define X0X_GFINAL_CHANCE_PROBE 1
#endif
#if X0X_GFINAL_CHANCE_PROBE
#include "gfinal/ui_chance_exact.h"
#ifdef X0X_HOST
/* Host UI is an amalgamated TU; native retains the strict separate object. */
#include "gfinal/ui_chance_exact.c"
#endif
enum { CHANCE_NOT_TARGET, CHANCE_DRAWN, CHANCE_LIGHT_PENDING,
       CHANCE_NATIVE_LAW_PENDING, CHANCE_RENDER_ERROR };
static uint8_t chance_knob_status[4];
static int chance_native_knob(pref_t r,const x0x_param_t *d,int val,int cx)
{
    uint8_t prepared;
    uint16_t pixels[26];
    int row,x;
    if(r.kind!=R_BRKSET || r.a || r.b) return CHANCE_NOT_TARGET;
    /* Named configured event probabilities share one exact CHANCE type.
     * ROLL is the base stay amount before complexity/fill scaling; R2/3/4/8
     * are the native per-bar settings before length-indexed trigger scaling.
     * Neither glyph nor percent claims measured instantaneous outcomes. */
    switch(r.c){
    case BRK_BCHANCE: case BRK_ROLL:
    case BRK_R2: case BRK_R3: case BRK_R4: case BRK_R8: break;
    default: return CHANCE_NOT_TARGET;
    }
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
