/* SPDX-License-Identifier: GPL-3.0-only */
#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
#include <math.h>
const s1c *host_coarse_owner(unsigned owner);
enum { DETUNE=30, ROUTE=51 };
#ifdef S1_REV_OLD
int main(void)
{
    for(unsigned word=0;word<8192;++word) {
        uint8_t extra[32]={0}; s1_extra_state out;
        extra[0]=(uint8_t)word; extra[1]=(uint8_t)(216u+word/256u); extra[13]=1;
        CHECK(!s1_extra_read(extra,&out));
    }
    for(unsigned owner=0;owner<2;++owner) {
        setup(V_PART,PART_303A+owner);
        proj.set.bt|=PROJ_BT_CLK_IN;
        stored_project("reverb-lfo",owner,PROJECT_MOUNT);
        unsigned w=store_writes;
        CHECK(project_load()<0 && store_writes==w);
        CHECK(project_save()<0 && store_writes==w);
        stored_project("reverb-lfo",owner,PROJECT_VERIFY);
    }
    printf("PASS exact prior reader refuses both actual Sound objects and attempted save; %u checks\n",checks);
}
#else
static void prepare(unsigned owner)
{
    setup(V_PART,PART_303A+owner);
    proj.set.bt|=PROJ_BT_CLK_IN;
    select_patch(owner); select_patch(1-owner); build_pages();
    proj.sound.v[T_MIX][PART_303A][MX_PAN]=0;
    proj.sound.v[T_MIX][PART_303B][MX_PAN]=127;
    for(unsigned i=0;i<2;++i) {
        proj.sound.v[T_MIX][PART_303A+i][MX_REV]=0;
        proj.sound.v[T_MIX][PART_303A+i][MX_DLY]=0;
    }
    proj.sound.v[T_MST][0][MST_RATIO]=proj.sound.v[T_MST][0][MST_PUMP]=0;
    proj.sound.v[T_MST][0][MST_LIMIT]=proj.sound.v[T_MST][0][MST_MODE]=0;
}
static void edit_route(unsigned owner,int value)
{
    visit("FX SWITCHES"); smc_peek_cancel(); smc_page_set(cur_page());
    unsigned ix=(unsigned)cur_page();
    CHECK(ix+1u<pg.n && pg.title[ix]==PT_FX_BYPASS && pg.title[ix+1]==PT_OUTPUT);
    pref_t r=pg.r[ix][2];
    CHECK(r.kind==R_S1PATCH && r.b==owner && s1_patch_at(r.c)->factory_id==ROUTE && smc_same(r,smc_knob_target(2)));
    int delta=value-read_slot(owner,ROUTE).position;
    if(delta) { now_ms+=250; knob(2,delta); }
    else CHECK(engine_s1_patch_set(owner,read_slot(owner,ROUTE).revision,ROUTE,value)==S1_EDIT_QUEUED);
    render(1);
    CHECK(read_slot(owner,ROUTE).physical==(float)value);
}
/* Passive host observation adjacent to the actual stock DSP stage.
 * Original multiplies wet reverb coefficient mono, using descending LFO or mean. */
static const s1c *watched;
static unsigned taps,affected,wanted;
void host_reverb_lfo_tap(const s1c *s,float in,float out)
{
    if(s!=watched) return;
    const float l1=s->lfo[0].o10,l2=s->lfo[1].o10;
    const float factor=wanted==1 ? l1 : wanted==2 ? l2 : wanted==3 ? .5f*(l1+l2) : 1.f;
    CHECK(out==in*factor);
    affected+=in!=out; ++taps;
}
static void measure(unsigned owner,unsigned route,unsigned depth,int32_t *pcm)
{
    begin_pair(owner,60); render(64);
    wanted=route; taps=affected=0;
    watched=host_coarse_owner(owner);
    record(pcm);
    watched=NULL;
    CHECK(taps==FRAMES);
    CHECK(route&&depth ? affected>FRAMES/2 : affected==0);
    fprintf(stderr,"owner=%u route=%u depth=%u changed wet reverb coefficient mono samples=%u/%u\n",
            owner,route,depth,affected,taps);
}
int main(void)
{
    CHECK(!s1_part_programs());
    const char *dump=getenv("S1_REV_PCM");
    if(dump) {
        prepare(0); begin_pair(0,57); record(reference);
        FILE *f=fopen(dump,"wb");
        CHECK(f && fwrite(reference,sizeof reference,1,f)==1 && !fclose(f));
        return 0;
    }
#ifdef S1_REV_RED
    prepare(0); (void)read_slot(0,ROUTE); return 0;
#else
    uint8_t root[32], c[32];
    CHECK(original_sound_init(root,ORIGINAL_SOUND_SYNTHONE,0));
    uint8_t extra[32];
    for(unsigned header=0;header<8192;++header) {
        unsigned in_head=header%1935u, hp=in_head%129u, left=in_head/129u, wide=left%3u, cut=left/3u;
        memset(extra,0,sizeof extra); extra[0]=(uint8_t)header;extra[1]=(uint8_t)(216u+header/256u);
        s1_extra_state out;
        CHECK(s1_extra_read(extra,&out)==(header<7740));
        if(header<7740){
            CHECK(out.reverb_mix_lfo==1u+header/1935u && out.cutoff_lfo==cut);
            CHECK(out.wet[S1_REVERB_HP-S1_WET_FIRST]==hp && out.wet[S1_WIDEN-S1_WET_FIRST]==wide);
            memcpy(c,root,32); memset(extra,0,32);
            if(hp) CHECK(original_sound_s1_extra_set(c,extra,S1_REVERB_HP,hp));
            if(wide) CHECK(original_sound_s1_extra_set(c,extra,S1_WIDEN,wide));
            if(cut) CHECK(original_sound_s1_extra_set(c,extra,S1_CUTOFF_LFO,cut));
            CHECK(original_sound_s1_extra_set(c,extra,S1_REVERB_MIX_LFO,1u+header/1935u));
            CHECK(extra[0]==(uint8_t)header && extra[1]==216u+header/256u);
            CHECK(s1_extra_read(extra,&out) && out.reverb_mix_lfo==1u+header/1935u);
            CHECK(original_sound_s1_extra_valid(c,extra) && s1_extra_size(extra)==15);
        }
    }
    memcpy(c,root,32); memset(extra,0,32); extra[14]=1;
    uint8_t stale[32]; memcpy(stale,extra,32);
    CHECK(!original_sound_s1_extra_set(c,extra,S1_REVERB_MIX_LFO,1));
    CHECK(!memcmp(c,root,32) && !memcmp(extra,stale,32));
    for(unsigned owner=0;owner<2;++owner) {
        prepare(owner);
        CHECK(read_slot(owner,ROUTE).physical==0.f && read_slot(owner,DETUNE).physical==0.f);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,ROUTE).revision,ROUTE,4)==S1_EDIT_REJECTED);
        /* Solo oscillator 2 using real controls; equivalent repeats deterministic. */
        visit("OSC MIX"); detent(0,127);
        visit("OSCILLATOR"); detent(0,127);
        visit("TEMPO SYNC"); detent(1,127); detent(2,127);
        visit("LFO"); detent(1,1); detent(3,2);
        const float l1=read_slot(owner,14).physical, l2=read_slot(owner,46).physical;
        CHECK(l1==1.f && l2==1.f);
        visit("EFFECT"); detent(2,95);
        CHECK(read_slot(owner,40).physical>.5f && read_slot(owner,37).physical==1.f);
        edit_route(owner,0);
        measure(owner,0,1,reference);
        for(unsigned mode=1;mode<=3;++mode) {
            edit_route(owner,(int)mode);
            measure(owner,mode,1,changed);
            own_pcm(owner);
        }
        visit("FX SWITCHES"); detent(1,-1); CHECK(read_slot(owner,37).physical==0.f);
        edit_route(owner,0);
        measure(owner,0,0,reference);
        for(unsigned mode=1;mode<=3;++mode) {
            edit_route(owner,(int)mode);
            measure(owner,mode,0,changed);
            CHECK(!memcmp(reference,changed,sizeof reference));
        }
        CHECK(read_slot(1-owner,ROUTE).physical==0.f);
        visit("TEMPO SYNC"); detent(1,-17); detent(2,-19);
        float set_l1=read_slot(owner,14).physical, set_l2=read_slot(owner,46).physical;
        CHECK(set_l1<1.f && set_l2<1.f && read_slot(owner,ROUTE).physical==3.f);
        visit("FX SWITCHES"); detent(0,1); detent(1,-1);
        visit("PITCH MOD"); detent(0,2); detent(1,1); detent(2,3);
        CHECK(read_slot(owner,55).physical==2.f && read_slot(owner,57).physical==1.f && read_slot(owner,59).physical==3.f);
        visit("EFFECT"); detent(0,14);
        const uint8_t *sound=proj.sound.v[T_303][owner];
        unsigned crush=original_sound_panel_position(sound,ORIGINAL_SYNTHONE_CRUSH);
        CHECK(crush && ((sound[2]&128u)!=0) && (sound[31]&1u));
        CHECK(original_sound_panel_position(sound,0)==64 && original_sound_panel_position(sound,1)==0);
        sound_t before_wheel=proj.sound;
        uint32_t channel=(uint32_t)seq.ch[TRK_BASS0+owner];
        host_midi_put(0xeu | ((0xe0u|channel)<<8) | 127u<<16 | 127u<<24); render(1);
        CHECK(engine_bass_perf(owner,0,1)==127 && read_slot(owner,ROUTE).physical==3.f);
        CHECK(!memcmp(&proj.sound,&before_wheel,sizeof before_wheel));
        host_midi_put(0xeu | ((0xe0u|channel)<<8) | 64u<<24); render(1);
        CHECK(engine_bass_perf(owner,0,1)==64 && read_slot(owner,ROUTE).physical==3.f);
        CHECK(!memcmp(&proj.sound,&before_wheel,sizeof before_wheel));
        CHECK(read_slot(owner,ROUTE).physical==3.f);
        visit("OUTPUT"); detent(1,-11);
        const float level=read_slot(owner,32).physical;
        visit("MOD ROUTE"); detent(2,2); detent(3,1);
        CHECK(read_slot(owner,54).physical==2.f && read_slot(owner,52).physical==1.f);
        CHECK(read_slot(owner,ROUTE).physical==3.f);
        stabilize(); CHECK(!project_save()); stored_project("reverb-lfo",owner,PROJECT_EXPORT);
        CHECK(!project_load());
        CHECK(read_slot(owner,14).physical==set_l1 && read_slot(owner,46).physical==set_l2);
        CHECK(read_slot(owner,ROUTE).physical==3.f && read_slot(owner,41).physical==1.f && read_slot(owner,37).physical==0.f);
    CHECK(read_slot(owner,32).physical==level && read_slot(owner,54).physical==2.f &&
              read_slot(owner,52).physical==1.f && read_slot(owner,55).physical==2.f && read_slot(owner,57).physical==1.f && read_slot(owner,59).physical==3.f);
        CHECK(original_sound_panel_position(sound,ORIGINAL_SYNTHONE_CRUSH)==crush && (sound[2]&128u));
        engine_note_lock_capture(1);
        CHECK(!engine_bass_note_capture(proj.arr.lane,0,owner,0,&proj.sound));
        engine_note_lock_capture(0);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        build_pages(); edit_route(owner,0);
        bpart_t *line=&proj.pat[0].bass[owner];
        for(unsigned i=0;i<NSTEPS;++i) line->step[i].flags=G_REST;
        line->len=16; line->rate=RATE_16; line->dir=DIR_FWD;
        line->step[0].flags=line->step[4].flags=G_NOTE;
        line->step[0].note=36; line->step[4].note=43;
        line->step[0].velocity=line->step[4].velocity=100;
        seq.ppat[TRK_BASS0+owner]=0;
        engine_note_lock_refresh(); seq_start(&seq);
        for(unsigned i=0;i<2000 && !read_slot(owner,ROUTE).note_locked;++i) render(1);
        CHECK(read_slot(owner,ROUTE).note_locked && read_slot(owner,ROUTE).physical==3.f);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,ROUTE).revision,ROUTE,1)==S1_EDIT_BUSY);
        for(unsigned i=0;i<2000 && read_slot(owner,ROUTE).note_locked;++i) render(1);
        CHECK(!read_slot(owner,ROUTE).note_locked && read_slot(owner,ROUTE).physical==0.f);
        CHECK(read_slot(owner,14).physical==set_l1 && read_slot(owner,46).physical==set_l2);
        seq_stop(&seq); render(4);
    }
    printf("PASS physical FX SWITCHES K3, original wet reverb coefficient each sample, zero-depth null, peer exact and cold recall; %u checks\n",checks);
    return 0;
#endif
}
#endif
