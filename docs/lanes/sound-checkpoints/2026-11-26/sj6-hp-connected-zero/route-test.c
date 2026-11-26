/* Same production provider function, with independently owned test arenas. */
#include "original_runtime.c"
#include <assert.h>
#include <stdio.h>
static original_runtime_part p[3];
#define original_bass p
#define NBASS 2
#include "engine-owner.inc"
static const float exact[8]={10.f,0.f,.44387006759643555f,0.f,0.f,0.f,0.f,1.f};
#define state original_sj6_zero_leaf
static void setup(unsigned a){
    juno6_t *s=&p[a].dsp.juno.synth;
    juno6_init(s,44100);
    juno6_set_sj6_hpf(s,0.f);
    p[a].dsp.juno.crush.active=0;
    p[a].dsp.juno.crush.step=1.f;
    juno6_chorus_init(&p[a].dsp.juno.chorus,44100);
    p[a].dsp.juno.chorus.factory_mode=JUNO6_CHORUS_OFF;
    p[a].status=ORIGINAL_RUNTIME_OK;p[a].kind=ORIGINAL_SOUND_JUNO;
}
static void signal(float *l,float *r,unsigned n){
    unsigned x=0x48276db3u;
    for(unsigned i=0;i<n;i++){
        x=x*1664525u+1013904223u;l[i]=(float)((int32_t)(x>>16)-32768)*0x1p-16f;
        x=x*1664525u+1013904223u;r[i]=(float)((int32_t)(x>>16)-32768)*0x1p-16f;
    }
}
int main(void){
    for(unsigned i=0;i<3;i++)setup(i);
    float l[128],r[128],a[128],b[128],c[128];
    sj6_zero_fx_channel ref[2]={{0},{0}}, frozen[2],zero[2]={{0},{0}};
    assert(!sj6_zero_leaf_admit(p));
    float wrong[8]; memcpy(wrong,exact,sizeof wrong);
    for(unsigned i=0;i<8;i++){
        wrong[i]=(i==7?0.f:1.f);
        assert(!engine_sj6_zero_pin(p,wrong) && !sj6_zero_leaf_admit(p));
        wrong[i]=exact[i];
    }
    assert(engine_sj6_zero_pin(p,exact) && engine_sj6_zero_pin(p+1,exact));
    assert(!engine_sj6_zero_pin(p+2,exact));
    assert(sj6_zero_leaf_admit(p)==state[0]);
    assert(sj6_zero_leaf_admit(p+1)==state[1]);
    assert(!sj6_zero_leaf_admit(p+2));
    signal(l,r,128);memcpy(a,l,sizeof l);memcpy(b,r,sizeof r);
    for(unsigned i=0;i<128;i++){
        a[i]=sj6_zero_crusher_44100(ref,sj6_hp10_44100(ref,a[i]));
        b[i]=sj6_zero_crusher_44100(ref+1,sj6_hp10_44100(ref+1,b[i]));
    }
    sj6_run_effects(p,l,r,128,NULL);
    assert(memcmp(a,l,sizeof a)==0 && memcmp(b,r,sizeof b)==0);
    assert(memcmp(ref,state[0],sizeof ref)==0 && memcmp(zero,state[1],sizeof zero)==0);
    memcpy(frozen,state[0],sizeof frozen);
    assert(!engine_sj6_zero_pin(p,NULL)); /* genuine no connected proof */
    signal(l,r,128); memcpy(c,l,sizeof c);sj6_run_effects(p,l,r,128,NULL);
    assert(memcmp(l,c,sizeof c)==0 && memcmp(frozen,state[0],sizeof frozen)==0);
    assert(engine_sj6_zero_pin(p,exact));
    p[0].dsp.juno.crush.active=1;p[0].dsp.juno.crush.step=.75f;
    p[2].dsp.juno.crush.active=1;p[2].dsp.juno.crush.step=.75f; /* distinct unowned legacy comparator */
    signal(l,r,128);signal(a,b,128);
    sj6_run_effects(p,l,r,128,NULL);sj6_run_effects(p+2,a,b,128,NULL);
    assert(memcmp(l,a,sizeof l)==0 && memcmp(r,b,sizeof r)==0);
    assert(memcmp(frozen,state[0],sizeof frozen)==0);
    p[0].dsp.juno.crush.active=0;p[0].dsp.juno.crush.step=1.f;
    juno6_set_sj6_hpf(&p[0].dsp.juno.synth,.2f);
    assert(!sj6_zero_leaf_admit(p));
    juno6_set_sj6_hpf(&p[0].dsp.juno.synth,0.f);
    p[0].dsp.juno.synth.core.sample_rate=48000.f;
    assert(!sj6_zero_leaf_admit(p));
    p[0].dsp.juno.synth.core.sample_rate=44100.f;
    signal(l,r,128);signal(a,b,128);
    for(unsigned i=0;i<128;i++){
        a[i]=sj6_zero_crusher_44100(ref,sj6_hp10_44100(ref,a[i]));
        b[i]=sj6_zero_crusher_44100(ref+1,sj6_hp10_44100(ref+1,b[i]));
    }
    sj6_run_effects(p,l,r,128,NULL);
    assert(memcmp(a,l,sizeof a)==0 && memcmp(b,r,sizeof b)==0);
    assert(memcmp(ref,state[0],sizeof ref)==0 && memcmp(zero,state[1],sizeof zero)==0);
    assert(original_runtime_all_off(p)==ORIGINAL_RUNTIME_OK);
    assert(sj6_zero_leaf_admit(p)==state[0] && memcmp(zero,state[0],sizeof zero)==0);
    assert(original_runtime_retire(p)==ORIGINAL_RUNTIME_OK && !engine_sj6_zero_leaf(p));
    assert(sj6_zero_leaf_admit(p+1)==state[1] && memcmp(zero,state[1],sizeof zero)==0);
    printf("signed route: 512 stereo float words exact; bypass/nonzero comparator, warm resume, unrelated owner and 48k refuse PASS\n");
}
/* Fixture never enters another backend. */
#include <stdlib.h>
int original_chicago_backend_retire(original_chicago_backend *p){(void)p;abort();}
