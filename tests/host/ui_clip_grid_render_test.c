/* SPDX-License-Identifier: GPL-3.0-only */
/* Unchanged actual grid renderer with explicit fixture feedback. No device I/O. */
#define smk_clip_lcd_draw substitute_clip_lcd_draw
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
#undef smk_clip_lcd_draw
#include "app/smk_apc40.h"
#include "app/apc40_lcd565.h"
#include "app/apc40_lcd_idle.h"
#include "app/smk_clip_colour.h"
#define SMK_CLIP_LCD_MIN_MS 64u
#define fm1_ms now_ms
static uint8_t was_on;
static volatile uint8_t apc_led_velocity[SMK_APC40_CLIP_COUNT];
static volatile uint8_t apc_cell_flags[SMK_APC40_CLIP_COUNT];
static uint8_t apc_cols,apc_rows,apc_origin_col,apc_origin_row,apc_view,apc_view_has_clips;
static void ck_page_draw(void) { CHECK(0); } /* Subpages are outside this grid-only fixture. */
#include "actual_clip_grid.inc"
static void render_grid(const char *name)
{
    uint8_t velocities[40],flags[40];
    for(int i=0;i<40;i++){velocities[i]=apc_led_velocity[i];flags[i]=apc_cell_flags[i];}
    was_on=0;now_ms+=100;draw_header();draw_knobs();
    CHECK(!ck_page_on());CHECK(smk_clip_lcd_draw(240,2*BAND_H));
    cv_commit_rect(1,0,MAIN_Y);shot(name);
    smk_apc40_lcd_map_t map=smk_apc40_lcd_map(apc_cols,apc_rows,apc_origin_col,apc_origin_row,apc_view_has_clips,240,2*BAND_H);
    CHECK(map.pitch>0 && map.ncols>0 && map.nrows>0);
    for(unsigned row=0;row<map.nrows;row++)for(unsigned col=0;col<map.ncols;col++) {
        int x=map.x0+col*map.pitch;
        int y=map.y0+smk_apc40_lcd_screen_row(&map,map.row0+row)*map.pitch;
        unsigned note=(map.row0+row)*SMK_APC40_SOURCE_COLS+map.col0+col;
        CHECK(note<40);CHECK(x>=0 && x+map.pitch<=240);
        CHECK(y>=0 && y+map.pitch<=2*BAND_H);
        uint16_t expected=(apc_cell_flags[note]&SMK_APC40_CELL_QUEUED)?C_WHITE:
            apc_led_velocity[note]?smk_clip_lcd565(apc_led_velocity[note]):C_DIM;
        CHECK(fb[(MAIN_Y+y+map.pitch/2)*240+x+map.pitch/2]==swap16(expected));
    }
    memcpy(before_fb,fb,sizeof fb);
    CHECK(!smk_clip_lcd_draw(240,2*BAND_H));
    CHECK(!memcmp(before_fb,fb,sizeof fb));
    for(int i=0;i<40;i++){CHECK(velocities[i]==apc_led_velocity[i]);CHECK(flags[i]==apc_cell_flags[i]);}
    printf("ACTUAL_GRID %s cols=%u rows=%u origin=%u,%u view=%u\n",name,apc_cols,apc_rows,apc_origin_col,apc_origin_row,apc_view);
}
int main(void)
{
    setup(V_SEQ,PART_909);clip_grid_mode=1;ck.pg=0;
    apc_cols=8;apc_rows=5;apc_view_has_clips=1;
    render_grid("clip-grid-empty");
    for(int i=0;i<40;i++)apc_led_velocity[i]=(uint8_t)(1+(i*3)%127);
    render_grid("clip-grid-full-view0");
    for(int i=0;i<40;i++)apc_cell_flags[i]=(i%3==0)?SMK_APC40_CELL_QUEUED:0;
    render_grid("clip-grid-queued-view0");
    apc_view=1;render_grid("clip-grid-queued-view1");
    apc_cols=4;apc_rows=4;apc_origin_col=2;apc_origin_row=1;apc_view=0;
    render_grid("clip-grid-compact-offset");
    /* Rate-limited unchanged output and changed palette state use real signature logic. */
    now_ms+=64;apc_led_velocity[10]^=1;
    CHECK(smk_clip_lcd_draw(240,2*BAND_H));cv_commit_rect(1,0,MAIN_Y);
    CHECK(memcmp(before_fb,fb,sizeof fb));
    printf("PASS actual clip-grid renderer: %u checks; five fixture states, not hardware or complete clip subpages\n",checks);
    return 0;
}
