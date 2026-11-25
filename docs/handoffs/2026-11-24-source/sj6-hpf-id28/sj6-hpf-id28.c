/* SPDX-License-Identifier: GPL-3.0-only */
#define SJ6_LAYER1_FIXTURE_MAIN retained_tuning_main
#include "ui_superj6_layer1_tuning_test.c"
static unsigned hash_dry(juno6_t *orig)
{
    juno6_t s=*orig;
    unsigned h=2166136261u;
    CHECK(juno6_note_on(&s,57,100));
    for(unsigned b=0;b<4;b++){
        float l[256],r[256];CHECK(juno6_render_stereo(&s,l,r,256));
        for(unsigned k=0;k<256;k++){
            unsigned wl,wr;
            memcpy(&wl,&l[k],4);memcpy(&wr,&r[k],4);
            h=((h^wl)*16777619u^wr)*16777619u;
        }
    }
    return h;
}
#include "hpf-wire.h"
#ifdef HPF_NEW
static void turn_to(int want)
{
    pref_t r=pg.r[cur_page()][0];
    for(unsigned i=0;i<254&&pref_get(r)!=want;i++){
        now_ms+=200;knob(0,pref_get(r)<want?1:-1);render(1);
    }
    CHECK(pref_get(r)==want);
}
static void notes(unsigned owner, int newbase_q, const uint8_t legacy[NPARAMS_MAX])
{
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane,0,owner,0,&proj.sound));
    int i=bass_note_find(proj.arr.lane,0,owner,0);
    CHECK(i>=0 && !engine_bass_note_validate(&proj.arr.lane[i]));
    CHECK(original_sound_sj6_hpf_authored(proj.arr.lane[i].val));
    engine_note_lock_capture(0);
    turn_to(newbase_q);
    const float base=parts[owner]->hpf;
    bpart_t *line=&proj.pat[0].bass[owner];
    for(unsigned j=0;j<NSTEPS;j++)line->step[j].flags=G_REST;
    line->len=16;line->rate=RATE_16;line->dir=DIR_FWD;line->transpose=24;
    for(unsigned j=0;j<2;j++){
        line->step[j].flags=G_NOTE;line->step[j].note=45+j;line->step[j].velocity=100;
    }
    seq.ppat[TRK_BASS0+owner]=0;engine_note_lock_refresh();engine_bass_edit_hold(1);seq_start(&seq);
    for(unsigned j=0;j<2048 && parts[owner]->hpf!=-801.f;j++)render(1);
    CHECK(parts[owner]->hpf==-801.f);
    for(unsigned j=0;j<2048 && parts[owner]->hpf!=base;j++)render(1);
    CHECK(parts[owner]->hpf==base);
    seq_stop(&seq);engine_bass_edit_hold(0);render(4);
    CHECK(parts[owner]->hpf==base);
    if(getenv("HPF_WIRE_PATH")) {
        CHECK(!project_save());hpf_project(owner,0,1);
        uint8_t *cell=proj.sound.v[T_303][owner], authored[NPARAMS_MAX];
        memcpy(authored,cell,sizeof authored);
        memcpy(cell,legacy,sizeof authored);
        engine_bass_sound_refresh(&proj.sound,owner);
        CHECK(!project_save());hpf_project(owner,1,1);
        memcpy(cell,authored,sizeof authored);
        engine_bass_sound_refresh(&proj.sound,owner);
    }
}
#endif
static void journey(unsigned owner)
{
    setup_tuning(owner);
    visit("OUTPUT");
    pref_t r=pg.r[cur_page()][0];
    CHECK(r.kind==R_ENG&&r.a==T_303&&r.b==owner&&r.c==9);
    CHECK(!strcmp(pref_desc(r)->name,"HPF") && smc_same(r,smc_knob_target(0)));
    uint8_t *cell=proj.sound.v[T_303][owner];
    CHECK(original_sound_set(cell,9,64));
    /* Legacy persisted shape/old generic endpoint; intentionally not the new panel route. */
    CHECK(!(cell[16]&128u));
    engine_bass_sound_refresh(&proj.sound,owner);
    engine_set(T_303,owner,9,64);render(1);
    CHECK(parts[owner]->hpf==(64 * (1.f/127.f)));
    printf("owner %u legacy live %08x ",owner,hash_dry(parts[owner]));
    CHECK(!project_save()&&!project_load());engine_apply_sound(&proj.sound);render(4);
    uint8_t legacy[NPARAMS_MAX];memcpy(legacy,cell,sizeof legacy);
    CHECK(!(legacy[16]&128u) && legacy[9]==64);
    CHECK(parts[owner]->hpf==0.f); /* preserve prior cold profile, no silent re-interpretation */
    printf("cold %08x\n",hash_dry(parts[owner]));
#ifdef HPF_NEW
    sound_t peer=proj.sound;
    const float peer_hpf=parts[1-owner]->hpf;
    visit("OUTPUT");
    pref_set_impl(r,64,0);
    CHECK(!memcmp(legacy,cell,sizeof legacy) && parts[owner]->hpf==0.f);
    for(unsigned p=0;p<6;p++){
        int q=(int[]){1,16,32,63,96,127}[p];turn_to(q);
        float expected=(float)(800*pow(q/127.0,2.42));
        CHECK(fabsf((-parts[owner]->hpf-1)-expected)<.04f);
        CHECK((cell[16]&128u)&&((cell[16]&127u)==(legacy[16]&127u)));
        CHECK(pref_get(r)==q&&!engine_bass_sound_validate(&proj.sound));
    }
    char val[16]={0},unit[12]={0};
    CHECK(original_value_text(r,127,val,unit));
    CHECK(!strcmp(val,"800")&&!strcmp(unit,"Hz"));
    const unsigned authored=hash_dry(parts[owner]);
    CHECK(!project_save()&&!project_load());engine_apply_sound(&proj.sound);render(4);visit("OUTPUT");
    CHECK(pref_get(r)==127&&parts[owner]->hpf==-801.f&&hash_dry(parts[owner])==authored);
    for(unsigned row=1-owner;row<8;row+=2)
        CHECK(!memcmp(peer.v[T_303][row],proj.sound.v[T_303][row],NPARAMS_MAX));
    CHECK(parts[1-owner]->hpf==peer_hpf);
    CHECK(!memcmp(peer.v[T_FX],proj.sound.v[T_FX],sizeof peer.v[T_FX]));
    notes(owner,23,legacy);
    turn_to(0);CHECK(parts[owner]->hpf==-1.f);
    memset(val,0,sizeof val);CHECK(original_value_text(r,0,val,unit)&&!strcmp(val,"OFF"));
    CHECK(!project_save()&&!project_load());engine_apply_sound(&proj.sound);render(4);CHECK(parts[owner]->hpf==-1.f);
    printf("owner %u authored law, physical HPF, project/lock/restore and FX peer isolation PASS\n",owner);
#else
    cell[16]|=128u;
    CHECK(engine_bass_sound_bank(&proj.sound,owner)<0);
    CHECK(engine_bass_sound_validate(&proj.sound)!=0);
    CHECK(project_save()!=0);
    CHECK(!project_load()&&!memcmp(legacy,cell,sizeof legacy));
#endif
}
int main(void)
{
#ifndef HPF_NEW
    if (getenv("HPF_WIRE_PATH")) {
        for(unsigned v=0;v<2;v++)for(unsigned scenario=0;scenario<2;scenario++){
            setup_tuning(v);hpf_project(v,scenario,0);
            const unsigned writes=store_writes;
            CHECK(project_load()==-9 && project_storage_unsupported());
            CHECK(project_save()==-9 && store_writes==writes);
        }
        printf("PASS previous actual project reader refuses marked Sound and lock-only, zero writes (%u checks)\n",checks);
        return 0;
    }
#endif
    journey(0);journey(1);printf("PASS checks=%u\n",checks);return 0;
}
