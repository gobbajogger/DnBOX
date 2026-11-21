/* SPDX-License-Identifier: GPL-3.0-only */
/* The producer instruments the fallback painter's entry and optional elision.
 * A single malformed sibling lock makes the real wave projection reject a
 * valid Chicago oscillator, without substituting a renderer or data provider. */
#define SMC_UI_MAIN retained_ui_main
#include "../host/smc_ui_test.c"

static void redraw_wave(void)
{
    memset(blit_hash, 0, sizeof blit_hash);
    memset(cv_tile_valid, 0, sizeof cv_tile_valid);
    ui_frame();
}
int main(void)
{
    setup(V_PART, PART_303A);
    CHECK(engine_bass_select_preset(0,ORIGINAL_SOUND_CHICAGO,0,&proj.sound)==0);
    int32_t audio[512];
    for (int i=0;i<12;i++) engine_render(audio,256);
    CHECK(engine_bass_bank(0) == ORIGINAL_SOUND_CHICAGO);
    ui.page[V_PART][PART_303A]=0;
    build_pages();
    ui.held_step=0; ui.held_step_part=PART_303A; ui.held_step_pat=seq.ppat[PART_303A];
    pref_t sibling=PR(R_ENG,T_303,0,2), wave=PR(R_ENG,T_303,0,0);
    CHECK(lock_step()==0 && lockable(sibling));
    int slot=motion_alloc(proj.arr.lane,seq.ppat[PART_303A],PART_303A,T_303,0,2);
    CHECK(slot>=0);
    proj.arr.lane[slot].val[0]=127;
    wave_probe_calls=0;
    redraw_wave();
    CHECK(original_wave_view(wave,pref_get(wave)) && !wave_probe_calls);
    shot("chicago-wave-valid-lock");

    proj.arr.lane[slot].val[0]=128;
    CHECK(lock_of(sibling,0)==128 && !original_wave_view(wave,pref_get(wave)));
    redraw_wave();
    CHECK(wave_probe_calls>0);
    shot("chicago-wave-invalid-lock");
    static uint16_t real[240*240];
    memcpy(real,fb,sizeof real);
    wave_probe_hide=1;
    redraw_wave();
    int count=0, min_x=240, min_y=240, max_x=-1, max_y=-1;
    for (int y=0;y<240;y++) for (int x=0;x<240;x++) {
        if (fb[240*y+x]==real[240*y+x]) continue;
        ++count;
        if (x<min_x) min_x=x;
        if (x>max_x) max_x=x;
        if (y<min_y) min_y=y;
        if (y>max_y) max_y=y;
    }
    CHECK(count>0 && min_x>=38 && max_x<69 && min_y>=24 && max_y<73);
    printf("WAVE_FALLBACK %d %d %d %d %d\n", count,min_x,min_y,max_x,max_y);
    return 0;
}
