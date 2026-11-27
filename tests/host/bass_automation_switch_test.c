/* SPDX-License-Identifier: GPL-3.0-only */
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
#include "../../firmware/src/app/original_sound.h"
extern void engine_live_key_motion_step(int voice, int step);
extern void engine_live_key_pattern_note(int voice, int note);
/* This fixture exercises logical pattern and sound edits, not physical input. */
uint64_t plat_physical_keys(void) { return (uint64_t)plat_keys()<<5; }
int plat_outer_keys(plat_outer_event_t *events, uint64_t *keys, int *overflow,
                    uint32_t epoch, int enabled, int octave, int publish)
{
    (void)events;(void)epoch;(void)enabled;(void)octave;(void)publish;
    *keys=plat_physical_keys();*overflow=0;return 0;
}


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
    for(int i=0;i<NSTEPS;++i)CHECK(proj.arr.lane[empty].val[i]==MOT_NONE);
    select_sound(ORIGINAL_SOUND_CHICAGO,1); /* old scan rejected this empty off-pattern slot */
    CHECK(proj.arr.lane[empty].t==T_303);
    int a=motion_alloc(proj.arr.lane,0,PART_303A,T_303,0,2);
    int sibling=motion_alloc(proj.arr.lane,0,PART_303B,T_303,1,2);
    CHECK(a>=0 && sibling>=0);
    proj.arr.lane[a].val[0]=43;
    proj.arr.lane[sibling].val[1]=22;
    select_sound(ORIGINAL_SOUND_CHICAGO,2);
    CHECK(proj.arr.lane[a].t==T_303 && proj.arr.lane[a].val[0]==43);
    engine_live_key_motion_step(0,0);
    CHECK(engine_motion_value(a)==43);
    select_sound(ORIGINAL_SOUND_MIAMI,0);
    CHECK(engine_motion_value(a)==-1);
    CHECK(proj.arr.lane[a].t==MOT_PARK_BASS+ORIGINAL_SOUND_CHICAGO);
    CHECK(proj.arr.lane[sibling].t==T_303 && proj.arr.lane[sibling].val[1]==22);
    int b=motion_alloc(proj.arr.lane,0,PART_303A,T_303,0,2);
    CHECK(b>=0 && b!=a);
    proj.arr.lane[b].val[0]=31;
    CHECK(project_save()==0);
    uint8_t h[32];
    engine_project_hold(1);
    CHECK(project_load_stage()==0);
    CHECK(project_stage.arr.lane[a].t==MOT_PARK_BASS+ORIGINAL_SOUND_CHICAGO);
    CHECK(project_stage.arr.lane[a].val[0]==43 && project_stage.arr.lane[b].val[0]==31);
    project_file_header(h);
    CHECK(get32(h+20)==16u && project_file_validate(h)==0);
    CHECK(project_validate_stage()==0);
    project_stage.arr.lane[a].v=99;
    CHECK(project_validate_stage()!=0);
    CHECK(project_load_stage()==0);
    put32(h+20,15u);
    CHECK(project_file_validate(h)!=0);
    engine_project_release();

    select_sound(ORIGINAL_SOUND_CHICAGO,2);
    CHECK(proj.arr.lane[a].t==T_303 && proj.arr.lane[a].val[0]==43);
    CHECK(proj.arr.lane[b].t==MOT_PARK_BASS+ORIGINAL_SOUND_MIAMI && proj.arr.lane[b].val[0]==31);
    engine_live_key_motion_step(0,0);
    CHECK(engine_motion_value(a)==43 && engine_motion_value(b)==-1);
    select_sound(ORIGINAL_SOUND_MIAMI,1);
    CHECK(proj.arr.lane[b].t==T_303 && proj.arr.lane[b].val[0]==31);
    CHECK(proj.arr.lane[a].t==MOT_PARK_BASS+ORIGINAL_SOUND_CHICAGO && proj.arr.lane[a].val[0]==43);
    engine_live_key_motion_step(0,0);
    CHECK(engine_motion_value(a)==-1 && engine_motion_value(b)==31);
    undo_start();
    select_sound(ORIGINAL_SOUND_CHICAGO,0);
    do_undo(0);
    CHECK(engine_bass_bank(0)==ORIGINAL_SOUND_MIAMI && proj.arr.lane[b].t==T_303);
    CHECK(proj.arr.lane[a].t==MOT_PARK_BASS+ORIGINAL_SOUND_CHICAGO);
    do_undo(1);
    CHECK(engine_bass_bank(0)==ORIGINAL_SOUND_CHICAGO && proj.arr.lane[a].t==T_303);
    CHECK(proj.arr.lane[b].t==MOT_PARK_BASS+ORIGINAL_SOUND_MIAMI);
    select_sound(ORIGINAL_SOUND_CHICAGO,2);
    /* Emulate the real callback order at a cue mid-block: old motion step
     * first, then the onset adopts the queued bank and applies its locks. */
    int parked=motion_alloc(proj.arr.lane,1,PART_303A,MOT_PARK_BASS+ORIGINAL_SOUND_MIAMI,0,2);
    CHECK(parked>=0);
    proj.arr.lane[parked].val[0]=55;
    CHECK(engine_bass_select_preset_at(0,ORIGINAL_SOUND_MIAMI,0,&proj.sound,1)==0);
    for(int i=0;i<12;++i)tick();
    CHECK(engine_bass_bank(0)==ORIGINAL_SOUND_CHICAGO);
    seq.ppat[PART_303A]=1;
    eng_step[PART_303A]=0;
    engine_live_key_motion_step(0,0);
    engine_live_key_pattern_note(0,48);
    CHECK(engine_bass_bank(0)==ORIGINAL_SOUND_MIAMI && proj.arr.lane[parked].t==T_303);
    CHECK(engine_motion_value(parked)==55);
    engine_live_key_pattern_note(0,-1);
    for(int i=0;i<350;++i)tick();
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
    puts("PASS same-engine selection with empty and real lanes, per-engine switch and restore, sibling isolation, storage and unsafe downgrade refusal");
    return 0;
}
