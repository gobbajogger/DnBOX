/* SPDX-License-Identifier: GPL-3.0-only */
#define SJ6_LAYER1_FIXTURE_MAIN retained_tuning_main
#include "ui_superj6_layer1_tuning_test.c"
enum { TAP=66151 };
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
    CHECK(pg.r[cur_page()][1].c==SJ6_DELAY25 && pg.r[cur_page()][2].c==SJ6_DELAY26);
    CHECK(s->delay24.time_raw==0);
    for(unsigned n=0;n<256 && !s->delay24.time_raw;n++){now_ms+=400;knob(1,1);render(1);}
    CHECK(s->delay24.time_raw>0 && s->delay24.delay_target==1.5f*((float)s->delay24.time_raw/127.f)*44100.f);
    unsigned old=s->delay24.fb_raw;
    for(unsigned n=0;n<256 && s->delay24.fb_raw==old;n++){now_ms+=400;knob(2,1);render(1);}
    CHECK(s->delay24.fb_raw>old && s->delay24.fb_target==.99f*((float)s->delay24.fb_raw/127.f));
    CHECK(juno6_delay25_clock(s,240.f,1));
    CHECK(juno6_delay24_set(s,SJ6_DELAY25,3));
    CHECK(s->delay24.delay_target==1.5f*(3.f/127.f)*44100.f); /* sync division zero */
    CHECK(juno6_delay24_set(s,SJ6_DELAY25,8));
    CHECK(s->delay24.delay_target==.0234375f*44100.f);
    CHECK(!juno6_delay24_set(s,SJ6_DELAY25,127) && s->delay24.time_raw==8);
    const float prev_bpm=seq.bpm; const unsigned prev_ext=seq.ext;
    seq.ext=0; seq.bpm=240;
    CHECK(juno6_delay24_set(s,SJ6_DELAY25,90)); /* grid12, one bar at clock */
    render(1);
    CHECK(!s->delay24.clock_refused && s->delay24.delay_target==44100.f);
    seq.bpm=120;render(1);
    CHECK(s->delay24.clock_refused && s->delay24.delay_target==44100.f);
    CHECK(!juno6_delay24_set(s,SJ6_DELAY25,91) && s->delay24.time_raw==90);
    seq.bpm=240;render(1);
    CHECK(!s->delay24.clock_refused);
    CHECK(juno6_delay25_clock(s,240.f,0));
    CHECK(juno6_delay24_set(s,SJ6_DELAY25,8));
    seq.bpm=prev_bpm;seq.ext=prev_ext;
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
static void grid_and_ramps(void)
{
    static const float grid[18]={0,.0234375f,.03125f,.046875f,.0625f,.09375f,
        .125f,.1875f,.25f,.375f,.5f,.75f,1,2,3,4,6,8};
    juno6_t s;juno6_init(&s,44100.f);memset(ring[0],0,sizeof ring[0]);
    CHECK(juno6_delay24_arm(&s,ring[0][0],ring[0][1],TAP,0.f,10000));
    CHECK(juno6_delay24_set(&s,SJ6_DELAY25,127) && s.delay24.delay_target==66150.f);
    CHECK(!juno6_delay25_clock(&s,120.f,1) && !s.delay24.sync);
    CHECK(juno6_delay25_clock(&s,1440.f,1));
    for(unsigned raw=0;raw<128;raw++){
        CHECK(juno6_delay24_set(&s,SJ6_DELAY25,raw));
        unsigned index=(raw*34+127)/254;
        float target=index?grid[index]*(240.f/1440.f)*44100.f:1.5f*((float)raw/127.f)*44100.f;
        CHECK(s.delay24.delay_target==(target<1.f?1.f:target));
    }
    CHECK(juno6_delay24_set(&s,SJ6_DELAY26,127));
    CHECK(!juno6_delay24_set(&s,SJ6_DELAY26,128));
    float a[16]={0},b[16]={0};
    juno6_delay24_process(&s,a,b,8);
    CHECK(s.delay24.feedback==0.f && s.delay24.delay==1.f);
    juno6_delay24_process(&s,a,b,8);
    CHECK(s.delay24.feedback>0.f && s.delay24.feedback<.99f);
    CHECK(s.delay24.delay>1.f && s.delay24.delay<s.delay24.delay_target);
    memset(ring[0],0,sizeof ring[0]);
}
static void impulse(void)
{
    juno6_t s;
    juno6_init(&s,44100.f);float a[8]={0},b[8]={0};
    CHECK(juno6_delay24_arm(&s,a,b,8,.5f,10000));
    s.delay24.delay=s.delay24.delay_target=4.f;
    float l[32]={1},r[32]={0};
    /* Off still captures. Retiming is not in this test; verify the retained
     * independently routed feedback before the first Wet sample. */
    juno6_delay24_process(&s,l,r,4);CHECK(l[0]==1 && a[0]==1 && b[0]==0);
    s.delay24.dry=s.delay24.target=.25f;
    juno6_delay24_process(&s,l+4,r+4,12);
    CHECK(l[4]==.75f && l[8]==.375f && l[12]==.1875f);
    for(unsigned i=0;i<16;i++)CHECK(r[i]==0.f);
    juno6_init(&s,44100.f);memset(a,0,sizeof a);memset(b,0,sizeof b);
    CHECK(juno6_delay24_arm(&s,a,b,8,0.f,10000));
    s.delay24.dry=s.delay24.target=0;
    s.delay24.delay=s.delay24.delay_target=2.25f;
    float fl[6]={1},fr[6]={0};juno6_delay24_process(&s,fl,fr,6);
    CHECK(fl[1]==0.f && fl[2]==.75f && fl[3]==.25f && fl[4]==0.f);
    for(unsigned i=0;i<6;i++)CHECK(fr[i]==0.f);
}
int main(void)
{
    grid_and_ramps();impulse();journey(0);journey(1);
    printf("PASS ID24–27 real SMK controls, live independent stereo delay audition and exact settled dry bypass both owners (%u checks)\n",checks);
}
