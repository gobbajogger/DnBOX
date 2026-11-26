#!/usr/bin/env python3
"""Create first canvas delta only in a disposable exact donor copy."""
from pathlib import Path
import sys
T=Path(sys.argv[1]).resolve()
D=Path(__file__).resolve().parent
orig=D.parent/'rehydrated-reverb/dual-room/tree'
assert T!=orig and (T/'firmware/src/gfx.c').read_bytes()==(orig/'firmware/src/gfx.c').read_bytes()
assert (T/'firmware/src/app/ui.c').read_bytes()==(orig/'firmware/src/app/ui.c').read_bytes()
def replace(s,a,b):
 assert s.count(a)==1,(a,s.count(a))
 return s.replace(a,b)
f=T/'firmware/src/gfx.c';s=f.read_text()
a=s.index('static void draw_boot_screen(')
pre,s=s[:a],s[a:]
for a,b in {
'for (int band=0; band<2; ++band) {':'for (int band=0; band<3; ++band) {',
'cv_begin(240,120,C_BLACK);':'cv_begin(240,80,C_BLACK);',
'if (picture) for (unsigned y=0; y<BOOT_PIC_H; ++y)':'/* Only source rows intersecting this immutable boot strip. */\n        unsigned first = band ? (unsigned)band * 80u - 20u : 0u;\n        if (picture) for (unsigned y=first; y<BOOT_PIC_H && y+20u<(unsigned)(band+1)*80u; ++y)',
'y+20-band*120':'y+20-band*80',
'(uint32_t)band*120u':'(uint32_t)band*80u',
}.items():s=replace(s,a,b)
f.write_text(pre+s)

f=T/'firmware/src/app/ui.c';s=f.read_text()
snap=(D.parent/'recovered-sound/ui-home-strip-proof/home-snapshot.inc').read_text()
snap=replace(snap,'static void take_home','static int take_home')
snap=replace(snap,'put_s(h->status,state==SAVE_SAVED?"SAVED":header_save_label(state));','''const char *status=state==SAVE_SAVED?"SAVED":header_save_label(state);
 for(size_t i=0;;++i){
    if(i==sizeof h->status)return 0;
    if(!(h->status[i]=status[i]))break;
 }''')
snap=replace(snap,'assert(strlen(name)<sizeof h->names[pos]);put_s(h->names[pos],name);','''for(size_t i=0;;++i){
        if(i==sizeof h->names[pos])return 0;
        if(!(h->names[pos][i]=name[i]))break;
    }''')
snap=replace(snap,' }\n}\nstatic void draw_home_snapshot',' }\n return 1;\n}\nstatic void draw_home_snapshot')
wiring='''
/* Experimental opt-in only. Other 138-row routes still require CV_ROWS=138.
 * The split must stay on old dirty-tile boundaries, not at semantic row 69. */
#if X0X_UI_HOME_STRIP_SPIKE
#if defined(X0X_HOST)
static uint8_t home_strip_fixture_use_full;
#endif
'''+snap+'''
enum { HOME_STRIP_SPLIT = 80 };
_Static_assert(HOME_STRIP_SPLIT % X0X_LCD_TILE_H == 0 &&
               HOME_STRIP_SPLIT <= KNOB_H &&
               MAIN_ROWS-HOME_STRIP_SPLIT <= KNOB_H, "Home physical strips");
static UI_DRAW int commit_home_strips(void)
{
    home_snapshot h;
    if (!take_home(&h)) return 0;   /* copy limits may not change fallback pixels */
    /* Arm this single logical region once. A returning input callback can
     * invalidate the whole frame by clearing blit_hash/cv_tile_valid while
     * DMA is in progress. Never rearm it on strip two. */
    const int force = !blit_hash[1] || !cv_tile_valid[1];
    blit_hash[1] = 1u;
    cv_tile_valid[1] = 1u;
    for (int top = 0; top < MAIN_ROWS; top += HOME_STRIP_SPLIT) {
        int size = MAIN_ROWS-top > HOME_STRIP_SPLIT ? HOME_STRIP_SPLIT : MAIN_ROWS-top;
        cv_begin(240, (unsigned)size, h.color[5]);
        for (int b = 0; b < 2; ++b) {
            int a = b * BAND_H - top;
            cv_oy = a;
            cv_y0 = a > 0 ? a : 0;
            cv_y1 = a+BAND_H < size ? a+BAND_H : size;
            if (cv_y0 < cv_y1) draw_home_snapshot(b, &h);
        }
        const cv_lcd_pos_t pos = {0, MAIN_Y + (unsigned)top};
        x0x_lcd_tile_commit(cv_px, (unsigned)size,
            main_tileh + ((unsigned)top / X0X_LCD_TILE_H) * X0X_LCD_TILE_COLS,
            force, cv_lcd_emit, (void *)&pos);
    }
    return 1;
}
#endif

'''
s=replace(s,'/* SONG: a table of bars',wiring+'/* SONG: a table of bars')
s=replace(s,'    cv_begin(240, 2 * BAND_H, C_BLACK);           /* both bands, then one transfer: no tear between them */','''#if X0X_UI_HOME_STRIP_SPIKE
    if (!readout && ui.view == V_HOME && !ui.home_live && !ui.help && ui.overlay == O_NONE
#if defined(X0X_HOST)
        && !home_strip_fixture_use_full
#endif
        && commit_home_strips())
        return;
#endif
    cv_begin(240, 2 * BAND_H, C_BLACK);           /* both bands, then one transfer: no tear between them */''')
f.write_text(s)
