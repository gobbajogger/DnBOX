/* SPDX-License-Identifier: GPL-3.0-only */
#include "dnbx_core.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks, tests;
#define CHECK(x) do { checks++; if (!(x)) { fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#x); exit(1); } } while (0)
#define RUN(x) do { x(); tests++; printf("PASS %s\n",#x); } while (0)
static float left[DNBX_MAX_FRAMES],right[DNBX_MAX_FRAMES];
static float alt_left[DNBX_MAX_FRAMES],alt_right[DNBX_MAX_FRAMES];

static dnbx_layer_params instant(float sustain)
{
    dnbx_layer_params p; dnbx_default_layer_params(&p);
    memset(&p.amp,0,sizeof p.amp); memset(&p.filter,0,sizeof p.filter);
    p.amp.sustain=sustain; p.filter.sustain=sustain;
    p.waveform=1; p.cutoff_hz=20000; p.filter_env_hz=0;
    return p;
}
static void set_both(dnbx_part *s,const dnbx_layer_params *p)
{ CHECK(dnbx_set_layer(s,0,p)==0); CHECK(dnbx_set_layer(s,1,p)==0); }
static void advance(dnbx_part *s,unsigned n)
{
    for (unsigned k=0;k<n;) {
        unsigned take=n-k>DNBX_MAX_FRAMES?DNBX_MAX_FRAMES:n-k;
        CHECK(dnbx_render(s,left,right,take,0)==0); k+=take;
    }
}
static void test_init_and_invalid_calls(void)
{
    dnbx_part s,copy; memset(&s,0xa5,sizeof s); copy=s;
    CHECK(dnbx_init(&s,NAN)==DNBX_E_RANGE); CHECK(!memcmp(&s,&copy,sizeof s));
    CHECK(dnbx_init(&s,INFINITY)==DNBX_E_RANGE);
    CHECK(dnbx_init(&s,7999)==DNBX_E_RANGE);
    CHECK(dnbx_init(&s,192001)==DNBX_E_RANGE);
    CHECK(dnbx_init(0,48000)==DNBX_E_ARGUMENT);
    CHECK(dnbx_init(&s,48000)==0); copy=s;
    CHECK(dnbx_render(&s,0,0,0,0)==0); CHECK(!memcmp(&s,&copy,sizeof s));
    CHECK(dnbx_render(&s,left,left,1,0)==DNBX_E_ARGUMENT);
    CHECK(dnbx_render(&s,left,right,DNBX_MAX_FRAMES+1,0)==DNBX_E_RANGE);
    CHECK(dnbx_note_on(&s,128,127)==DNBX_E_RANGE);
    CHECK(dnbx_note_on(&s,60,128)==DNBX_E_RANGE);
    CHECK(dnbx_note_off(&s,128)==DNBX_E_RANGE);
    CHECK(dnbx_set_sustain(&s,2)==DNBX_E_RANGE);
    CHECK(dnbx_set_velocity_mode(&s,2)==DNBX_E_RANGE);
    CHECK(!memcmp(&s,&copy,sizeof s));
    CHECK(dnbx_part_size()==sizeof s); CHECK(dnbx_layer_params_size()==sizeof(dnbx_layer_params));
    CHECK(dnbx_abi_version()==DNBX_ABI_VERSION);
    advance(&s,64);
    for (unsigned i=0;i<64;i++) CHECK(left[i]==0 && right[i]==0);
}
static void test_parameter_rejection_and_raw_admission(void)
{
    dnbx_part s,copy; dnbx_layer_params p,bad; float out;
    CHECK(dnbx_init(&s,48000)==0); dnbx_default_layer_params(&p);
    copy=s;
    for (unsigned i=0;i<23;i++) {
        bad=p; float value=NAN; memcpy((char *)&bad+i*sizeof(float),&value,sizeof value);
        CHECK(dnbx_set_layer(&s,0,&bad)==DNBX_E_RANGE); CHECK(!memcmp(&s,&copy,sizeof s));
        value=INFINITY; memcpy((char *)&bad+i*sizeof(float),&value,sizeof value);
        CHECK(dnbx_set_layer(&s,0,&bad)==DNBX_E_RANGE); CHECK(!memcmp(&s,&copy,sizeof s));
    }
    bad=p; bad.amp.attack_s=16.0001f; CHECK(dnbx_set_layer(&s,0,&bad)==DNBX_E_RANGE);
    bad=p; bad.pulse_width=.029f; CHECK(dnbx_set_layer(&s,0,&bad)==DNBX_E_RANGE);
    bad=p; bad.lfo_hz=16.001f; CHECK(dnbx_set_layer(&s,0,&bad)==DNBX_E_RANGE);
    bad=p; bad.gain=4.001f; CHECK(dnbx_set_layer(&s,0,&bad)==DNBX_E_RANGE);
    bad=p; bad.oscillator_mode=2; CHECK(dnbx_set_layer(&s,0,&bad)==DNBX_E_RANGE);
    CHECK(!memcmp(&s,&copy,sizeof s));
    const unsigned ids[]={0,185,203,143,207,212};
    const float values[]={1.32f,70.0f,9.0f,-.008403362f,0.0f,1.0f};
    for (unsigned i=0;i<6;i++) {
        int status=dnbx_store_raw(&s,ids[i],values[i]);
        CHECK(status==(ids[i]==207?DNBX_RAW_RESERVED:ids[i]==212?DNBX_RAW_EXTERNALLY_OWNED:DNBX_RAW_STORED_ONLY));
        CHECK(dnbx_read_raw(&s,ids[i],&out)==0); CHECK(!memcmp(&out,&values[i],sizeof out));
    }
    CHECK(!memcmp(s.layer,copy.layer,sizeof s.layer));
    CHECK(dnbx_store_raw(&s,10,FLT_MAX)==DNBX_RAW_STORED_ONLY);
    CHECK(dnbx_read_raw(&s,10,&out)==0 && out==FLT_MAX);
    CHECK(dnbx_read_raw(&s,11,&out)==DNBX_E_ABSENT);
    copy=s;
    CHECK(dnbx_store_raw(&s,218,0)==DNBX_E_RANGE);
    CHECK(dnbx_store_raw(&s,10,NAN)==DNBX_E_RANGE);
    CHECK(dnbx_store_raw(&s,10,-INFINITY)==DNBX_E_RANGE);
    CHECK(!memcmp(&s,&copy,sizeof s));
    p.gain=1.32f; CHECK(dnbx_set_layer(&s,0,&p)==0); CHECK(s.layer[0].params.gain==1.32f);
}
static void test_scalar_laws_and_waveforms(void)
{
    const float u[]={0,.25f,.5f,.75f,1};
    const float attack[]={.002f,.1601997968f,1.5928740834f,6.1398984597f,16};
    const float lfo[]={0,.1582195742f,1.5910729675f,6.1386657929f,16};
    float x,a,b,c;
    for (unsigned i=0;i<5;i++) {
        CHECK(dnbx_attack_from_unit(u[i],&x)==0 && fabsf(x-attack[i])<.00001f);
        CHECK(dnbx_lfo_from_unit(u[i],&x)==0 && fabsf(x-lfo[i])<.00001f);
        CHECK(dnbx_pwm_from_unit(u[i],&x)==0 && fabsf(x-(.5f-.47f*u[i]))<.0000001f);
    }
    CHECK(dnbx_range_from_stop(0,&x)==0 && x==.5f);
    CHECK(dnbx_range_from_stop(.5f,&x)==0 && x==1);
    CHECK(dnbx_range_from_stop(1,&x)==0 && x==2);
    CHECK(dnbx_range_from_stop(.25f,&x)==DNBX_E_RANGE);
    CHECK(dnbx_attack_from_unit(NAN,&x)==DNBX_E_RANGE);
    CHECK(dnbx_wave_probe(.25f,.01f,0,.5f,&x)==0 && fabsf(x-1)<.000001f);
    CHECK(dnbx_wave_probe(.25f,.01f,1.0f/3,.5f,&x)==0 && fabsf(x)<.000001f);
    CHECK(dnbx_wave_probe(.25f,.01f,2.0f/3,.5f,&x)==0 && x==-.5f);
    CHECK(dnbx_wave_probe(.25f,.01f,1,.5f,&x)==0 && x==1);
    CHECK(dnbx_wave_probe(.31f,.01f,1.0f/3,.3f,&a)==0);
    CHECK(dnbx_wave_probe(.31f,.01f,2.0f/3,.3f,&b)==0);
    CHECK(dnbx_wave_probe(.31f,.01f,.5f,.3f,&c)==0 && fabsf(c-(a+b)*.5f)<.0000001f);
    CHECK(dnbx_wave_probe_mode(.01f,.1f,2.0f/3,.5f,DNBX_OSC_RAW,&a)==0 && a==-.98f);
    CHECK(dnbx_wave_probe(.01f,.1f,2.0f/3,.5f,&b)==0 && fabsf(a-b)>.5f);
    CHECK(dnbx_wave_probe_mode(.01f,.1f,1,.5f,DNBX_OSC_RAW,&a)==0 && a==1);
    CHECK(dnbx_wave_probe(.01f,.1f,1,.5f,&b)==0 && fabsf(a-b)>.5f);
}
static void test_zero_and_one_sample_restarts(void)
{
    dnbx_part s; dnbx_layer_params p=instant(.7f);
    CHECK(dnbx_init(&s,48000)==0); set_both(&s,&p);
    for (unsigned i=0;i<100;i++) {
        CHECK(dnbx_note_on(&s,60,100)==0);
        CHECK(s.voice[0].layer[0].amp.stage==DNBX_ENV_SUSTAIN);
        CHECK(s.voice[0].layer[0].amp.level==.7f);
        advance(&s,1);
        CHECK(dnbx_note_off(&s,60)==0); CHECK(dnbx_active_voices(&s)==0);
    }
    /* Regression boundary: zero attack retrigger exactly one rendered sample
     * after a nonzero attack, while the earlier segment has pending work. */
    p.amp.attack_s=.01f; p.filter.attack_s=.02f; set_both(&s,&p);
    CHECK(dnbx_note_on(&s,60,100)==0); advance(&s,1);
    CHECK(s.voice[0].layer[0].amp.stage==DNBX_ENV_ATTACK);
    p.amp.attack_s=0; p.filter.attack_s=0; set_both(&s,&p);
    CHECK(dnbx_note_on(&s,60,100)==0);
    CHECK(s.voice[0].layer[0].amp.stage==DNBX_ENV_SUSTAIN);
    CHECK(s.voice[0].layer[0].amp.level==.7f); advance(&s,1);
    CHECK(dnbx_note_off(&s,60)==0); CHECK(dnbx_active_voices(&s)==0);
    p.amp.attack_s=p.filter.attack_s=1.0f/48000; set_both(&s,&p);
    CHECK(dnbx_note_on(&s,60,127)==0);
    CHECK(s.voice[0].layer[0].amp.level==1); advance(&s,1);
    CHECK(dnbx_note_on(&s,60,127)==0); advance(&s,1); advance(&s,17);
    CHECK(s.voice[0].layer[0].amp.stage==DNBX_ENV_SUSTAIN);
    CHECK(s.voice[0].layer[0].amp.level==.7f);
    CHECK(dnbx_note_off(&s,60)==0); CHECK(dnbx_active_voices(&s)==0);
    /* All six stages can be zero without a one-sample dead state. */
    p=instant(0); set_both(&s,&p); CHECK(dnbx_note_on(&s,60,127)==0);
    advance(&s,1); CHECK(left[0]==0 && right[0]==0);
    CHECK(dnbx_note_on(&s,60,0)==DNBX_NOTE_OFF_ONLY); CHECK(dnbx_active_voices(&s)==0);
}
static void test_envelopes_separate_and_long_attack(void)
{
    dnbx_part s; dnbx_layer_params p=instant(.4f);
    CHECK(dnbx_init(&s,48000)==0);
    p.amp.attack_s=.01f; p.amp.decay_s=.02f; p.amp.release_s=.01f;
    p.filter.attack_s=.1f; p.filter.release_s=.1f; set_both(&s,&p);
    CHECK(dnbx_note_on(&s,60,127)==0); advance(&s,1600);
    CHECK(s.voice[0].layer[0].amp.stage==DNBX_ENV_SUSTAIN);
    CHECK(s.voice[0].layer[0].filter.stage==DNBX_ENV_ATTACK);
    CHECK(s.voice[0].layer[0].amp.level!=s.voice[0].layer[0].filter.level);
    CHECK(dnbx_note_off(&s,60)==0); advance(&s,600);
    CHECK(s.voice[0].layer[0].amp.stage==DNBX_ENV_IDLE);
    CHECK(s.voice[0].layer[0].filter.stage==DNBX_ENV_RELEASE);
    advance(&s,4800); CHECK(dnbx_active_voices(&s)==0);
    p=instant(.7f); p.amp.attack_s=16; set_both(&s,&p);
    CHECK(dnbx_note_on(&s,60,127)==0); advance(&s,16u*48000u-1u);
    CHECK(s.voice[0].layer[0].amp.stage==DNBX_ENV_ATTACK);
    CHECK(s.voice[0].layer[0].amp.level>.999f && s.voice[0].layer[0].amp.level<1);
    advance(&s,1); CHECK(s.voice[0].layer[0].amp.level==1);
    advance(&s,17); CHECK(s.voice[0].layer[0].amp.stage==DNBX_ENV_SUSTAIN);
}
static void test_allocator_sustain_and_tail(void)
{
    dnbx_part s,copy; dnbx_layer_params p=instant(.7f);
    p.amp.release_s=p.filter.release_s=.1f;
    CHECK(dnbx_init(&s,48000)==0); set_both(&s,&p);
    for (unsigned i=0;i<6;i++) CHECK(dnbx_note_on(&s,60+i,127)==(int)i);
    CHECK(dnbx_active_voices(&s)==6);
    CHECK(dnbx_note_on(&s,62,100)==2); CHECK(dnbx_active_voices(&s)==6);
    CHECK(dnbx_note_on(&s,67,127)==0); CHECK(s.voice[0].note==67);
    CHECK(dnbx_note_off(&s,64)==0); CHECK(dnbx_note_on(&s,68,127)==4);
    CHECK(dnbx_set_sustain(&s,1)==0); CHECK(dnbx_note_off(&s,61)==0);
    CHECK(s.voice[1].pedal_held==1 && s.voice[1].gate==0);
    advance(&s,8000); CHECK(s.voice[1].layer[0].amp.stage==DNBX_ENV_SUSTAIN);
    CHECK(dnbx_set_sustain(&s,0)==0);
    CHECK(s.voice[1].layer[0].amp.stage==DNBX_ENV_RELEASE);
    advance(&s,100); copy=s;
    CHECK(dnbx_set_sustain(&s,1)==0); CHECK(dnbx_set_sustain(&s,0)==0);
    CHECK(!memcmp(&s.voice[1],&copy.voice[1],sizeof s.voice[1]));
    advance(&s,5000); CHECK(!s.voice[1].layer[0].amp.stage);
    CHECK(dnbx_all_off(&s)==0); CHECK(dnbx_active_voices(&s)==0); advance(&s,64);
    for (unsigned i=0;i<64;i++) CHECK(left[i]==0 && right[i]==0);
    /* Rank allocator has no timestamp wrap after many event cycles. */
    for (unsigned i=0;i<5000;i++) CHECK(dnbx_note_on(&s,40+i%70,100)>=0);
    for (unsigned i=0;i<6;i++) CHECK(s.voice[i].age_rank<6);
}
static void test_oldest_after_retirement(void)
{
    dnbx_part s; dnbx_layer_params p=instant(1);
    CHECK(dnbx_init(&s,48000)==0); set_both(&s,&p);
    for (unsigned i=0;i<6;i++) CHECK(dnbx_note_on(&s,60+i,127)==(int)i);
    for (unsigned i=0;i<5;i++) {
        CHECK(dnbx_note_off(&s,60+i)==0); advance(&s,1);
        CHECK(dnbx_note_on(&s,70+i,127)==(int)i);
    }
    for (unsigned note=80,previous=70;note<90;previous=note,note++) {
        CHECK(dnbx_note_off(&s,previous)==0); advance(&s,1);
        CHECK(dnbx_note_on(&s,note,127)==0);
    }
    CHECK(dnbx_note_on(&s,99,127)==5);
    for (unsigned i=0;i<6;i++) {
        CHECK(s.voice[i].age_rank<6);
        for (unsigned j=i+1;j<6;j++) CHECK(s.voice[i].age_rank!=s.voice[j].age_rank);
    }
}
static void test_layer_and_part_isolation(void)
{
    dnbx_part a,b,foreign; dnbx_layer_params p=instant(.8f),altered;
    CHECK(dnbx_init(&a,48000)==0);
    p.pan=-1; p.lfo_hz=4; p.lfo_pwm_depth=.2f; CHECK(dnbx_set_layer(&a,0,&p)==0);
    p.pan=1; p.waveform=.2f; p.lfo_hz=7; CHECK(dnbx_set_layer(&a,1,&p)==0);
    b=a; altered=a.layer[0].params; altered.cutoff_hz=20; altered.lfo_hz=16;
    CHECK(dnbx_set_layer(&a,0,&altered)==0);
    CHECK(dnbx_note_on(&a,60,100)==0); CHECK(dnbx_note_on(&b,60,100)==0);
    CHECK(dnbx_render(&a,left,right,512,0)==0);
    CHECK(dnbx_render(&b,alt_left,alt_right,512,0)==0);
    CHECK(memcmp(left,alt_left,512*sizeof(float))!=0);
    CHECK(!memcmp(right,alt_right,512*sizeof(float)));
    CHECK(!memcmp(&a.voice[0].layer[1],&b.voice[0].layer[1],sizeof(dnbx_layer_voice)));
    CHECK(!memcmp(&a.layer[1],&b.layer[1],sizeof(dnbx_layer)));
    b=a; CHECK(dnbx_init(&foreign,44100)==0);
    for (unsigned block=0;block<100;block++) {
        CHECK(dnbx_note_on(&foreign,block%128,127)>=0);
        CHECK(dnbx_store_raw(&foreign,185,70+block)==DNBX_RAW_STORED_ONLY);
        advance(&foreign,33);
        CHECK(dnbx_render(&a,left,right,64,0)==0);
        CHECK(dnbx_render(&b,alt_left,alt_right,64,0)==0);
        CHECK(!memcmp(left,alt_left,64*sizeof(float)));
        CHECK(!memcmp(right,alt_right,64*sizeof(float)));
    }
    CHECK(!memcmp(&a,&b,sizeof a));
}
static void test_velocity_modes(void)
{
    dnbx_part a,b; dnbx_layer_params p=instant(1);
    const float q=64.0f/127;
    CHECK(dnbx_init(&a,48000)==0); set_both(&a,&p); b=a;
    CHECK(dnbx_note_on(&a,60,64)==0); CHECK(dnbx_note_on(&b,60,127)==0);
    CHECK(dnbx_render(&a,left,right,128,0)==0);
    CHECK(dnbx_render(&b,alt_left,alt_right,128,0)==0);
    for (unsigned i=0;i<128;i++) CHECK(fabsf(left[i]-alt_left[i]*q)<.0000001f);
    CHECK(dnbx_set_velocity_mode(&a,DNBX_VELOCITY_SQUARED)==0);
    CHECK(dnbx_set_velocity_mode(&b,DNBX_VELOCITY_SQUARED)==0);
    CHECK(dnbx_render(&a,left,right,128,0)==0);
    CHECK(dnbx_render(&b,alt_left,alt_right,128,0)==0);
    for (unsigned i=0;i<128;i++) CHECK(fabsf(left[i]-alt_left[i]*q*q)<.0000001f);
}
static void event(dnbx_part *s,unsigned frame)
{
    switch (frame) {
    case 0: CHECK(dnbx_note_on(s,60,100)==0); break;
    case 1001: CHECK(dnbx_note_off(s,60)==0); break;
    case 2123: CHECK(dnbx_note_on(s,60,64)>=0); break;
    case 3007: CHECK(dnbx_set_sustain(s,1)==0); break;
    case 5003: CHECK(dnbx_note_off(s,60)==0); break;
    case 6001: CHECK(dnbx_set_sustain(s,0)==0); break;
    case 7003: CHECK(dnbx_note_on(s,67,127)>=0); break;
    default: break;
    }
}
static void partition_render(dnbx_part *s,float *l,float *r,const unsigned *blocks,unsigned count)
{
    const unsigned events[]={0,1001,2123,3007,5003,6001,7003,8192};
    unsigned frame=0,ei=0,bi=0;
    while (frame<8192) {
        if (frame==events[ei]) { event(s,frame); ei++; }
        unsigned take=blocks[bi++%count];
        if (take>events[ei]-frame) take=events[ei]-frame;
        CHECK(dnbx_render(s,l+frame,r+frame,take,0)==0); frame+=take;
    }
}
static void test_block_partition_invariance(void)
{
    static float a_l[8192],a_r[8192],b_l[8192],b_r[8192];
    dnbx_part a,b; dnbx_layer_params p;
    const unsigned regular[]={1024},irregular[]={1,3,5,7,13,16,63,127,509};
    CHECK(dnbx_init(&a,48000)==0); dnbx_default_layer_params(&p);
    p.amp.attack_s=.01013f; p.amp.hold_s=.00017f; p.amp.decay_s=.05f;
    p.amp.release_hold_s=.0037f; p.amp.release_s=.1f;
    p.lfo_hz=16; p.lfo_pwm_depth=.47f; p.waveform=.85f; set_both(&a,&p); b=a;
    partition_render(&a,a_l,a_r,regular,1); partition_render(&b,b_l,b_r,irregular,9);
    CHECK(!memcmp(a_l,b_l,sizeof a_l)); CHECK(!memcmp(a_r,b_r,sizeof a_r));
    CHECK(!memcmp(&a,&b,sizeof a));
}
static uint32_t random_state=0x625baa31u;
static uint32_t random_word(void)
{ random_state^=random_state<<13; random_state^=random_state>>17; random_state^=random_state<<5; return random_state; }
static float unit(void) { return (float)(random_word()&65535)/65535.0f; }
static float fuzz_time(float sr)
{
    switch (random_word()%5) {
    case 0: return 0;
    case 1: return 1/sr;
    case 2: return 16;
    default: return unit()*.1f;
    }
}
static void test_finite_parameter_boundaries(void)
{
    const float rates[]={8000,44100,48000,96000,192000};
    for (unsigned sr=0;sr<5;sr++) {
        dnbx_part s; CHECK(dnbx_init(&s,rates[sr])==0);
        for (unsigned block=0;block<1000;block++) {
            if (block%17==0) for (unsigned layer=0;layer<2;layer++) {
                dnbx_layer_params p=instant(unit());
                p.amp.attack_s=fuzz_time(rates[sr]); p.amp.hold_s=fuzz_time(rates[sr]);
                p.amp.decay_s=fuzz_time(rates[sr]); p.amp.release_s=fuzz_time(rates[sr]);
                p.amp.release_hold_s=fuzz_time(rates[sr]);
                p.filter.attack_s=fuzz_time(rates[sr]); p.filter.decay_s=fuzz_time(rates[sr]);
                p.filter.hold_s=fuzz_time(rates[sr]); p.filter.release_s=fuzz_time(rates[sr]);
                p.filter.release_hold_s=fuzz_time(rates[sr]); p.filter.sustain=unit();
                p.waveform=unit(); p.pulse_width=.03f+.47f*unit(); p.gain=4; p.sub_gain=.5f;
                p.cutoff_hz=20+19980*unit(); p.filter_env_hz=unit()<.5f?-20000:20000;
                p.resonance=1; p.lfo_hz=16; p.lfo_pwm_depth=.47f;
                p.tune_semitones=unit()<.5f?-48:48; p.pan=unit()*2-1;
                p.oscillator_mode=random_word()%2; CHECK(dnbx_set_layer(&s,layer,&p)==0);
            }
            unsigned note=random_word()%128;
            CHECK(dnbx_note_on(&s,note,1+random_word()%127)>=0);
            if (block%5==0) CHECK(dnbx_note_off(&s,note)==0);
            CHECK(dnbx_set_sustain(&s,(block/11)%2)==0);
            CHECK(dnbx_render(&s,left,right,32,0)==0);
            for (unsigned i=0;i<32;i++) CHECK(isfinite(left[i]) && isfinite(right[i]) &&
                fabsf(left[i])<=DNBX_OUTPUT_ABS_BOUND && fabsf(right[i])<=DNBX_OUTPUT_ABS_BOUND);
            for (unsigned v=0;v<6;v++) for (unsigned l=0;l<2;l++) {
                const dnbx_layer_voice *x=&s.voice[v].layer[l];
                CHECK(x->amp.level>=0 && x->amp.level<=1 && isfinite(x->amp.level));
                CHECK(x->filter.level>=0 && x->filter.level<=1 && isfinite(x->filter.level));
                CHECK(x->phase>=0 && x->phase<1 && isfinite(x->phase));
                CHECK(fabsf(x->filter_first)<=4 && fabsf(x->filter_second)<=4);
            }
        }
    }
}
static void test_callback_counters(void)
{
    dnbx_part s; dnbx_layer_params p=instant(1); dnbx_work work={0};
    CHECK(dnbx_init(&s,48000)==0); p.waveform=.85f; p.lfo_hz=16; set_both(&s,&p);
    for (unsigned i=0;i<6;i++) CHECK(dnbx_note_on(&s,60+i,127)==(int)i);
#if defined(DNBX_DIAGNOSTIC_COUNTERS) && !DNBX_DIAGNOSTIC_COUNTERS
    dnbx_part copy=s;
    CHECK(dnbx_diagnostic_counters_enabled()==0);
    CHECK(dnbx_render(&s,left,right,128,&work)==DNBX_E_UNSUPPORTED);
    CHECK(!memcmp(&s,&copy,sizeof s)); CHECK(work.frames==0);
    CHECK(dnbx_render(&s,left,right,128,0)==0);
    printf("COUNTERS explicitly_disabled nonnull_work_rejected\n");
    return;
#endif
    CHECK(dnbx_render(&s,left,right,128,&work)==0);
    CHECK(work.frames==128); CHECK(work.voice_slots==128*6);
    CHECK(work.active_voice_samples==128*6); CHECK(work.layer_samples==128*12);
    CHECK(work.envelope_samples==128*24); CHECK(work.lfo_samples==128*2);
    CHECK(work.blep_calls==128*12*3); CHECK(work.sine_calls==0);
    CHECK(work.envelope_ticks==0 && work.exp2_calls==0 && work.log2_calls==0);
    printf("COUNTERS steady_128 frames=%llu voice_slots=%llu layers=%llu envelopes=%llu lfo=%llu blep=%llu\n",
      (unsigned long long)work.frames,(unsigned long long)work.voice_slots,
      (unsigned long long)work.layer_samples,(unsigned long long)work.envelope_samples,
      (unsigned long long)work.lfo_samples,(unsigned long long)work.blep_calls);
}
static void dump_traces(const char *path)
{
    FILE *f=fopen(path,"w"); CHECK(f!=0);
    fprintf(f,"case_index,frame,amp,stage\n");
    for (unsigned which=1;which<=2;which++) {
        dnbx_part s; dnbx_layer_params p=instant(which==1?.7f:.4f);
        unsigned frames=which==1?8192:32768,off=which==1?4096:16384;
        p.amp.attack_s=which==1?.01f:.03f; p.amp.hold_s=which==1?0:.02f;
        p.amp.decay_s=which==1?.05f:.2f; p.amp.release_hold_s=which==1?0:.04f;
        p.amp.release_s=which==1?.1f:.3f;
        CHECK(dnbx_init(&s,48000)==0); set_both(&s,&p); CHECK(dnbx_note_on(&s,60,127)==0);
        for (unsigned n=0;n<frames;n++) {
            if (n==off) CHECK(dnbx_note_off(&s,60)==0);
            CHECK(dnbx_render(&s,left,right,1,0)==0);
            fprintf(f,"%u,%u,%.9g,%u\n",which,n,(double)s.voice[0].layer[0].amp.level,s.voice[0].layer[0].amp.stage);
        }
    }
    CHECK(fclose(f)==0);
}
int main(int argc,char **argv)
{
    RUN(test_init_and_invalid_calls);
    RUN(test_parameter_rejection_and_raw_admission);
    RUN(test_scalar_laws_and_waveforms);
    RUN(test_zero_and_one_sample_restarts);
    RUN(test_envelopes_separate_and_long_attack);
    RUN(test_allocator_sustain_and_tail);
    RUN(test_oldest_after_retirement);
    RUN(test_layer_and_part_isolation);
    RUN(test_velocity_modes);
    RUN(test_block_partition_invariance);
    RUN(test_finite_parameter_boundaries);
    RUN(test_callback_counters);
    if (argc>1) dump_traces(argv[1]);
    printf("RESULT tests=%u checks=%u status=PASS part_bytes=%zu params_bytes=%zu env_bytes=%zu voice_bytes=%zu work_bytes=%zu\n",
      tests,checks,sizeof(dnbx_part),sizeof(dnbx_layer_params),sizeof(dnbx_envelope_state),sizeof(dnbx_voice),sizeof(dnbx_work));
    return 0;
}
