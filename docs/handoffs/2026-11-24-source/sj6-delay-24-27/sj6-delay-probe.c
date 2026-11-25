/* SPDX-License-Identifier: GPL-3.0-only */
#define SJ6_LAYER1_FIXTURE_MAIN retained_tuning_main
#include "ui_superj6_layer1_tuning_test.c"
enum { TAP=11025 };
static float ring[2][2][TAP];
static juno6_t *watched;
static unsigned words;
void __real_juno6_delay24_process(juno6_t*,float*,float*,unsigned);
void __wrap_juno6_delay24_process(juno6_t *s,float *l,float *r,unsigned n)
{
    if(s!=watched){__real_juno6_delay24_process(s,l,r,n);return;}
    float before[2][256];
    CHECK(n<=256);
    memcpy(before[0],l,n*sizeof *l);memcpy(before[1],r,n*sizeof *r);
    __real_juno6_delay24_process(s,l,r,n);
    for(unsigned k=0;k<n;k++)words+=(l[k]!=before[0][k])+(r[k]!=before[1][k]);
}
static int has_delay(void)
{
    build_pages();
    for(int i=0;i<pg.n;i++)if(!strcmp(page_title(&pg,i),"DELAY"))return 1;
    return 0;
}
static void journey(unsigned v)
{
    setup_tuning(v);
    CHECK(!has_delay());
    juno6_t *s=parts[v];watched=s;words=0;
    CHECK(!juno6_delay24_arm(s,ring[v][0],ring[v][1],TAP,.5f,0));
    CHECK(!juno6_delay24_arm(s,ring[v][0],ring[v][0],TAP,.5f,10000));
    CHECK(juno6_delay24_arm(s,ring[v][0],ring[v][1],TAP,.5f,10000));
    CHECK(has_delay());
    visit("DELAY");
    CHECK(pg.r[cur_page()][0].c==SJ6_DELAY24);
    CHECK(pg.r[cur_page()][1].kind==R_NONE && pg.r[cur_page()][2].kind==R_NONE);
    CHECK(pg.r[cur_page()][3].c==SJ6_DELAY27);
    const sound_t base=proj.sound; const unsigned writes=store_writes;
    for(unsigned n=0;n<256 && !s->delay24.mix;n++){now_ms+=400;knob(3,1);render(1);}
    CHECK(s->delay24.mix>0 && s->delay24.enabled==0 && s->delay24.dry==1.f);
    engine_bass_on(v,57,0,0);render(40);
    CHECK(words==0);  /* exact waveform bypass, not mute */
    unsigned pos=s->delay24.pos;
    now_ms+=400;knob(0,1);render(1);
    CHECK(s->delay24.enabled==1 && s->delay24.pos!=pos);
    CHECK(s->delay24.target<1.f);
    render(56);
    CHECK(words>0 && s->delay24.dry==s->delay24.target);
    unsigned gained=words;
    now_ms+=400;knob(3,1);render(56);
    CHECK(s->delay24.mix>1 && words>gained);
    now_ms+=400;knob(0,-1);render(56);
    CHECK(!s->delay24.enabled && s->delay24.dry==1.f);
    words=0;pos=s->delay24.pos;render(18);
    CHECK(!words && s->delay24.pos!=pos && !parts[1-v]->delay24.tap);
    CHECK(!memcmp(&base,&proj.sound,sizeof base) && store_writes==writes);
    engine_bass_off(v);render(8);watched=NULL;
}
static void impulse(void)
{
    juno6_t s;
    juno6_init(&s,44100.f);float a[4]={0},b[4]={0};
    CHECK(juno6_delay24_arm(&s,a,b,4,.5f,10000));
    float l[32]={1},r[32]={0};
    /* Off still captures. Retiming is not in this test; verify the retained
     * independently routed feedback before the first Wet sample. */
    juno6_delay24_process(&s,l,r,4);CHECK(l[0]==1 && a[0]==1 && b[0]==0);
    s.delay24.dry=s.delay24.target=.25f;
    juno6_delay24_process(&s,l+4,r+4,12);
    CHECK(l[4]==.75f && l[8]==.375f && l[12]==.1875f);
    for(unsigned i=0;i<16;i++)CHECK(r[i]==0.f);
}
int main(void)
{
    impulse();journey(0);journey(1);
    printf("PASS ID24/27 real SMK controls, live independent stereo delay audition and exact settled dry bypass both owners (%u checks)\n",checks);
}
