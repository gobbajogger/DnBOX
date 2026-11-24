#define PWM_FIXTURE_MAIN retained_pwm_main
#include "ui_superj6_pwm1_lfo2_test.c"
#include "bank_reader.c"
#include "fixed_provider_host.h"
static juno6_t *last_synth,*parts[2];
void __real_juno6_init(juno6_t *s, float rate);
void __wrap_juno6_init(juno6_t *s, float rate) { __real_juno6_init(s,rate); last_synth=s; }
typedef struct {const char *name; unsigned midi,first,count; uint8_t uid[16];} original_picker_bank;
typedef struct {const char *name; unsigned position,midi; uint8_t uid[16];} original_picker_program;
#include "picker_catalog.inc"
static uint8_t image[FIX_SIZE], saved[sizeof store], previous[sizeof store_previous];
static int lens[OBJ_NOBJ], prevlens[OBJ_NOBJ];
static void check_saved(unsigned writes)
{
    CHECK(writes==store_writes && !memcmp(saved,store,sizeof store) &&
        !memcmp(previous,store_previous,sizeof store_previous) &&
        !memcmp(lens,store_len,sizeof store_len) && !memcmp(prevlens,store_previous_len,sizeof store_previous_len));
}
int main(void)
{
    const char *path=getenv("FIXED_HOST_IMAGE");
    CHECK(path);
    FILE *in=fopen(path,"rb");
    CHECK(in && fread(image,1,sizeof image,in)==sizeof image && fgetc(in)==EOF && !ferror(in));
    CHECK(!fclose(in));
    setup(V_PART,PART_303A);
    proj.set.bt|=PROJ_BT_CLK_IN;
    for(unsigned v=0;v<2;++v) for(unsigned legacy=0;legacy<4;++legacy) {
        select_owner(v,legacy);
        CHECK(proj.sound.v[T_303][v][30]==legacy);
    }
    isolate_dry_pair();
    save_and_reload();
    engine_apply_sound(&proj.sound);
    render(24);
    const sound_t prior=proj.sound;
    unsigned writes=store_writes;
    memcpy(saved,store,sizeof saved); memcpy(previous,store_previous,sizeof previous);
    memcpy(lens,store_len,sizeof lens); memcpy(prevlens,store_previous_len,sizeof prevlens);
    CHECK(!fixed_open(NULL,0) && !fixed_audition(0,0,picker_banks[0].uid,picker_programs[0].uid));
    CHECK(!fixed_open(image,sizeof image-1));
    const unsigned tamper[]={0,4,6,8,10,16,32,48,FIX_SIZE-1};
    for(unsigned k=0;k<sizeof tamper/sizeof *tamper;++k){
        image[tamper[k]]^=1;
        CHECK(!fixed_open(image,sizeof image) && !fixed_record(0));
        CHECK(!fixed_audition(0,0,picker_banks[0].uid,picker_programs[0].uid));
        image[tamper[k]]^=1;
    }
    CHECK(fixed_open(image,sizeof image) && !fixed_record(640));
    /* A plausible alternate order with valid program UUIDs and exact same
       field bytes. Reindex and keep positions contiguous: still not v1. */
    const unsigned a=fixed_index[0], b=fixed_index[1], c=fixed_index[2];
    CHECK(c-a<3000 && c>b && b>a && image[a+2]==image[b+2]);
    uint8_t ordered[3000], replaced[3000];
    memcpy(ordered,image+a,c-a);
    memcpy(replaced,ordered+b-a,c-b);
    memcpy(replaced+c-b,ordered,b-a);
    replaced[0]=replaced[3]=0;
    replaced[c-b]=replaced[c-b+3]=1;
    memcpy(image+a,replaced,c-a);
    CHECK(!fixed_open(image,sizeof image) && !fixed_record(0));
    CHECK(!fixed_audition(0,0,picker_banks[0].uid,picker_programs[1].uid));
    memcpy(image+a,ordered,c-a);
    CHECK(fixed_open(image,sizeof image));
    image[32]^=1; /* Passed original input cannot substitute identity later. */
    CHECK(fixed_record(0)[24]==picker_programs[0].uid[0]);
    image[32]^=1;
    CHECK(!fixed_audition(0,0,picker_banks[1].uid,picker_programs[0].uid));
    /* A different independently valid bank UUID does not bind this program. */
    memcpy(image+16,picker_banks[1].uid,16);
    CHECK(!fixed_open(image,sizeof image) && !fixed_record(0));
    memcpy(image+16,picker_banks[0].uid,16);
    CHECK(fixed_open(image,sizeof image));
    CHECK(!fixed_audition(0,0,picker_banks[0].uid,picker_programs[1].uid));
    CHECK(!fixed_audition(2,0,picker_banks[0].uid,picker_programs[0].uid));
    CHECK(!fixed_audition(0,640,picker_banks[0].uid,picker_programs[0].uid));
    CHECK(!engine_superj6_factory_pending() && !memcmp(&prior,&proj.sound,sizeof prior));
    check_saved(writes);
    for (unsigned v=0;v<2;++v) {
        last_synth=NULL;
        select_owner(v,0);
        CHECK(last_synth);
        parts[v]=last_synth;
    }
    CHECK(parts[0]!=parts[1]);
    unsigned selected=0,words=0,audio=0;
    for(unsigned b=0;b<12;++b) {
        const original_picker_bank *bank=picker_banks+b;
        for(unsigned p=0;p<bank->count;++p) {
            unsigned ord=bank->first+p;
            const original_picker_program *prog=picker_programs+ord;
            const uint8_t *r=fixed_record(ord);
            CHECK(r && r[2]==bank->midi && r[3]==prog->position && r[4]==prog->midi);
            CHECK(!memcmp(r+8,bank->uid,16) && !memcmp(r+24,prog->uid,16));
            CHECK(r[5]==strlen(bank->name) && !memcmp(r+40,bank->name,r[5]));
            CHECK(r[6]==strlen(prog->name) && !memcmp(r+40+r[5],prog->name,r[6]));
            sj6_record_reader rr;
            CHECK(sj6_select_uid(bank->uid,prog->uid,&rr));
            const uint8_t *f=fixed_fields(r);
            unsigned id; uint32_t w;
            for(unsigned n=0;n<r[7];++n,f+=5){
                CHECK(sj6_next_field(&rr,&id,&w) && id==f[0] && w==fixed_word(f));
                ++words;
            }
            CHECK(!sj6_next_field(&rr,&id,&w));
            for(unsigned v=0;v<2;++v){
                const sound_t before=proj.sound;
                if (!fixed_audition(v,ord,bank->uid,prog->uid)) {
                    fprintf(stderr,"ordinal=%u owner=%u\n",ord,v);
                    CHECK(0);
                }
                CHECK(engine_superj6_factory_pending());
                render(1);
                CHECK(!engine_superj6_factory_pending());
                CHECK(!memcmp(before.v[T_303][1-v],proj.sound.v[T_303][1-v],NPARAMS_MAX));
                CHECK(!memcmp(before.v[T_303][ORIGINAL_SJ6_EXTRA_ROW+1-v],proj.sound.v[T_303][ORIGINAL_SJ6_EXTRA_ROW+1-v],NPARAMS_MAX));
                ++selected;
                if (p==bank->count/2) {
                    float out[2][256]; unsigned nz=0;
                    CHECK(juno6_note_on(parts[v],69,100));
                    for(unsigned k=0;k<4;++k) {
                        CHECK(juno6_render_stereo(parts[v],out[0],out[1],256));
                        for(unsigned i=0;i<256;++i) nz += out[0][i]!=0.f || out[1][i]!=0.f;
                    }
                    CHECK(nz>512);
                    juno6_all_off(parts[v]);
                    ++audio;
                }
            }
        }
    }
    CHECK(selected==1280 && audio==24);
    check_saved(writes);
    CHECK(!project_load() && !memcmp(&prior,&proj.sound,sizeof prior));
    check_saved(writes);
    printf("PASS 640 named UUID records and %u exact authored binary32 fields; %u accepted actual owner loads, %u non-silent owner/bank renders; legacy four and negative no-write; %u checks\n",words,selected,audio,checks);
    return 0;
}
