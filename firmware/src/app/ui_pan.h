/* SPDX-License-Identifier: GPL-3.0-only */
/* Presentation only. Stored projects, CC and the DSP retain raw pan 0..127. */
#ifndef UI_PAN_H
#define UI_PAN_H
static inline int ui_pan_display(int raw)
{
    if(raw<0)raw=0;
    if(raw>127)raw=127;
    int d=raw-64,a=d<0?-d:d;
    a=(a*50+31)/63;
    if(a>50)a=50;
    return d<0?-a:a;
}
static inline int ui_pan_raw(int display)
{
    if(display<=-50)return 0;
    if(display>=50)return 127;
    int a=display<0?-display:display;
    a=(a*63+25)/50;
    return display<0?64-a:64+a;
}
static inline int ui_pan_pos(int raw,int travel)
{ return (ui_pan_display(raw)+50)*travel/100; }
#endif
