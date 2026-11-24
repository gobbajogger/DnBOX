/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
#include "../../firmware/src/app/s1c_part.h"

void host_s1_retrigger(const s1c *s) { (void)s; }
enum { NOISE_LFO = 53 };
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
    for (unsigned index = 0; index < 694; ++index) {
        CHECK(s1_part_program(&part,index));
        const float *t = part.e.t;
        if (t[53] == 0.f && t[14]>=.35f && t[46]>=.35f &&
            (audible ? t[12] > .06f : t[12] == 0.f)) return index+1u;
    }
    CHECK(0 && "authentic v9 program with known oscillator routing");
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
static void codec(void)
{
    uint8_t cell[32], stable[32], extra[15]={0}, prior_extra[15];
    CHECK(original_sound_init(cell,ORIGINAL_SOUND_SYNTHONE,0));
    CHECK(original_sound_s1_set(cell,S1_PART_LEVEL,21));
    CHECK(original_sound_s1_set(cell,S1_LFO1_DEPTH,75));
    CHECK(original_sound_s1_set(cell,S1_LFO2_DEPTH,9));
    CHECK(original_sound_s1_extra_set(cell,extra,S1_REVERB_HP,87));
    CHECK(original_sound_s1_extra_set(cell,extra,S1_WIDEN,2));
    CHECK(original_sound_s1_extra_set(cell,extra,S1_CUTOFF_LFO,4));
    CHECK(original_sound_s1_extra_set(cell,extra,S1_MONO_LEGATO,1));
    memcpy(stable,cell,sizeof stable);
    memcpy(prior_extra,extra,sizeof prior_extra);
    for(unsigned pos=0;pos<4;++pos) {
        CHECK(original_sound_s1_set(cell,S1_NOISE_LFO,pos));
        CHECK(original_sound_valid(cell));
        CHECK((cell[25]&7u)==pos+1u);
        CHECK(!memcmp(extra,prior_extra,sizeof extra));
        for(unsigned i=0;i<sizeof cell;++i) if(i!=25) CHECK(cell[i]==stable[i]);
    }
    for(unsigned code=0;code<8;++code) {
        cell[25]=(uint8_t)code;
        CHECK(!!original_sound_valid(cell)==(code<=4u));
    }
    cell[25]=4; memcpy(stable,cell,sizeof stable);
    CHECK(!original_sound_s1_set(cell,S1_NOISE_LFO,4));
    CHECK(!memcmp(stable,cell,sizeof stable));
    puts("PASS exact prior base + packed cutoff/HP/Widen/Legato; Noise codes 0–4 accepted, reserved 5–7 rejected");
}

static void lock_roundtrip(unsigned owner, float authored, float factory)
{
    const unsigned id=NOISE_LFO;
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

static void route_journey(unsigned owner,unsigned preset,unsigned audible)
{
    const unsigned id=NOISE_LFO, col=3;
    setup(V_PART,PART_303A+owner); memset(proj.arr.lane,0,sizeof proj.arr.lane);
    proj.set.bt|=PROJ_BT_CLK_IN;
    choose(owner,preset); choose(1-owner,0);
    CHECK(pg.n==14); visit("FM / NOISE");
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][col])->name,"NOISE LFO"));
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][1])->name,"FM INDEX"));
    CHECK(pg.r[cur_page()][2].kind==R_S1PATCH);
    const s1_patch_value init=read_slot(owner,id);
    CHECK(init.physical==0.f && init.max_position==3);
    const float peer=read_slot(1-owner,id).physical;
    const s1c *e=host_s1_owner(owner);
    const float d1=e->t[14], d2=e->t[46], r1=e->t[15], r2=e->t[47], base=e->t[16];
    /* Prior HP and Widen stay live, including in old Sound/note payloads. */
    CHECK(engine_s1_patch_set(owner,read_slot(owner,39).revision,39,12)==S1_EDIT_QUEUED); render(1);
    CHECK(engine_s1_patch_set(owner,read_slot(owner,121).revision,121,1)==S1_EDIT_QUEUED); render(1);
    const float old_hp=read_slot(owner,39).physical;
    CHECK(engine_s1_patch_set(owner,read_slot(owner,48).revision,48,3)==S1_EDIT_QUEUED); render(1);
    uint8_t old_tail[15];
    memcpy(old_tail,proj.sound.v[T_303][S1_EXTRA_ROW+owner],sizeof old_tail);
    if(audible) {
        capture_lock(owner); stabilize(); CHECK(!project_save() && !project_load());
        stored_project("noise-v1-cutoff",owner,PROJECT_EXPORT);
    }

    float phase[2][2], shapes[2][2];
    float prior_lfo1[FRAMES][2], prior_lfo2[FRAMES][2];
    sound_t oldsound=proj.sound;
    for(unsigned mode=1;mode<=3;++mode) {
        proj.sound=oldsound;
        for(unsigned side=0;side<2;++side) {
            begin_pair(owner,57); render(48);
            for(unsigned step=0;step<3;++step) {
                detent(col,side && step<mode ? 1 : 0);
                CHECK(read_slot(owner,id).physical==(float)(side ? (step<mode ? step+1 : mode) : 0));
            }
            take(side);
            for(unsigned k=0;k<2;++k) { phase[side][k]=e->lfo[k].ph; shapes[side][k]=e->lfo[k].sm; }
        }
        CHECK(!memcmp(phase[0],phase[1],sizeof phase[0]) && !memcmp(shapes[0],shapes[1],sizeof shapes[0]));
        CHECK(e->t[14]==d1 && e->t[46]==d2 && e->t[15]==r1 && e->t[47]==r2 && e->t[16]==base);
        CHECK(read_slot(1-owner,id).physical==peer && read_slot(owner,39).physical==old_hp && read_slot(owner,121).physical==1.f);
        CHECK(!memcmp(takes[0][1-owner],takes[1][1-owner],sizeof takes[0][0]));
        unsigned nchanged=0;
        for(unsigned n=0;n<FRAMES;++n) nchanged+=memcmp(takes[0][owner][n],takes[1][owner][n],sizeof takes[0][owner][n])!=0;
        CHECK(audible ? nchanged>100 : !nchanged);
        CHECK(read_slot(owner,48).physical==3.f && !memcmp(old_tail,proj.sound.v[T_303][S1_EXTRA_ROW+owner],sizeof old_tail));
        if(mode==1) memcpy(prior_lfo1,takes[1][owner],sizeof prior_lfo1);
        if(mode==2) { if(audible) CHECK(memcmp(prior_lfo1,takes[1][owner],sizeof prior_lfo1)); memcpy(prior_lfo2,takes[1][owner],sizeof prior_lfo2); }
        if(mode==3 && audible) {
            CHECK(memcmp(prior_lfo1,takes[1][owner],sizeof prior_lfo1));
            CHECK(memcmp(prior_lfo2,takes[1][owner],sizeof prior_lfo2));
        }
        printf("owner %u noise destination %u: %u/%u changed, target/peer/phase exact\n", owner,mode,nchanged,FRAMES);
    }
    stabilize(); CHECK(!project_save() && !project_load());
    CHECK(read_slot(owner,id).physical==3.f && read_slot(owner,39).physical==old_hp && read_slot(owner,121).physical==1.f);
    if(!audible) return;
    stored_project("noise-v2-sound",owner,PROJECT_EXPORT);
    capture_lock(owner);
    choose(owner,preset); CHECK(read_slot(owner,id).physical==init.physical);
    stabilize(); CHECK(!project_save() && !project_load());
    stored_project("noise-v2-lock",owner,PROJECT_EXPORT);
    lock_roundtrip(owner,3.f,init.physical);
}
#endif

int main(void)
{
    const char *path=getenv("S1_EDITOR_TEST_PACK"); CHECK(path);
    FILE *file=fopen(path,"rb"); CHECK(file);
    unsigned bytes=(unsigned)fread(pack,1,sizeof pack,file);
    CHECK(fgetc(file)==EOF && !fclose(file) && s1_part_set_pack(pack,bytes)==694);
#ifdef S1_WET_LEGACY
    for(unsigned key=0;key<2;++key) {
        setup(V_PART,PART_303A+key);
        stored_project("noise-v1-cutoff",key,PROJECT_MOUNT);
        CHECK(!project_load() && !project_storage_unsupported());
        stabilize();
        CHECK(read_slot(key,39).physical>80.f && read_slot(key,121).physical==1.f && read_slot(key,48).physical==3.f);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        const char *const records[]={"noise-v2-sound","noise-v2-lock"};
        for(unsigned i=0;i<2;++i) {
            stored_project(records[i],key,PROJECT_MOUNT);
            const unsigned writes=store_writes;
            CHECK(project_load()==-9 && project_storage_unsupported());
            CHECK(project_save()==-9 && store_writes==writes);
            stored_project(records[i],key,PROJECT_VERIFY);
        }
    }
    printf("PASS old cutoff/HP/WIDEN and lock accepted; Noise route Sound/lock types rejected before writes (%u checks)\n",checks);
#elif defined(S1_WET_RED)
    setup(V_PART,PART_303A); choose(0,0);
    s1_patch_value missing; CHECK(engine_s1_patch_read(0,NOISE_LFO,&missing));
#else
    codec();
    for(unsigned audible=0;audible<2;++audible) {
        const unsigned preset=program_for(audible);
        for(unsigned owner=0;owner<NBASS;++owner) route_journey(owner,preset,audible);
    }
    printf("PASS held Noise routes across both owners and Sound/lock protections (%u checks)\n",checks);
#endif
    return 0;
}
