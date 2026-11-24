/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
#include "../../firmware/src/app/s1c_part.h"

void host_s1_retrigger(const s1c *s) { (void)s; }
enum { AMP_DECAY_LFO = 52 };
const s1c *host_s1_owner(unsigned owner);
static uint8_t pack[0xf000];
static float takes[2][NBASS][FRAMES][2];
static unsigned take_frames[NBASS];
static int record_take = -1;

void host_lfo_capture(unsigned owner, const float *left, const float *right, unsigned frames)
{
    if (record_take < 0) return;
    for (unsigned i = 0; i < frames && take_frames[owner] < FRAMES; ++i) {
        float *sample = takes[record_take][owner][take_frames[owner]++];
        sample[0] = left[i];
        sample[1] = right[i];
    }
}

/* Test-only reads from the actual runtime owner. */
void host_s1_wet_peek(unsigned owner, float enables[2], unsigned layout[5]);

static void take(unsigned n)
{
    memset(take_frames, 0, sizeof take_frames);
    record_take = (int)n;
    record(n ? changed : reference);
    record_take = -1;
    for (unsigned owner = 0; owner < NBASS; ++owner) CHECK(take_frames[owner] == FRAMES);
}

static void choose(unsigned owner, unsigned preset)
{
    CHECK(!engine_bass_select_preset(owner, ORIGINAL_SOUND_SYNTHONE, preset, &proj.sound));
    for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n) render(1);
    CHECK(!engine_bass_preset_pending(owner));
    CHECK(engine_bass_bank(owner) == ORIGINAL_SOUND_SYNTHONE);
    CHECK(engine_bass_preset_index(owner) == (int)preset);
    build_pages();
}


static unsigned program_for(unsigned audible)
{
    static s1_dry_part part;
    for(unsigned i=0;i<694;++i) {
        CHECK(s1_part_program(&part,i));
        const float *t=part.e.t;
        if(t[52]==0.f && t[14]>.02f && t[46]>.02f && t[26]<=.01f && t[12]>.05f &&
           (audible ? t[27]>.25f && t[28]<.75f : t[27]==.005f && t[28]<1.f)) return i+1u;
    }
    CHECK(0 && "authentic program with AMP decay and two usable LFOs");
    return 0;
}

static void capture_lock(unsigned owner)
{
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane,0,owner,0,&proj.sound));
    engine_note_lock_capture(0);
    CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
}

#if !defined(S1_WET_RED) && !defined(S1_WET_LEGACY)
#endif
static void historical(void)
{
    uint8_t clean[32], original[32], cell[32];
    CHECK(original_sound_init(clean, ORIGINAL_SOUND_SYNTHONE, 0));
    for (unsigned bank=0;bank<=3;++bank) {
        memcpy(cell,clean,32); cell[30]=(uint8_t)bank;
        cell[0]=33; cell[4]=45; cell[22]=17;
        memcpy(original,cell,32);
        CHECK(original_sound_is_legacy_slot2_cell(cell));
        CHECK(original_sound_adopt_legacy_cell(cell));
        CHECK(!memcmp(cell,clean,32));
        original[24]|=128u; /* explicit typed marker never looks historical */
        memcpy(cell,original,32);
        CHECK(!original_sound_is_legacy_slot2_cell(cell));
        CHECK(!original_sound_adopt_legacy_cell(cell) && !memcmp(cell,original,32));
    }
}
static void historical_owner(void)
{
    for (unsigned owner=0;owner<NBASS;++owner) {
        setup(V_PART,PART_303A+owner); choose(owner,0);
        sound_t baseline=proj.sound;
        uint8_t cell[32];
        CHECK(original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, 0));
        for (unsigned tag=0;tag<2;++tag) for (unsigned historical_bank=0;historical_bank<=3;++historical_bank) {
            sound_t legacy=baseline;
            memcpy(legacy.v[T_303][owner],cell,32);
            legacy.v[T_303][owner][0]=33;
            legacy.v[T_303][owner][22]=1;
            legacy.v[T_303][owner][30]=(uint8_t)historical_bank;
            if (!tag) memset(&legacy.bass[owner],0,sizeof legacy.bass[owner]);
            CHECK(engine_bass_sound_bank(&legacy,owner)==ORIGINAL_SOUND_SYNTHONE);
            CHECK(!memcmp(legacy.v[T_303][owner],cell,32));
            CHECK(!memcmp(&legacy.bass[owner],&baseline.bass[owner],sizeof legacy.bass[owner]));
        }
    }
    puts("PASS both owners historical slots 0..3 adopt and retag identically, with or without old tag");
}

#if !defined(S1_WET_RED) && !defined(S1_WET_LEGACY)
static void codec(void)
{
    uint8_t cell[32], before[32], extra[15]={0}, old_extra[15];
    CHECK(original_sound_init(cell,ORIGINAL_SOUND_SYNTHONE,0));
    CHECK(original_sound_s1_set(cell,S1_PART_LEVEL,21));
    CHECK(original_sound_s1_set(cell,S1_LFO1_DEPTH,75));
    CHECK(original_sound_s1_extra_set(cell,extra,S1_REVERB_HP,87));
    CHECK(original_sound_s1_extra_set(cell,extra,S1_WIDEN,2));
    CHECK(original_sound_s1_extra_set(cell,extra,S1_CUTOFF_LFO,4));
    memcpy(before,cell,sizeof before); memcpy(old_extra,extra,sizeof old_extra);
    for(unsigned noise=0;noise<=4;++noise) for(unsigned mix=0;mix<=4;++mix) for(unsigned res=0;res<=4;++res) {
        uint8_t old[32]; memcpy(old,before,sizeof old);
        if(res) CHECK(original_sound_s1_set(old,S1_RESONANCE_LFO,res-1u));
        if(noise) CHECK(original_sound_s1_set(old,S1_NOISE_LFO,noise-1u));
        if(mix) CHECK(original_sound_s1_set(old,S1_OSC_MIX_LFO,mix-1u));
        for(unsigned fm=0;fm<=4;++fm) for(unsigned decay=0;decay<=4;++decay) {
            memcpy(cell,old,sizeof cell);
            if(decay) CHECK(original_sound_s1_set(cell,S1_AMP_DECAY_LFO,decay-1u));
            if(fm) CHECK(original_sound_s1_set(cell,S1_FM_LFO,fm-1u));
            CHECK(cell[30]==fm+5u*decay && original_sound_valid(cell) && !original_sound_is_legacy_slot2_cell(cell));
            CHECK(!original_sound_adopt_legacy_cell(cell));
            for(unsigned n=0;n<sizeof cell;++n)
                if(n!=30) CHECK(cell[n]==(old[n]|(n==24 && (fm || decay) ? 128u : 0u)));
            CHECK(!memcmp(extra,old_extra,sizeof extra));
        }
    }
    for(unsigned tag=0;tag<2;++tag) for(unsigned code=0;code<256;++code) {
        memcpy(cell,before,sizeof cell); cell[24]|=(uint8_t)(tag*128u); cell[30]=(uint8_t)code;
        CHECK(!!original_sound_valid(cell)==(tag ? code>=1u && code<25u : !code));
    }
    memcpy(cell,before,sizeof cell);
    CHECK(!original_sound_s1_set(cell,S1_AMP_DECAY_LFO,4) && !memcmp(cell,before,sizeof cell));
    puts("PASS all 3125 mixed independent states; all 512 marker/payload combinations classified");
}

static void lock_roundtrip(unsigned owner, float authored, float factory)
{
    const unsigned id=AMP_DECAY_LFO;
    bpart_t *line=&proj.pat[0].bass[owner];
    for(unsigned s=0;s<NSTEPS;++s) line->step[s].flags=G_REST;
    line->len=16; line->rate=RATE_16; line->dir=DIR_FWD; line->transpose=24;
    line->step[0].flags=line->step[4].flags=G_NOTE;
    line->step[0].note=36; line->step[4].note=43;
    line->step[0].velocity=line->step[4].velocity=100;
    seq.ppat[TRK_BASS0+owner]=0;
    engine_note_lock_refresh(); seq_start(&seq);
    for(unsigned n=0;n<2000 && !read_slot(owner,id).note_locked;++n) render(1);
    CHECK(read_slot(owner,id).note_locked && read_slot(owner,id).physical==authored);
    sound_t before=proj.sound; lane_t lane=proj.arr.lane[0];
    CHECK(engine_s1_patch_set(owner,read_slot(owner,id).revision,id,2)==S1_EDIT_BUSY);
    CHECK(!memcmp(&before,&proj.sound,sizeof before) && !memcmp(&lane,&proj.arr.lane[0],sizeof lane));
    for(unsigned n=0;n<2000 && read_slot(owner,id).note_locked;++n) render(1);
    CHECK(!read_slot(owner,id).note_locked && read_slot(owner,id).physical==factory);
    seq_stop(&seq); render(4); seq_start(&seq);
    for(unsigned n=0;n<2000 && !read_slot(owner,id).note_locked;++n) render(1);
    CHECK(read_slot(owner,id).note_locked && read_slot(owner,id).physical==authored);
    seq_stop(&seq); render(4);
    CHECK(!read_slot(owner,id).note_locked && read_slot(owner,id).physical==factory);
}

static const s1c_voice *held_voice(const s1c *e)
{
    for(unsigned j=0;j<S1C_VOICES;++j)
        if(e->v[j].gate>0.f && e->v[j].root==57) return &e->v[j];
    return NULL;
}

static void route_journey(unsigned owner,unsigned preset,unsigned audible)
{
    const unsigned id=AMP_DECAY_LFO, col=3;
    setup(V_PART,PART_303A+owner); memset(proj.arr.lane,0,sizeof proj.arr.lane);
    proj.set.bt|=PROJ_BT_CLK_IN;
    choose(owner,preset); choose(1-owner,0);
    CHECK(pg.n==15); visit("MOD ROUTE");
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][col])->name,"AMP DEC LFO"));
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][0])->name,"OSC MIX LFO"));
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][1])->name,"RES LFO"));
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][2])->name,"FM LFO"));
    const s1_patch_value init=read_slot(owner,id);
    CHECK(init.physical==0.f && init.max_position==3);
    const float peer=read_slot(1-owner,id).physical;
    const s1c *e=host_s1_owner(owner);
    const float d1=e->t[14], d2=e->t[46], r1=e->t[15], r2=e->t[47], base=e->t[27], cutoff=e->t[16], noise=e->t[12];
    /* Prior HP and Widen stay live, including in old Sound/note payloads. */
    CHECK(engine_s1_patch_set(owner,read_slot(owner,39).revision,39,12)==S1_EDIT_QUEUED); render(1);
    CHECK(engine_s1_patch_set(owner,read_slot(owner,121).revision,121,1)==S1_EDIT_QUEUED); render(1);
    const float old_hp=read_slot(owner,39).physical;
    CHECK(engine_s1_patch_set(owner,read_slot(owner,48).revision,48,3)==S1_EDIT_QUEUED); render(1);
    CHECK(engine_s1_patch_set(owner,read_slot(owner,53).revision,53,2)==S1_EDIT_QUEUED); render(1);
    CHECK(engine_s1_patch_set(owner,read_slot(owner,50).revision,50,2)==S1_EDIT_QUEUED); render(1);
    CHECK(engine_s1_patch_set(owner,read_slot(owner,49).revision,49,2)==S1_EDIT_QUEUED); render(1);
    CHECK(engine_s1_patch_set(owner,read_slot(owner,54).revision,54,2)==S1_EDIT_QUEUED); render(1);
    uint8_t old_tail[15];
    memcpy(old_tail,proj.sound.v[T_303][S1_EXTRA_ROW+owner],sizeof old_tail);
    if(audible) {
        capture_lock(owner); stabilize(); CHECK(!project_save() && !project_load());
        stored_project("decay-v1-fm",owner,PROJECT_EXPORT);
    }

    float phase[2][2], shapes[2][2], envelope[2], filter_env[2], pole[2];
    static float held_audio[NBASS][FRAMES][2];
    float prior_lfo1[FRAMES][2], prior_lfo2[FRAMES][2];
    sound_t oldsound=proj.sound;
    for(unsigned mode=1;mode<=3;++mode) {
        proj.sound=oldsound;
        for(unsigned side=0;side<2;++side) {
            begin_pair(owner,57); render(4);
            const s1c_voice *v=held_voice(e);
            CHECK(v && v->ea.mode==S1U_E_DEC);
            const float held_pole=v->ea.a;
            for(unsigned step=0;step<3;++step) {
                detent(col,side && step<mode ? 1 : 0);
                CHECK(read_slot(owner,id).physical==(float)(side ? (step<mode ? step+1 : mode) : 0));
            }
            CHECK(v->ea.mode==S1U_E_DEC && v->ea.a==held_pole);
            take(side);
            /* Original and compact sample the input at ATTACK->DECAY.
             * A retained, already-decaying note must not be retimed. */
            if(!side) memcpy(held_audio,takes[side],sizeof held_audio);
            else CHECK(!memcmp(held_audio,takes[side],sizeof held_audio));
            begin_pair(owner,57); render(4);
            v=held_voice(e);
            CHECK(v && v->ea.mode==S1U_E_DEC);
            pole[side]=v->ea.a;
            take(side);
            envelope[side]=v->ea.y; filter_env[side]=v->ef.y;
            if(audible) CHECK(v->ea.mode==S1U_E_DEC && v->ea.y>e->t[28]);
            for(unsigned k=0;k<2;++k) { phase[side][k]=e->lfo[k].ph; shapes[side][k]=e->lfo[k].sm; }
        }
        CHECK(!memcmp(phase[0],phase[1],sizeof phase[0]) && !memcmp(shapes[0],shapes[1],sizeof shapes[0]));
        CHECK(e->t[14]==d1 && e->t[46]==d2 && e->t[15]==r1 && e->t[47]==r2 && e->t[27]==base && e->t[16]==cutoff && e->t[12]==noise);
        CHECK(read_slot(1-owner,id).physical==peer && read_slot(owner,39).physical==old_hp && read_slot(owner,121).physical==1.f);
        CHECK(!memcmp(takes[0][1-owner],takes[1][1-owner],sizeof takes[0][0]));
        unsigned nchanged=0;
        for(unsigned n=0;n<FRAMES;++n) nchanged+=memcmp(takes[0][owner][n],takes[1][owner][n],sizeof takes[0][owner][n])!=0;
        CHECK(audible ? nchanged>100 && envelope[1]<envelope[0] && pole[1]<pole[0] : !nchanged && envelope[1]==envelope[0] && pole[1]==pole[0]);
        CHECK(filter_env[1]==filter_env[0] && read_slot(owner,54).physical==2.f);
        CHECK(read_slot(owner,48).physical==3.f && read_slot(owner,53).physical==2.f && read_slot(owner,50).physical==2.f && !memcmp(old_tail,proj.sound.v[T_303][S1_EXTRA_ROW+owner],sizeof old_tail));
        if(mode==1) memcpy(prior_lfo1,takes[1][owner],sizeof prior_lfo1);
        if(mode==2) { if(audible) CHECK(memcmp(prior_lfo1,takes[1][owner],sizeof prior_lfo1)); memcpy(prior_lfo2,takes[1][owner],sizeof prior_lfo2); }
        if(mode==3 && audible) {
            CHECK(memcmp(prior_lfo1,takes[1][owner],sizeof prior_lfo1));
            CHECK(memcmp(prior_lfo2,takes[1][owner],sizeof prior_lfo2));
        }
        printf("owner %u AMP decay audible %u route %u: %u/%u fresh frames changed; held PCM, cutoff/filter target, noise, peer and phase exact; poles %g -> %g\n", owner,audible,mode,nchanged,FRAMES,pole[0],pole[1]);
    }
    stabilize(); CHECK(!project_save() && !project_load());
    CHECK(read_slot(owner,id).physical==3.f && read_slot(owner,39).physical==old_hp && read_slot(owner,121).physical==1.f);
    if(!audible) return;
    stored_project("decay-v2-sound",owner,PROJECT_EXPORT);
    capture_lock(owner);
    CHECK(proj.arr.lane[0].val[30]==23 && (proj.arr.lane[0].val[24]&128u));
    choose(owner,preset); CHECK(read_slot(owner,id).physical==init.physical);
    stabilize(); CHECK(!project_save() && !project_load());
    stored_project("decay-v2-lock",owner,PROJECT_EXPORT);
    lock_roundtrip(owner,3.f,init.physical);
    CHECK(engine_s1_patch_set(owner,read_slot(owner,48).revision,48,3)==S1_EDIT_QUEUED); render(1);
    CHECK(engine_s1_patch_set(owner,read_slot(owner,39).revision,39,12)==S1_EDIT_QUEUED); render(1);
    for(unsigned noise=0;noise<4;++noise) for(unsigned mix=0;mix<4;++mix) for(unsigned res=0;res<4;++res) {
        CHECK(engine_s1_patch_set(owner,read_slot(owner,id).revision,id,res)==S1_EDIT_QUEUED); render(1);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,53).revision,53,noise)==S1_EDIT_QUEUED); render(1);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,50).revision,50,mix)==S1_EDIT_QUEUED); render(1);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,49).revision,49,res)==S1_EDIT_QUEUED); render(1);
        capture_lock(owner);
        CHECK(!memcmp(proj.arr.lane[0].val+BASS_NOTE_USED,proj.sound.v[T_303][S1_EXTRA_ROW+owner],15));
        CHECK(proj.arr.lane[0].val[25]==128u+res*25u+(noise+1u)*5u+mix+1u);
        CHECK(proj.arr.lane[0].val[30]==5u*(res+1u));
    }

}
#endif

int main(void)
{
    historical();
    const char *path=getenv("S1_EDITOR_TEST_PACK"); CHECK(path);
    FILE *file=fopen(path,"rb"); CHECK(file);
    unsigned bytes=(unsigned)fread(pack,1,sizeof pack,file);
    CHECK(fgetc(file)==EOF && !fclose(file) && s1_part_set_pack(pack,bytes)==694);
    historical_owner();
#ifdef S1_WET_LEGACY
    for(unsigned key=0;key<2;++key) {
        setup(V_PART,PART_303A+key);
        stored_project("decay-v1-fm",key,PROJECT_MOUNT);
        CHECK(!project_load() && !project_storage_unsupported());
        stabilize();
        CHECK(read_slot(key,54).physical==2.f && read_slot(key,49).physical==2.f && read_slot(key,39).physical>80.f && read_slot(key,121).physical==1.f && read_slot(key,48).physical==3.f && read_slot(key,53).physical==2.f && read_slot(key,50).physical==2.f);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        const char *const records[]={"decay-v2-sound","decay-v2-lock"};
        for(unsigned i=0;i<2;++i) {
            stored_project(records[i],key,PROJECT_MOUNT);
            const unsigned writes=store_writes;
            CHECK(project_load()==-9 && project_storage_unsupported());
            CHECK(project_save()==-9 && store_writes==writes);
            stored_project(records[i],key,PROJECT_VERIFY);
        }
    }
    printf("PASS old FM/Noise/Osc/Resonance/cutoff/HP/WIDEN and lock accepted; AMP Decay route Sound/lock types rejected before writes (%u checks)\n",checks);
#elif defined(S1_WET_RED)
    setup(V_PART,PART_303A); choose(0,0);
    s1_patch_value missing; CHECK(engine_s1_patch_read(0,AMP_DECAY_LFO,&missing));
#else
    codec();
    for(unsigned audible=0;audible<2;++audible) {
        const unsigned preset=program_for(audible);
        for(unsigned owner=0;owner<NBASS;++owner) route_journey(owner,preset,audible);
    }
    printf("PASS held AMP Decay routes across both owners and Sound/lock protections (%u checks)\n",checks);
#endif
    return 0;
}
