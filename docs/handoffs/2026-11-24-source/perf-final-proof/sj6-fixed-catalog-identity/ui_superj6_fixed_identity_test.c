/* SPDX-License-Identifier: GPL-3.0-only */
/* No product code changed. Real prior parser decides acceptance. */
#define PWM_FIXTURE_MAIN retained_pwm_main
#include "ui_superj6_pwm1_lfo2_test.c"
#include "fixed_catalog_synthetic.h"

static uint8_t current[OBJ_NOBJ][PLAT_STORE_MAX], previous[OBJ_NOBJ][PLAT_STORE_MAX];
static int lens[OBJ_NOBJ], prevlens[OBJ_NOBJ];
static project_t live;

static void codec(void)
{
    for(unsigned f=0;f<4;++f) {
        uint8_t c[32], untouched[32];
        CHECK(original_sound_init(c,ORIGINAL_SOUND_JUNO,f) && original_sound_valid(c));
        memcpy(untouched,c,sizeof c);
        unsigned n=999;
        CHECK(!pilot_sound_decode(c,&n,1) && n==999);
        CHECK(!pilot_sound_store(c,0,0) && !memcmp(c,untouched,sizeof c));
    }
    for(unsigned n=0;n<FIXED_N;++n) {
        uint8_t c[32], low[32], r[3]={0xc4,0xe4,0};
        unsigned k,p,expect=9999,peer=FIXED_N-1-n;
        CHECK(original_sound_init(c,ORIGINAL_SOUND_JUNO,0));
        uint8_t p_[SJ6_OSC_KNOBS]={0};
        p_[SJ6_PWM1]=71;p_[SJ6_PWM2]=13;
        original_sound_sj6_store(c,(1u<<SJ6_PWM1)|(1u<<SJ6_PWM2),p_);
        memcpy(low,c,sizeof c);
        CHECK(pilot_sound_store(c,n,1) && pilot_sound_decode(c,&expect,1) && expect==n);
        CHECK(!original_sound_valid(c) && !pilot_sound_decode(c,&expect,0));
        CHECK((c[18]&127u)==low[18] && (c[19]&127u)==low[19]);
        CHECK(pilot_pattern_store(r,0,n,1) && r[1]==0xe4);
        CHECK(pilot_pattern_store(r,1,peer,1));
        CHECK(pilot_pattern_read(r,0,&expect,1) && expect==n);
        CHECK(pilot_pattern_read(r,1,&expect,1) && expect==peer);
        CHECK(!pilot_pattern_valid(r,0));
        CHECK(!bass_pattern_preset_decode(r[0],&k,&p) && !bass_pattern_preset_decode(r[1],&k,&p));
    }
    uint8_t unknown[3]={0xe4,0xc2,0xa5}, snap[3];
    memcpy(snap,unknown,3);
    CHECK(pilot_pattern_valid(unknown,0));
    CHECK(!pilot_pattern_store(unknown,0,0,1) && !memcmp(snap,unknown,3));
    uint8_t no_provider[3]={0,0,0};
    CHECK(!pilot_pattern_store(no_provider,0,0,0) && !memcmp(no_provider,(uint8_t[3]){0},3));
    uint8_t malformed[3]={82u,0,0xaa};
    CHECK(!pilot_pattern_valid(malformed,1));
    uint8_t unowned_nibble[3]={82,0,0x10};
    CHECK(!pilot_pattern_valid(unowned_nibble,1));
}

static void reset_old(void)
{
    for(unsigned o=0;o<OBJ_NOBJ;++o)store_len[o]=store_previous_len[o]=-1;
    memset(stored_known,0,sizeof stored_known);
    setup(V_PART, PART_303A);
    proj.set.bt |= PROJ_BT_CLK_IN;
    select_owner(0,0); select_owner(1,0);
    proj.pat[0].rsv[0]=0xe1;
    proj.pat[0].rsv[1]=0xe4;
    proj.pat[0].rsv[2]=0xa5; /* old ignored byte must stay literally unchanged */
    project_stage=proj;
    CHECK(!project_validate_stage());
    project_commit_stage();
    int wr=project_save(), rd=wr?999:project_load();
    fprintf(stderr,"setup legacy save %d load %d flag %d\n",wr,rd,project_storage_unsupported());
    CHECK(!wr && !rd && !project_storage_unsupported());
    CHECK(proj.pat[0].rsv[0]==0xe1 && proj.pat[0].rsv[1]==0xe4 && proj.pat[0].rsv[2]==0xa5);
    memcpy(&live,&proj,sizeof live);
}

static void wire_reject(unsigned pat,unsigned high,unsigned stage)
{
    reset_old();
    if(pat) {
        /* Test-only stage future bytes with the genuine stream encoder and
         * actual plausible older A/B bank record retained. */
        const unsigned p=1;
        CHECK(pilot_pattern_store(proj.pat[p].rsv,0,high?639u:0u,1));
        CHECK(pilot_pattern_store(proj.pat[p].rsv,1,high?0u:639u,1));
        CHECK(!measure_project(&proj,&packed_len));
        CHECK(!save_stream());
        memcpy(&proj,&live,sizeof proj);
    } else {
        sound_t payload=proj.sound;
        const unsigned owner=high?1:0;
        unsigned n=high?508u:0u; /* n=508 yields cell[30]==0 plus high PWM marker */
        CHECK(pilot_sound_store(payload.v[T_303][owner],n,1));
        unsigned tagged=n+4u;
        payload.bass[owner].bytes[8]=(uint8_t)tagged;
        payload.bass[owner].bytes[9]=(uint8_t)(tagged>>8);
        CHECK(!save_obj(OBJ_SOUND,&payload,sizeof payload));
    }
    memcpy(current,store,sizeof current);memcpy(previous,store_previous,sizeof previous);
    memcpy(lens,store_len,sizeof lens);memcpy(prevlens,store_previous_len,sizeof prevlens);
    const uint32_t writes=store_writes;
    const int result=stage?project_load_stage():project_load();
    const int flag=project_storage_unsupported();
    CHECK(result<0 && flag);
    if(stage) CHECK(!memcmp(&live,&proj,sizeof live)); /* staged rejection must not commit */
    CHECK(project_save()==-9);
    CHECK(store_writes==writes);
    CHECK(!memcmp(current,store,sizeof current) && !memcmp(previous,store_previous,sizeof previous));
    CHECK(!memcmp(lens,store_len,sizeof lens) && !memcmp(prevlens,store_previous_len,sizeof prevlens));
    printf("prior %s %s: %s=%d unsupported=%d save=-9 no mounted writes\n",
           pat?"PAT":"SOUND",high?"high/collision":"low",stage?"load_stage":"boot_load",result,flag);
}

int main(void)
{
    codec();
    for(unsigned s=0;s<=1;++s) for(unsigned p=0;p<=1;++p) for(unsigned h=0;h<=1;++h)
        wire_reject(p,h,s);
    printf("PASS fixed catalog detached codec and exact old-reader staged project guard: %u checks\n",checks);
    return 0;
}
