/* SPDX-License-Identifier: GPL-3.0-only */
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
#include "../../firmware/src/app/original_sound.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x); exit(1); } } while (0)
static void tick(void)
{
    int32_t audio[512];
    engine_render(audio,256);
}
static void select_sound(int bank,int preset)
{
    CHECK(engine_bass_select_preset(0,bank,preset,&proj.sound)==0);
    for(int i=0;i<4096 && engine_bass_preset_pending(0);++i)tick();
    CHECK(!engine_bass_preset_pending(0));
    CHECK(engine_bass_bank(0)==bank && engine_bass_preset_index(0)==preset);
}
int main(void)
{
    boot();
    for(int i=0;i<15;++i)tick();
    select_sound(ORIGINAL_SOUND_CHICAGO,0);
    int empty=motion_alloc(proj.arr.lane,31,PART_303A,T_303,0,2);
    CHECK(empty>=0);
    select_sound(ORIGINAL_SOUND_CHICAGO,1);
    CHECK(proj.arr.lane[empty].used);
    select_sound(ORIGINAL_SOUND_MIAMI,0); /* off-pattern empty slot must not veto */
    CHECK(proj.arr.lane[empty].used && proj.arr.lane[empty].t==MOT_PARK_BASS+ORIGINAL_SOUND_CHICAGO);
    select_sound(ORIGINAL_SOUND_CHICAGO,1);
    int same=motion_alloc(proj.arr.lane,31,PART_303A,T_303,0,2);
    CHECK(same>=0);
    proj.arr.lane[same].val[21]=47;
    select_sound(ORIGINAL_SOUND_CHICAGO,2);
    CHECK(proj.arr.lane[same].used && proj.arr.lane[same].val[21]==47);
    select_sound(ORIGINAL_SOUND_MIAMI,0);
    CHECK(proj.arr.lane[same].used && proj.arr.lane[same].t==MOT_PARK_BASS+ORIGINAL_SOUND_CHICAGO);
    CHECK(proj.arr.lane[same].val[21]==47);
    select_sound(ORIGINAL_SOUND_CHICAGO,2);
    ui.part=PART_303A;
    ui.view=V_PART;
    ui.overlay=O_NONE;
    build_pages();
    int page=-1;
    for(int i=0;i<pg.n;i++) if(pg.r[i][0].kind==R_BENGINE) { page=i; break; }
    CHECK(page>=0);
    *page_of(V_PART)=(uint8_t)page;
    int p0=seq.ppat[PART_303A];
    CHECK(p0>=0 && p0<NPAT-1);
    proj.pat[p0+1].rsv[0]=0; /* no separate, deliberate PAT SOUND recall */
    uint32_t serial=engine_bass_choice_serial();
    sound_t before=proj.sound;
    now_ms+=1000;enc_acc[EN_PRESET]=1;ui_input_only();
    CHECK(seq.ppat[PART_303A]==p0+1 || seq_cue_of(&seq,PART_303A)==p0+1);
    CHECK(engine_bass_choice_serial()==serial);
    CHECK(!memcmp(&before,&proj.sound,sizeof before));
    now_ms+=1000;enc_acc[EN_K2]=1;ui_input_only();
    CHECK(engine_bass_choice_serial()==serial+1);
    CHECK(engine_bass_requested_preset(0)==3);
    for(int i=0;i<4096 && engine_bass_preset_pending(0);++i)tick();
    CHECK(engine_bass_preset_index(0)==3);
    puts("PASS physical Pattern role never changes sound on ENGINE/PRESET row; K6 still changes presets");
    puts("PASS empty off-pattern lanes never veto; existing automation survives presets; another engine parks it");
    return 0;
}
