/* SPDX-License-Identifier: GPL-3.0-only */
/* Pixel regression for the real F7 matrix, native fonts and per-part voice observer. */
#define SMC_UI_MAIN retained_smc_fixture
#include "smc_ui_test.c"

enum { CELL_W = 59, CELL_H = 49, PIXELS = 240*240 };
static uint16_t label_pixels[CELL_W*CELL_H], baseline[2][PIXELS];

static void blocks(unsigned count)
{
    int32_t samples[512];
    while (count--)
        engine_render(samples, 256);
}

static void choose_patch(unsigned part)
{
    CHECK(!engine_bass_select_preset(part, ORIGINAL_SOUND_SYNTHONE, 0, &proj.sound));
    for (unsigned i = 0; i < 1000 && engine_bass_preset_pending(part); ++i)
        blocks(1);
    CHECK(engine_bass_bank(part) == ORIGINAL_SOUND_SYNTHONE);
}

static void edit(unsigned part, unsigned id, unsigned position)
{
    s1_patch_value value;
    CHECK(engine_s1_patch_read(part,id,&value));
    CHECK(engine_s1_patch_set(part,value.revision,id,position) == S1_EDIT_QUEUED);
    blocks(1);
    CHECK(engine_s1_patch_read(part,id,&value) && value.position == position);
}

static void show(unsigned part)
{
    ui.part = PART_303A + part;
    set_view(V_PART);
    ui_input_only();
    *page_of(V_PART) = 0;
    ui.touched = -1;
    ui.msg_until = 0;
    draw_header(); draw_main(); draw_knobs();
}

static void filter_labels(void)
{
    s1_patch_snapshot snapshot;
    CHECK(engine_s1_patch_snapshot(0,&snapshot));
    s1_draw_patch = &snapshot;
    /* Depth zero admits every cutoff at idle. Change only the matrix snapshot
     * to sweep its complete display domain independently of audio scheduling. */
    snapshot.physical[4] = 0;
    unsigned collisions = 0, damaged_settings = 0;
    for (unsigned slot = 2; slot <= 4; ++slot) {
        pref_t ref = PR(R_S1PATCH,T_303,0,slot);
        cv_begin(CELL_W,CELL_H,C_BLACK);
        text_c(CELL_W/2,11,&FONT_T,matrix_name(ref),SOUND_INK);
        memcpy(label_pixels,cv_px,sizeof label_pixels);
        for (unsigned cutoff = 0; cutoff <= S1_PATCH_STEPS; ++cutoff) {
            snapshot.physical[2] = s1_patch_physical(s1_patch_at(2),cutoff);
            for (unsigned resonance = 0; resonance <= S1_PATCH_STEPS; ++resonance) {
                snapshot.physical[3] = s1_patch_physical(s1_patch_at(3),resonance);
                cv_begin(CELL_W,CELL_H,C_BLACK);
                draw_param_widget(ref,0,0,CELL_W,1,matrix_col(),CELL_H,1);
                unsigned damaged = 0;
                for (unsigned i = 0; i < sizeof label_pixels/sizeof label_pixels[0]; ++i)
                    if (label_pixels[i] != swap16(C_BLACK) && cv_px[i] != label_pixels[i])
                        ++damaged;
                collisions += damaged;
                damaged_settings += damaged != 0;
            }
        }
    }
    s1_draw_patch = 0;
    printf("Filter caption mask: %u damaged pixels over %u of 49152 widget states\n",
           collisions,damaged_settings);
    CHECK(!collisions);
}

static void value_bounds(void)
{
    /* Exercise the same formatter and exact mixed-font drawing as the matrix;
     * include all editable values, including rounding across the 1s/1kHz boundary. */
    for (unsigned slot = 0; slot < S1_PATCH_COUNT; ++slot) {
        const s1_patch_descriptor *d = s1_patch_at(slot);
        unsigned column = slot < 2 ? slot : slot < 5 ? slot-2 : (slot-5)%4;
        int envelope = slot >= 5;
        int center = (envelope ? 31 : 33) + CELL_W*(int)column;
        int xmin=240,xmax=-1,ymin=CELL_H,ymax=-1;
        for (unsigned q = 0; q <= S1_PATCH_STEPS; ++q) {
            char number[24]={0}, unit[12]={0}, label[40];
            unit_value(d->unit,s1_patch_display(d,s1_patch_physical(d,q)),number,unit);
            if (envelope)
                snprintf(label,sizeof label,"K%u %c %s",UI_FIRST_KNOB+column,"ADSR"[column],number);
            else
                put_s(label,number);
            cv_begin(240,CELL_H,C_BLACK);
            strip_value(center,envelope ? 11 : 36,label,unit,C_WHITE);
            for (int y=0; y<CELL_H; ++y) for (int x=0; x<240; ++x)
                if (cv_px[y*240+x] != swap16(C_BLACK)) {
                    if (x<xmin) xmin=x;
                    if (x>xmax) xmax=x;
                    if (y<ymin) ymin=y;
                    if (y>ymax) ymax=y;
                }
        }
        CHECK(xmin >= 4 + CELL_W*(int)column && xmax < 4 + CELL_W*((int)column+1));
        CHECK(envelope ? (ymin>9 && ymax<24) : (ymin>35 && ymax<CELL_H));
    }
    puts("All 1664 formatted values fit their physical controls");
}

static void busy_frame(void)
{
    now_ms += 1000;
    enc_acc[EN_K1] = 1;
    enc_acc[EN_K1+1] = -1;
    ui_input_only();
    CHECK(!strcmp(ui.msg,"EDIT BUSY - RETRY"));
    draw_header(); draw_main(); draw_knobs();
    shot("simultaneous-editor-busy");
    blocks(1);
}

static unsigned compare(unsigned owner, const char *name, int all_rows_must_move)
{
    show(owner);
    unsigned count = 0, rows[4] = {0};
    for (int i=0; i<PIXELS; ++i) if (fb[i] != baseline[owner][i]) {
        ++count;
        int y=i/240;
        if (y>=24 && y<224) ++rows[(y-24)/50];
    }
    float amplitude,filter; unsigned stage,held;
    CHECK(engine_bass_voice_view(owner,&amplitude,&filter,&stage,&held));
    printf("%s: amp %.8f, filter %.8f, stage %u, held %u, %u changed pixels (%u/%u/%u/%u)\n",
           name,amplitude,filter,stage,held,count,rows[0],rows[1],rows[2],rows[3]);
    if (all_rows_must_move) {
        CHECK(amplitude>0 && filter>0 && stage!=6 && held);
        for (unsigned i=0; i<4; ++i) CHECK(rows[i]);
    } else CHECK(!count && stage==6 && !held);
    shot(name);
    return count;
}

int main(void)
{
    setup(V_PART,PART_303A);
    choose_patch(0); choose_patch(1);
    show(0);
    filter_labels(); value_bounds(); busy_frame();
    for (unsigned v=0; v<2; ++v) {
        edit(v,2,0); edit(v,5,127); edit(v,16,125); edit(v,17,127); edit(v,19,106);
        edit(v,22,7); edit(v,26,7);
        show(v); memcpy(baseline[v],fb,sizeof baseline[v]);
    }
    engine_bass_on_velocity(0,60,0,0,127);
    blocks(3);
    unsigned early=compare(0,"bass-a-attack",1);
    compare(1,"bass-b-idle-control",0);
    blocks(32);
    CHECK(compare(0,"bass-a-held",1) != early);
    compare(1,"bass-b-held-control",0);
    engine_bass_off_note(0,60); blocks(42);
    compare(0,"bass-a-released",0);
    puts("PASS Synth One F7 frame ownership and geometry");
    return 0;
}
