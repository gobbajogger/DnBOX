/* SPDX-License-Identifier: GPL-3.0-only */
/* The exact SMK wheels function and real audio/MIDI queue, selected Init route. */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
#include "juno6.h"
#include "s1c_part.h"
static struct { unsigned frames; int wheel[2]; uint16_t wheel_mod_seq; } fm1_in;
static int clamp(int v,int lo,int hi){return v<lo?lo:v>hi?hi:v;}
#include "wheels.inc"
static juno6_t *j;
static s1_dry_part *s;
void __real_juno6_init(juno6_t *,float);
void __wrap_juno6_init(juno6_t *p,float rate){__real_juno6_init(p,rate);j=p;}
void __real_s1_part_init(s1_dry_part *);
void __wrap_s1_part_init(s1_dry_part *p){__real_s1_part_init(p);s=p;}
static void adc_mod(int mod)
{
    fm1_in.wheel[0]=500; fm1_in.wheel[1]=mod;
    fm1_in.frames=301; ++fm1_in.wheel_mod_seq;
    smk_wheels(); render(4);
}
static void select_kind(unsigned v,unsigned bank)
{
    EXPECT(!engine_bass_select_preset(v,bank,0,&proj.sound));
    for(unsigned i=0;i<1024&&engine_bass_preset_pending(v);++i)render(1);
    EXPECT(engine_bass_bank(v)==(int)bank);
}
static void cc1(unsigned v,unsigned value)
{
    unsigned ch=seq.ch[TRK_BASS0+v];
    host_midi_put(0xbu | ((0xb0u|ch)<<8) | (1u<<16) | (value<<24));
    render(4);
}
static float maxdiff;
static unsigned process_calls;
static juno6_t *active;
void __real_juno6_alt3_process(juno6_t *,float *,float *,unsigned);
void __wrap_juno6_alt3_process(juno6_t *part,float *l,float *r,unsigned n)
{
    if(part!=active){__real_juno6_alt3_process(part,l,r,n);return;}
    float old[256];
    EXPECT(n<=256);
    memcpy(old,l,n*sizeof(float));
    __real_juno6_alt3_process(part,l,r,n);
    for(unsigned i=0;i<n;i++)maxdiff=fmaxf(maxdiff,fabsf(l[i]-old[i]));
    ++process_calls;
}
static float synced_rate(unsigned cc,float bpm)
{
    const double g[]={64,48,32,24,16,12,8,6,4,3,2,1.5,1,.5,.3333,.25,.1667,.125};
    unsigned idx=(unsigned)floorf((1.f-.9f*((float)cc/127.f))*17.f+.5f);
    return (float)(((double)bpm/60.0*g[idx])*.25);
}
static void wheel_case(unsigned v,int sync)
{
    unsigned peer=1-v;
    select_kind(peer,ORIGINAL_SOUND_JUNO); juno6_t *other=j;
    select_kind(v,ORIGINAL_SOUND_JUNO); juno6_t *part=j;
    EXPECT(part&&other&&part!=other);
    EXPECT(!juno6_set_mod_destination(part,3));
    EXPECT(!juno6_arm_alt3_free(part,10000,1));
    juno6_t guard=*part;
    EXPECT(!juno6_arm_alt3_running(part,0,sync)&&!juno6_arm_alt3_running(part,9999,sync));
    EXPECT(!memcmp(&guard,part,sizeof guard));
    EXPECT(!part->alt3.armed && !other->alt3.armed);
    active=part;
    engine_bass_on(v,57,0,0);render(4);
    EXPECT(process_calls>0 && maxdiff==0 && !part->alt3.armed);
    EXPECT(juno6_arm_alt3_running(part,10000,sync)&&juno6_set_mod_destination(part,3));
    EXPECT(part->alt3.rate==3.f && part->alt3.depth==0.f);
    const sound_t saved=proj.sound;
    const unsigned writes=store_writes;
    ui.part=PART_303A+v;adc_mod(615);
    EXPECT(part->alt3.depth==0.f);
    adc_mod(485);
    EXPECT(engine_bass_perf(v,2,1)==127 && part->alt3.to_depth==1.f);
    EXPECT(part->alt3.depth>0.f && part->alt3.depth<1.f && maxdiff>0.f);
    EXPECT(!other->alt3.armed && !other->alt3.depth && !other->alt3.phase);
    render(48);
    EXPECT(part->alt3.depth==1.f && part->alt3.rate==part->alt3.to_rate);
    if(sync)EXPECT(part->alt3.to_rate==synced_rate(127,seq_tempo(&seq)));
    cc1(v,43);
    EXPECT(part->alt3.to_depth==43.f/127.f && part->alt3.depth>part->alt3.to_depth);
    if(sync){
        EXPECT(part->alt3.to_rate==synced_rate(43,seq_tempo(&seq)));
        seq.bpm=143.f; render(1);
        EXPECT(part->alt3.to_rate==synced_rate(43,143.f));
        EXPECT(part->alt3.depth_elapsed>1024u && part->alt3.elapsed<=256u);
    }
    render(48);EXPECT(part->alt3.depth==43.f/127.f);
    cc1(v,0);
    EXPECT(part->alt3.to_depth==0.f && part->alt3.depth>0.f);
    render(48);EXPECT(part->alt3.depth==0.f);
    maxdiff=0;render(8);EXPECT(maxdiff==0);
    EXPECT(!memcmp(&saved,&proj.sound,sizeof(saved)) && store_writes==writes);
    engine_bass_off(v);render(16);active=0;maxdiff=0;process_calls=0;
}
static void interpolation(void)
{
    juno6_t a,b;
    juno6_init(&a,44100.f);
    EXPECT(juno6_arm_alt3_free(&a,10000,0)&&juno6_set_mod_destination(&a,3));
    EXPECT(juno6_set_mod_wheel(&a,127));
    b=a;
    float l[24],r[24],ll[24],rr[24];
    for(unsigned i=0;i<24;i++)l[i]=r[i]=ll[i]=rr[i]=1.f;
    juno6_alt3_process(&a,l,r,24);
    juno6_alt3_process(&b,ll,rr,5);
    juno6_alt3_process(&b,ll+5,rr+5,7);
    juno6_alt3_process(&b,ll+12,rr+12,12);
    EXPECT(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r)&&!memcmp(&a.alt3,&b.alt3,sizeof a.alt3));
    for(unsigned i=0;i<8;i++)EXPECT(l[i]==1.f&&r[i]==1.f);
    const float phase=(float)(((unsigned)nearbyintf(1.5f*(0x1p24f/44100.f)))*16u)*(1.f/0x1p24f);
    const float wave=.5f*(1.f+sinf(6.283185307179586f*phase));
    EXPECT(fabsf(l[8]-(1.f-wave*(8.f/10000.f)))<1e-7f);
    EXPECT(fabsf(a.alt3.depth-(16.f/10000.f))<1e-8f);
}
int main(void)
{
    interpolation();
    juno6_t x;
    juno6_init(&x,44100.f);
    EXPECT(!juno6_arm_alt3_running(&x,10000,1));
    juno6_set_lfo_tempo(&x,93.5f);
    EXPECT(juno6_arm_alt3_running(&x,10000,1)&&juno6_set_mod_destination(&x,3));
    for(unsigned q=0;q<128;q++) {
        EXPECT(juno6_set_mod_wheel(&x,q));
        EXPECT(x.alt3.to_rate==synced_rate(q,93.5f));
    }
    juno6_set_lfo_tempo(&x,0.f);
    juno6_t old=x;
    EXPECT(!juno6_set_mod_wheel(&x,40) && !memcmp(&old,&x,sizeof x));
    setup(V_PART,PART_303A);
    wheel_case(0,0);setup(V_PART,PART_303B);wheel_case(1,0);
    setup(V_PART,PART_303A);wheel_case(0,1);
    setup(V_PART,PART_303B);wheel_case(1,1);
    if(failures)return 1;
    printf("PASS fenced free-Hz and synchronized ALT3 real SMK ADC/CC1 on both owners, active ramps, no peer or authored mutation (%u assertions)\n",expectations);
    return 0;
}
