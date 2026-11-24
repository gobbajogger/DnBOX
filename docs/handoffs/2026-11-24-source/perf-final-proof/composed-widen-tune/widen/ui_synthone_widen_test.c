/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
#include "../../firmware/src/app/s1c_part.h"

enum { REVERB_HP = 39, WIDEN = 121 };
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

static unsigned program_for(unsigned enabled)
{
    static s1_dry_part part;
    for (unsigned index = 0; index < 694; ++index) {
        CHECK(s1_part_program(&part, index));
        float delay = part.e.t[S1C_P_DLON], verb = part.e.t[S1C_P_RVON];
        if (delay == (float)enabled && verb == (float)enabled &&
            (part.e.t[S1C_P_WIDEN] == (float)enabled)) return index+1;
    }
    CHECK(0 && "authentic factory case with matching wet enables");
    return 0;
}

static void play_lock(unsigned owner, const float *locked, const float *base)
{
    bpart_t *line = &proj.pat[0].bass[owner];
    for (unsigned i = 0; i < NSTEPS; ++i) line->step[i].flags = G_REST;
    line->len = 16; line->rate = RATE_16; line->dir = DIR_FWD; line->transpose = 24;
    line->step[0].flags = line->step[4].flags = G_NOTE;
    line->step[0].note = 36; line->step[4].note = 43;
    line->step[0].velocity = line->step[4].velocity = 100;
    seq.ppat[TRK_BASS0+owner] = 0;
    engine_note_lock_refresh();
    seq_start(&seq);
    for (unsigned n = 0; n < 2000 && !read_slot(owner, WIDEN).note_locked; ++n) render(1);
    CHECK(read_slot(owner, WIDEN).physical == *locked && read_slot(owner, WIDEN).note_locked);
    const sound_t sound = proj.sound;
    const lane_t lock = proj.arr.lane[0];
    CHECK(engine_s1_patch_set(owner, read_slot(owner, WIDEN).revision, WIDEN, (unsigned)!*locked) == S1_EDIT_BUSY);
    CHECK(!memcmp(&sound, &proj.sound, sizeof sound));
    CHECK(!memcmp(&lock, &proj.arr.lane[0], sizeof lock));
    for (unsigned n = 0; n < 2000 && read_slot(owner, WIDEN).note_locked; ++n) render(1);
    CHECK(read_slot(owner, WIDEN).physical == *base && !read_slot(owner, WIDEN).note_locked);
    seq_stop(&seq);
    render(4);
    seq_start(&seq);
    for (unsigned n = 0; n < 2000 && !read_slot(owner, WIDEN).note_locked; ++n) render(1);
    CHECK(read_slot(owner, WIDEN).physical == *locked && read_slot(owner, WIDEN).note_locked);
    seq_stop(&seq);
    render(4);
    CHECK(read_slot(owner, WIDEN).physical == *base && !read_slot(owner, WIDEN).note_locked);
}



#if !defined(S1_WET_RED) && !defined(S1_WET_LEGACY)
static void codec(void)
{
    uint8_t cell[ORIGINAL_SOUND_BYTES], bytes[S1_EXTRA_BYTES] = {0};
    CHECK(original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, 0));
    for (unsigned slot = S1_EXTRA_FIRST; slot < S1_LFO_FIRST; ++slot)
        CHECK(original_sound_s1_extra_set(cell,bytes,slot,s1_patch_steps(slot)));
    for (unsigned slot = S1_LFO_FIRST; slot < S1_LFO_SYNC; ++slot)
        CHECK(original_sound_s1_extra_set(cell,bytes,slot,s1_lfo_rate_slot(slot) ? S1_LFO_RATE_CODES-1u : 4u));
    CHECK(original_sound_s1_sync_set(cell,bytes,2,(const uint8_t[]){0,0}));
    CHECK(original_sound_s1_extra_set(cell,bytes,S1_DELAY_MIX,1));
    CHECK(original_sound_s1_extra_set(cell,bytes,S1_REVERB_MIX,128));
    CHECK(original_sound_s1_extra_set(cell,bytes,S1_REVERB_HP,97));
    CHECK(bytes[0] == 97 && bytes[1] == 0xf8);
    s1_extra_state prior, state;
    CHECK(s1_extra_read(bytes,&prior) && prior.wet[3] == 0);
    for (unsigned w=0;w<=1;++w) {
        CHECK(original_sound_s1_extra_set(cell,bytes,S1_WIDEN,w+1));
        CHECK(bytes[0] == 97 && bytes[1] == 0xf9+w);
        CHECK(s1_extra_read(bytes,&state) && state.wet[3] == w+1);
        state.wet[3] = 0;
        CHECK(!memcmp(&state,&prior,sizeof prior));
    }
    for(unsigned k=3;k<8;++k) {
        bytes[1]=(uint8_t)(0xf8u | k);
        CHECK(!s1_extra_read(bytes,&state) && !memcmp(&state,&prior,sizeof prior));
    }
}

static void capture_lock(unsigned owner)
{
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane,0,owner,0,&proj.sound));
    engine_note_lock_capture(0);
    CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
}

static void widen_journey(unsigned owner, unsigned wet, unsigned preset)
{
    setup(V_PART,PART_303A+owner);
    memset(proj.arr.lane,0,sizeof proj.arr.lane);
    proj.set.bt |= PROJ_BT_CLK_IN;
    choose(owner,preset); choose(1-owner,0);
    visit("OUTPUT");
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][0])->name,"WIDEN"));
    for(unsigned k=1;k<4;++k) CHECK(pg.r[cur_page()][k].kind == R_NONE);
    const s1_patch_value init = read_slot(owner,WIDEN);
    CHECK(init.max_position == 1 && init.position <= 1);
    const unsigned edit = !init.position;
    const float peer = read_slot(1-owner,WIDEN).physical;

    /* Existing live HP uses the prior escape byte and remains unchanged. */
    CHECK(engine_s1_patch_set(owner,read_slot(owner,REVERB_HP).revision,REVERB_HP,0) == S1_EDIT_QUEUED);
    render(1);
    CHECK(read_slot(owner,REVERB_HP).physical == 80.f);
    if(wet) {
        capture_lock(owner);
        stabilize();
        CHECK(!project_save() && !project_load());
        stored_project("widen-v1-hp",owner,PROJECT_EXPORT);
    }
    begin_pair(owner,57); render(48);
    float enables[2]; unsigned layout[5], again[5];
    host_s1_wet_peek(owner,enables,layout);
    CHECK(enables[0] == (float)wet && enables[1] == (float)wet);
    detent(0,0);
    take(0);
    begin_pair(owner,57); render(48);
    detent(0,edit ? 1 : -1);
    host_s1_wet_peek(owner,enables,again);
    CHECK(!memcmp(layout,again,sizeof layout));
    CHECK(enables[0] == (float)wet && enables[1] == (float)wet);
    take(1);
    host_s1_wet_peek(owner,enables,again);
    CHECK(!memcmp(layout,again,sizeof layout));
    CHECK(read_slot(owner,WIDEN).physical == (float)edit);
    CHECK(read_slot(1-owner,WIDEN).physical == peer);
    CHECK(read_slot(owner,REVERB_HP).physical == 80.f);
    CHECK(!memcmp(takes[0][1-owner],takes[1][1-owner],sizeof takes[0][0]));
    unsigned changed = 0;
    for(unsigned t=0;t<FRAMES;++t) {
        CHECK(!memcmp(&takes[0][owner][t][0],&takes[1][owner][t][0],sizeof(float)));
        changed += memcmp(&takes[0][owner][t][1],&takes[1][owner][t][1],sizeof(float)) != 0;
    }
    CHECK(changed > FRAMES/2);
    printf("owner %u wet effects %u: WIDEN factory %.6f -> %u, owner RIGHT %u/%u changed, LEFT and peer exact\n",
           owner,wet,init.physical,edit,changed,FRAMES);
    stabilize();
    CHECK(!project_save() && !project_load());
    CHECK(read_slot(owner,WIDEN).physical == (float)edit && read_slot(owner,REVERB_HP).physical == 80.f);
    if(!wet) return;
    stored_project("widen-v2-sound",owner,PROJECT_EXPORT);
    capture_lock(owner);
    choose(owner,preset);
    CHECK(read_slot(owner,WIDEN).physical == init.physical);
    stabilize();
    CHECK(!project_save() && !project_load());
    stored_project("widen-v2-lock",owner,PROJECT_EXPORT);
    const float locked = (float)edit;
    play_lock(owner,&locked,&init.physical);
}
#endif

int main(void)
{
    const char *path=getenv("S1_EDITOR_TEST_PACK");
    CHECK(path);
    FILE *file=fopen(path,"rb"); CHECK(file);
    unsigned bytes=(unsigned)fread(pack,1,sizeof pack,file);
    CHECK(fgetc(file)==EOF && !fclose(file) && s1_part_set_pack(pack,bytes)==694);
#ifdef S1_WET_LEGACY
    for(unsigned owner=0;owner<NBASS;++owner) {
        setup(V_PART,PART_303A+owner);
        stored_project("widen-v1-hp",owner,PROJECT_MOUNT);
        CHECK(!project_load() && !project_storage_unsupported());
        stabilize();
        CHECK(read_slot(owner,REVERB_HP).physical==80.f);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        const char *const names[]={"widen-v2-sound","widen-v2-lock"};
        for(unsigned i=0;i<2;++i) {
            stored_project(names[i],owner,PROJECT_MOUNT);
            const unsigned writes=store_writes;
            CHECK(project_load()==-9 && project_storage_unsupported());
            CHECK(project_save()==-9 && store_writes==writes);
            stored_project(names[i],owner,PROJECT_VERIFY);
        }
    }
    printf("PASS old HP Sound/lock accepted; Widen Sound/lock rejected before any writes (%u checks)\n",checks);
#elif defined(S1_WET_RED)
    setup(V_PART,PART_303A);
    choose(0,0);
    s1_patch_value absent;
    CHECK(engine_s1_patch_read(0,WIDEN,&absent));
#else
    codec();
    for(unsigned wet=0;wet<2;++wet) {
        const unsigned preset=program_for(wet);
        for(unsigned owner=0;owner<NBASS;++owner) widen_journey(owner,wet,preset);
    }
    printf("PASS physical part OUTPUT WIDEN and shared payload (%u checks)\n",checks);
#endif
    return 0;
}
