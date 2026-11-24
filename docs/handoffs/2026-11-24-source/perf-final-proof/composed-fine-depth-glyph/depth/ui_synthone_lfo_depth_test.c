/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
#include "../../firmware/src/app/s1c_part.h"

void host_s1_retrigger(const s1c *s) { (void)s; }
enum { DEPTH1 = 14, DEPTH2 = 46 };
static const unsigned ids[] = {DEPTH1, DEPTH2};
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


static unsigned program_for(unsigned layer, unsigned routed)
{
    static s1_dry_part part;
    for (unsigned index = 0; index < 694; ++index) {
        CHECK(s1_part_program(&part,index));
        const float *t = part.e.t;
        unsigned hits = 0;
        for (unsigned i=48;i<=59;++i) hits += t[i] == (float)(layer+1u) || t[i] == 3.f;
        if ((!routed && !hits) || (routed && t[48]==(float)(layer+1u) && t[16]<12000.f)) return index+1u;
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
    uint8_t cell[ORIGINAL_SOUND_BYTES], keep[ORIGINAL_SOUND_BYTES], extra[S1_EXTRA_BYTES]={0}, old_extra[S1_EXTRA_BYTES];
    CHECK(original_sound_init(cell,ORIGINAL_SOUND_SYNTHONE,0));
    CHECK(original_sound_s1_set(cell,S1_PART_LEVEL,23));
    CHECK(original_sound_s1_extra_set(cell,extra,S1_REVERB_MIX,5));
    CHECK(original_sound_s1_extra_set(cell,extra,S1_REVERB_HP,87));
    CHECK(original_sound_s1_extra_set(cell,extra,S1_WIDEN,2));
    CHECK(original_sound_s1_extra_set(cell,extra,S1_MONO_LEGATO,1));
    memcpy(old_extra,extra,sizeof extra); memcpy(keep,cell,sizeof keep);
    for(unsigned i=0;i<=127;++i) {
      CHECK(original_sound_s1_set(cell,S1_LFO1_DEPTH,i));
      CHECK(original_sound_s1_set(cell,S1_LFO2_DEPTH,127-i));
      CHECK(original_sound_valid(cell));
      CHECK(cell[3]==23 && cell[4]==i && cell[5]==127-i && (original_sound_edit_mask(cell)&0x38u)==0x38u);
      for(unsigned k=0;k<sizeof cell;++k) if(k!=4 && k!=5 && k!=22) CHECK(cell[k]==keep[k]);
      CHECK(!memcmp(extra,old_extra,sizeof extra));
    }
    memcpy(keep,cell,sizeof keep);
    CHECK(!original_sound_s1_set(cell,S1_LFO1_DEPTH,128) && !memcmp(keep,cell,sizeof keep));
    cell[5]=128; CHECK(!original_sound_valid(cell));
}

static void lock_roundtrip(unsigned owner, unsigned layer, float authored, float factory)
{
    const unsigned id=ids[layer];
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
    CHECK(engine_s1_patch_set(owner,read_slot(owner,id).revision,id,76)==S1_EDIT_BUSY);
    CHECK(!memcmp(&before,&proj.sound,sizeof before) && !memcmp(&lane,&proj.arr.lane[0],sizeof lane));
    for(unsigned n=0;n<2000 && read_slot(owner,id).note_locked;++n) render(1);
    CHECK(!read_slot(owner,id).note_locked && read_slot(owner,id).physical==factory);
    seq_stop(&seq); render(4); seq_start(&seq);
    for(unsigned n=0;n<2000 && !read_slot(owner,id).note_locked;++n) render(1);
    CHECK(read_slot(owner,id).note_locked && read_slot(owner,id).physical==authored);
    seq_stop(&seq); render(4);
    CHECK(!read_slot(owner,id).note_locked && read_slot(owner,id).physical==factory);
}

static void depth_journey(unsigned owner,unsigned layer,unsigned routed,unsigned preset)
{
    const unsigned id=ids[layer], other=ids[1-layer], col=1+layer;
    setup(V_PART,PART_303A+owner); memset(proj.arr.lane,0,sizeof proj.arr.lane);
    proj.set.bt|=PROJ_BT_CLK_IN;
    choose(owner,preset); choose(1-owner,0);
    CHECK(pg.n==14);
    visit("TEMPO SYNC");
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][0])->name,"SYNC TO ARP"));
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][col])->name,layer ? "LFO 2 DEPTH" : "LFO 1 DEPTH"));
    CHECK(pg.r[cur_page()][3].kind == R_NONE);
    const s1_patch_value init=read_slot(owner,id);
    const float other_default=read_slot(owner,other).physical;
    const float peer=read_slot(1-owner,id).physical;
    const s1c *e=host_s1_owner(owner);
    const unsigned rhythm_id=layer ? 47 : 15, shape_id=layer ? 45 : 13;
    const float rhythm=e->t[rhythm_id], shape=e->t[shape_id];
    int direction=init.position<127 ? 1 : -1;
    begin_pair(owner,57); detent(col,direction);
    const float adjacent=read_slot(owner,id).physical;
    CHECK(fabsf(adjacent-init.physical) < .012f);
    CHECK(direction>0 ? adjacent>init.physical : adjacent<init.physical);
    engine_bass_off(owner); engine_bass_off(1-owner);
    choose(owner,preset); visit("TEMPO SYNC");
    CHECK(read_slot(owner,id).physical == init.physical);

    /* Preserve an old Part Level semantic and byte image in the same owner. */
    CHECK(engine_s1_patch_set(owner,read_slot(owner,32).revision,32,64)==S1_EDIT_QUEUED);
    render(1);
    const float level=read_slot(owner,32).physical;
    if (routed) {
        capture_lock(owner); stabilize();
        CHECK(!project_save() && !project_load());
        stored_project("depth-v1-level",owner*2+layer,PROJECT_EXPORT);
    }

    float phase[2], other_phase[2], coefficient[2];
    const unsigned new_on = init.physical < .5f;
    for(unsigned side=0;side<2;++side) {
        begin_pair(owner,57); render(48);
        detent(col,side ? (new_on ? 127 : -127) : 0);
        take(side);
        phase[side]=e->lfo[layer].ph; other_phase[side]=e->lfo[1-layer].ph;
        coefficient[side]=e->lfo[layer].o01;
    }
    CHECK(phase[0]==phase[1] && other_phase[0]==other_phase[1]);
    CHECK(e->t[rhythm_id]==rhythm && e->t[shape_id]==shape);
    CHECK(read_slot(owner,id).physical==(float)new_on && read_slot(owner,other).physical==other_default);
    CHECK(read_slot(1-owner,id).physical==peer && read_slot(owner,32).physical==level);
    CHECK(new_on ? coefficient[1]>=coefficient[0] : coefficient[1]<=coefficient[0]);
    CHECK(!memcmp(takes[0][1-owner],takes[1][1-owner],sizeof takes[0][0]));
    unsigned changes=0;
    for(unsigned n=0;n<FRAMES;++n) changes+=memcmp(takes[0][owner][n],takes[1][owner][n],sizeof takes[0][owner][n])!=0;
    CHECK(routed ? changes>100 : !changes);
    printf("owner %u LFO%u %s: adjacent %.8f -> %.8f, %u/%u affected, peer/clocks exact\n",
           owner,layer+1,routed?"routed":"unrouted",init.physical,adjacent,changes,FRAMES);
    stabilize();
    CHECK(!project_save() && !project_load());
    CHECK(read_slot(owner,id).physical==(float)new_on && read_slot(owner,32).physical==level);
    if (!routed) return;
    stored_project("depth-v2-sound",owner*2+layer,PROJECT_EXPORT);
    capture_lock(owner);
    lane_t bad=proj.arr.lane[0]; bad.val[4+layer]=128;
    CHECK(engine_bass_note_validate(&bad));
    choose(owner,preset);
    CHECK(read_slot(owner,id).physical == init.physical);
    stabilize(); CHECK(!project_save() && !project_load());
    stored_project("depth-v2-lock",owner*2+layer,PROJECT_EXPORT);
    lock_roundtrip(owner,layer,(float)new_on,init.physical);
}
#endif

int main(void)
{
    const char *path=getenv("S1_EDITOR_TEST_PACK"); CHECK(path);
    FILE *file=fopen(path,"rb"); CHECK(file);
    unsigned bytes=(unsigned)fread(pack,1,sizeof pack,file);
    CHECK(fgetc(file)==EOF && !fclose(file) && s1_part_set_pack(pack,bytes)==694);
#ifdef S1_WET_LEGACY
    for(unsigned key=0;key<4;++key) {
        setup(V_PART,PART_303A+(key/2));
        stored_project("depth-v1-level",key,PROJECT_MOUNT);
        CHECK(!project_load() && !project_storage_unsupported());
        stabilize();
        CHECK(fabsf(read_slot(key/2,32).physical-128.f/127.f)<.00001f);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        const char *const records[]={"depth-v2-sound","depth-v2-lock"};
        for(unsigned i=0;i<2;++i) {
            stored_project(records[i],key,PROJECT_MOUNT);
            const unsigned writes=store_writes;
            CHECK(project_load()==-9 && project_storage_unsupported());
            CHECK(project_save()==-9 && store_writes==writes);
            stored_project(records[i],key,PROJECT_VERIFY);
        }
    }
    printf("PASS old Level and lock accepted; both LFO depth Sound/lock types rejected before writes (%u checks)\n",checks);
#elif defined(S1_WET_RED)
    setup(V_PART,PART_303A); choose(0,0);
    s1_patch_value missing; CHECK(engine_s1_patch_read(0,DEPTH1,&missing));
#else
    codec();
    for(unsigned layer=0;layer<2;++layer)
        for(unsigned routed=0;routed<2;++routed) {
            const unsigned preset=program_for(layer,routed);
            for(unsigned owner=0;owner<NBASS;++owner) depth_journey(owner,layer,routed,preset);
        }
    printf("PASS both physical LFO depths across two owners, project/lock safety (%u checks)\n",checks);
#endif
    return 0;
}
