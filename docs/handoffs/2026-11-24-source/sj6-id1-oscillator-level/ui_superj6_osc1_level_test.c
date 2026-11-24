#define SJ6_LAYER1_FIXTURE_MAIN retained_osc1_tuning_main
#include "ui_superj6_layer1_tuning_test.c"
#include "osc1_old_wire.h"
static float out[2][2][COUNT];
static void arm(const juno6_t *p,unsigned which)
{
    juno6_t s=*p;
    s.core.layer[0].params.pan=-1.f; s.core.layer[1].params.pan=1.f;
    s.core.layer[0].params.sub_gain=.25f;
    CHECK(juno6_note_on(&s,69,100));
    for(unsigned i=0;i<COUNT;i+=256)
        CHECK(juno6_render_stereo(&s,out[which][0]+i,out[which][1]+i,256));
}
static void codec(void) {
    uint8_t base[32],c[32],previous[32];
    CHECK(original_sound_init(base,ORIGINAL_SOUND_JUNO,0));
    unsigned accepted=0;
    for(unsigned raw=0;raw<256;++raw) {
        unsigned hi=raw>>4;
        if((raw&7u)>4u || (hi && (hi<8u || hi>11u))) continue;
        for(unsigned pos=0;pos<=127;++pos) for(unsigned pwm=0;pwm<3;++pwm){
            memcpy(c,base,32); c[31]=(uint8_t)raw; c[30]=(uint8_t)(128+(pos^127));
            original_sound_sj6_lfo2_pwm_store(c,(uint16_t)(pwm<<7|7u));
            memcpy(previous,c,32);
            original_sound_sj6_osc1_store(c,pos);
            CHECK(original_sound_valid(c) && original_sound_sj6_osc1_code(c)==pos+1);
            CHECK(original_sound_sj6_marker(c)==raw && c[30]==previous[30]);
            CHECK(original_sound_sj6_lfo2_pwm(c)==(pwm<<7|7u));
            CHECK(!memcmp(c,previous,13) && c[16]==previous[16] && c[17]==previous[17]);
            original_sound_sj6_lfo2_pwm_store(c,0);
            CHECK(original_sound_valid(c) && original_sound_sj6_osc1_code(c)==pos+1);
            original_sound_sj6_shape_store(c,SJ6_LFO_SQUARE);
            CHECK(original_sound_sj6_shape(c)==SJ6_LFO_SQUARE && original_sound_sj6_osc1_code(c)==pos+1);
            CHECK(original_sound_set(c,13,42) && original_sound_panel_position(c,13)==42);
            CHECK(original_sound_sj6_osc1_code(c)==pos+1);
            ++accepted;
        }
    }
    CHECK(accepted==50u*128u*3u);
}
int main(void)
{
    codec();
    for(unsigned v=0;v<NBASS;++v) {
        setup_tuning(v);
        visit("OUTPUT");
        CHECK(pg.r[cur_page()][2].kind==R_SJ6KNOB && pg.r[cur_page()][2].c==SJ6_OSC1_LEVEL);
        CHECK(pg.r[cur_page()][3].c==SJ6_OSC2_LEVEL);
        CHECK(engine_superj6_knob_turn(v,SJ6_OSC2_LEVEL,85,0)); render(1);
        CHECK(engine_superj6_knob_position(v,SJ6_OSC1_LEVEL)==128);
        turn(2,-1); render(1);
        CHECK(parts[v]->osc1_level_code==128 && !parts[1-v]->osc1_level_code);
        CHECK(parts[v]->osc2_level_code==86);
        const juno6_t high=*parts[v];
        CHECK(!project_save() && !project_load());
        CHECK(parts[v]->osc1_level_code==128 && parts[v]->osc2_level_code==86);
        engine_note_lock_capture(1);
        CHECK(!engine_bass_note_capture(proj.arr.lane,0,v,0,&proj.sound));
        engine_note_lock_capture(0);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        CHECK(engine_superj6_knob_turn(v,SJ6_OSC1_LEVEL,0,0)); render(1);
        CHECK(parts[v]->osc1_level_code==1 && parts[v]->osc2_level_code==86);
        arm(&high,0); arm(parts[v],1);
        CHECK(!memcmp(out[0][1],out[1][1],sizeof out[0][1]));
        CHECK(memcmp(out[0][0],out[1][0],sizeof out[0][0]));
        unsigned nz=0; for(unsigned i=0;i<COUNT;++i) nz+=out[1][0][i]!=0.f;
        CHECK(nz>COUNT/2);
        bpart_t *line=&proj.pat[0].bass[v];
        for(unsigned n=0;n<NSTEPS;++n) line->step[n].flags=G_REST;
        line->len=16; line->rate=RATE_16; line->dir=DIR_FWD; line->transpose=24;
        line->step[0].flags=G_NOTE; line->step[0].note=45; line->step[0].velocity=100;
        seq.ppat[TRK_BASS0+v]=0; engine_note_lock_refresh(); engine_bass_edit_hold(1);
        seq_start(&seq);
        for(unsigned n=0;n<2000 && parts[v]->osc1_level_code!=128;++n) render(1);
        CHECK(parts[v]->osc1_level_code==128 && !parts[1-v]->osc1_level_code);
        CHECK(!engine_superj6_knob_turn(v,SJ6_OSC1_LEVEL,72,0));
        seq_stop(&seq); engine_bass_edit_hold(0); render(4);
        CHECK(parts[v]->osc1_level_code==1 && parts[v]->osc2_level_code==86);
        build_pages(); visit("OUTPUT"); now_ms+=2800; ui_frame();
        shot(v?"osc1-level-b":"osc1-level-a");
    }
    if (getenv("OSC1_WIRE_PATH")) for(unsigned v=0;v<NBASS;++v) {
        setup_tuning(v); memset(proj.arr.lane,0,sizeof proj.arr.lane);
        CHECK(engine_superj6_knob_turn(v,SJ6_OSC1_LEVEL,127,0)); render(1);
        engine_note_lock_capture(1);
        CHECK(!engine_bass_note_capture(proj.arr.lane,0,v,0,&proj.sound));
        engine_note_lock_capture(0);
        CHECK(!project_save()); osc1_project(v,0,1);
        uint8_t *c=proj.sound.v[T_303][v];
        c[31]=(uint8_t)original_sound_sj6_marker(c);
        c[13]&=127;c[14]&=127;c[18]&=127;c[19]&=127;
        original_sound_sj6_lfo2_pwm_store(c,original_sound_sj6_lfo2_pwm(c));
        CHECK(!original_sound_sj6_osc1_code(c));
        engine_bass_sound_refresh(&proj.sound,v);
        CHECK(!project_save()); osc1_project(v,1,1);
    }
    printf("PASS independent original OSC1 level real output K3 and audio, both owners, 19200 full joint shape/level/PWM codec states, existing ID3 independent, project/held lock (%u checks)\n",checks);
    return 0;
}
