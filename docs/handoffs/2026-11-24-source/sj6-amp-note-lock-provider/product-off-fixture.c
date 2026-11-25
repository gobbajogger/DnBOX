/* SPDX-License-Identifier: GPL-3.0-only */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
#include "sj6_note_amp.h"

static project_t preserved;
static uint32_t prng=0x12345678;
static uint16_t q16(void)
{
    prng ^= prng << 13; prng ^= prng >> 17; prng ^= prng << 5;
    return (uint16_t)prng;
}
static void format_lane(lane_t *n, unsigned i)
{
    const sj6_note_amp_t amp={{q16(),q16(),q16(),q16()},15,(uint8_t)(1+i%2)};
    EXPECT(sj6_note_amp_encode(n->val+BASS_NOTE_TAG,&amp,(unsigned)!!original_sound_is_edited(n->val)));
    n->i=SJ6_NOTE_AMP_VERSION;
}

int main(void)
{
    const int is_new=!!getenv("SJ6_NOTE_V2");
    EXPECT(sizeof(lane_t)==72 && sizeof(((lane_t*)0)->val)==64);
    EXPECT(BASS_NOTE_TAG==32 && BASS_NOTE_MIX==44 && BASS_NOTE_USED==49);
    setup(V_SEQ,PART_303A);
    proj.set.bt |= PROJ_BT_CLK_IN;
    for (unsigned v=0;v<NBASS;++v) {
        EXPECT(!engine_bass_select_preset(v,ORIGINAL_SOUND_JUNO,0,&proj.sound));
        for(int i=0;i<1024&&engine_bass_preset_pending(v);++i)render(1);
        EXPECT(engine_bass_bank(v)==ORIGINAL_SOUND_JUNO);
    }
    engine_note_lock_capture(1);
    for(unsigned n=0;n<NLANE;++n)
        EXPECT(!engine_bass_note_capture(proj.arr.lane,n/128, (n/64)%2,n%64,&proj.sound));
    engine_note_lock_capture(0);
    uint8_t header[32];
    project_stage=proj;
    preserved=proj;
    project_file_header(header);
    EXPECT(!project_file_validate(header));
    EXPECT(!memcmp(&project_stage,&preserved,sizeof preserved));
    EXPECT(!project_save()); /* Valid prior project metadata in the host memory store. */
    unsigned v1=0,v2=0;
    EXPECT(!measure_project(&proj,&v1));
    for(unsigned i=0;i<NLANE;++i){
        lane_t n=proj.arr.lane[i];
        EXPECT(n.used && !engine_bass_note_validate(&n));
        n.i=SJ6_NOTE_AMP_VERSION;
        EXPECT(engine_bass_note_validate(&n));
        format_lane(&proj.arr.lane[i],i);
        lane_t *l=&proj.arr.lane[i];
        EXPECT(!memcmp(n.val,l->val,32) && !memcmp(n.val+44,l->val+44,20));
        EXPECT((!engine_bass_note_validate(l))==is_new);
        lane_t mislabeled=*l; mislabeled.i=BASS_NOTE_VERSION;
        EXPECT(engine_bass_note_validate(&mislabeled));
    }
    EXPECT(!measure_project(&proj,&v2));
    printf("DENSE %u slots; same-geometry packed v1=%u v2=%u cap=%u\n",NLANE,v1,v2,(unsigned)STREAM_CAP);
    project_stage=proj;
    preserved=proj;
    project_file_header(header);
    const uint32_t before_file = store_writes;
    EXPECT(project_file_validate(header)==(is_new?0:-9));
    EXPECT(store_writes==before_file);
    EXPECT(!memcmp(&project_stage,&preserved,sizeof preserved));
    if(!is_new) {
        EXPECT(project_save()==-9);
        EXPECT(store_writes==before_file);
        /* Inject known v2-only stream into emulated storage, bypassing the old
         * writer solely for this fixture. No physical or external storage. */
        packed_len=v2;
        EXPECT(!save_stream());
        for(unsigned i=0;i<NSTREAM;++i) store_previous_len[STREAM_OBJ[i]]=-1;
        const uint32_t before_mount=store_writes;
        EXPECT(project_load()==-9);
        EXPECT(project_storage_unsupported());
        EXPECT(project_save()==-9);
        EXPECT(store_writes==before_mount);
    } else {
        EXPECT(!project_save());
        EXPECT(!project_load());
        EXPECT(!memcmp(proj.arr.lane,preserved.arr.lane,sizeof proj.arr.lane));
        for(unsigned i=0;i<NLANE;++i){
            const lane_t good=proj.arr.lane[i];
            lane_t bad=good; bad.val[41]^=1u;
            EXPECT(engine_bass_note_validate(&bad));
            bad=good;bad.val[42]=1;
            EXPECT(engine_bass_note_validate(&bad));
            bad=good;bad.val[29]=ORIGINAL_SOUND_SYNTHONE;
            EXPECT(engine_bass_note_validate(&bad));
            bad=good;bad.val[44]=255;
            EXPECT(engine_bass_note_validate(&bad));
            bad=good;bad.val[63]=1;
            EXPECT(engine_bass_note_validate(&bad));
        }
    }
    printf("%s %s reader: %u checks, %u failures\n",failures?"FAIL":"PASS",is_new?"v2":"prior",expectations,failures);
    return !!failures;
}
