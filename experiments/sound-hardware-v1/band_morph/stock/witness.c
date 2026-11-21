#define _POSIX_C_SOURCE 200809L
#include "original_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define CHECK(e) do { if (!(e)) {fprintf(stderr,"line %d: %s\n",__LINE__,#e);exit(1);} } while (0)
static original_runtime_part a, b, copy;
static original_runtime_workspace work;
static uint8_t cell[32];
static float left[256], right[256];
static void init(original_runtime_part *p) { CHECK(original_runtime_init(p,cell,120,0,0)==1); }
static void note(original_runtime_part *p, unsigned key, unsigned vel)
{ CHECK(original_runtime_key(p,ORIGINAL_INPUT_KEYS,key,vel)==1); }
static void render(original_runtime_part *p)
{ CHECK(original_runtime_render(p,&work,left,right,256)==1); }
static uint64_t hash_samples(uint64_t hash) {
    for(unsigned i=0;i<256;i++) {
        uint32_t word[2]; memcpy(word,left+i,4); memcpy(word+1,right+i,4);
        CHECK(isfinite(left[i])&&isfinite(right[i]));
        hash=(hash^word[0])*UINT64_C(1099511628211);
        hash=(hash^word[1])*UINT64_C(1099511628211);
    }
    return hash;
}
static uint64_t stream(original_runtime_part *p) {
    uint64_t hash=1;
    for(unsigned block=0;block<24;block++) {
        if(block==0)note(p,60,100);
        if(block==16)note(p,60,0);
        render(p); hash=hash_samples(hash);
    }
    return hash;
}
#ifdef TEST_MORPH
static const s1_oscillator_field group[]={{0,.2f},{1,.8f},{2,.5f},{3,-2},{4,7},{5,.8f},{6,.8f},{146,1.f}};
static void set(original_runtime_part *p,const s1_oscillator_field *fields)
{ CHECK(original_runtime_s1_oscillators(p,fields,8)==1); }
static void invalid_cases(void) {
    s1_oscillator_field fields[8]; init(&a); set(&a,group); note(&a,61,127);
    render(&a);copy=a;
    for(unsigned i=0;i<8;i++) {
        memcpy(fields,group,sizeof fields);fields[i].physical=NAN;
        CHECK(original_runtime_s1_oscillators(&a,fields,8)==0 && !memcmp(&a,&copy,sizeof a));
        memcpy(fields,group,sizeof fields);fields[i].physical=25;
        CHECK(original_runtime_s1_oscillators(&a,fields,8)==0 && !memcmp(&a,&copy,sizeof a));
    }
    for(unsigned bad=0;bad<4;bad++) {
        memcpy(fields,group,sizeof fields);
        if(bad==0)fields[0].app_field=1;
        if(bad==1)fields[6].app_field=8;
        CHECK(original_runtime_s1_oscillators(&a,fields,bad==2?7:bad==3?9:8)==0);
        CHECK(!memcmp(&a,&copy,sizeof a));
    }
}
static void read_catalog(const char *path) {
    FILE *f=fopen(path,"rb"); CHECK(f);
    uint32_t header[2];CHECK(fread(header,4,2,f)==2&&header[0]==0x53315043&&header[1]==694);
    init(&a); init(&b); copy=b;
    CHECK(original_runtime_param(&a,0,.75f)==1 && original_runtime_param(&a,1,1)==1);
    note(&a,60,100);note(&a,60,0);render(&a);
    for(unsigned k=0;k<694;k++){
        uint32_t n;CHECK(fread(&n,4,1,f)==1&&n==144);
        s1_oscillator_field values[8]; unsigned count=0;
        for(unsigned j=0;j<n;j++) {
            s1_oscillator_field w; CHECK(fread(&w,4,2,f)==2);
            if(w.app_field<=6||w.app_field==146){CHECK(count<8);values[count++]=w;}
        }
        CHECK(count==8);set(&a,values);
        CHECK(a.dsp.synthone.bend==.75f&&a.dsp.synthone.damper&&a.dsp.synthone.voice[0].gate);
        render(&a);hash_samples(1);
    }
    CHECK(fgetc(f)==EOF);fclose(f);CHECK(!memcmp(&b,&copy,sizeof b));
}
static void signal_checks(void) {
    uint64_t baseline;
    init(&a); set(&a,group);baseline=stream(&a);
    for(unsigned id=0;id<8;id++) {
        s1_oscillator_field edit[8];memcpy(edit,group,sizeof edit);
        edit[id].physical=id==3||id==4?12.f:0.f;
        init(&a);set(&a,edit);CHECK(stream(&a)!=baseline);
    }
    init(&a);set(&a,group);note(&a,60,120); render(&a); copy=a;
    s1_oscillator_field rev[8];for(unsigned i=0;i<8;i++)rev[7-i]=group[i];
    set(&a,rev); CHECK(!memcmp(&a,&copy,sizeof a));
    init(&b);set(&b,group);
    CHECK(original_runtime_all_off(&a)==1);CHECK(stream(&a)==stream(&b));
    for(unsigned osc=0;osc<2;osc++) {
        s1_oscillator_field p[8]; memcpy(p,group,sizeof p);
        p[2].physical=(float)osc;
        init(&a);set(&a,p);uint64_t heard=stream(&a);
        p[1-osc].physical=0.f;p[3+1-osc].physical=24;p[5+1-osc].physical=0.f;
        init(&b);set(&b,p);CHECK(stream(&b)==heard);
    }
}
#endif
static double ns(clockid_t clock) {
    struct timespec t;CHECK(clock_gettime(clock,&t)==0);
    return (double)t.tv_sec*1e9+t.tv_nsec;
}
static void timing(unsigned key, unsigned enabled) {
    init(&a);init(&b);
#ifdef TEST_MORPH
    if(enabled){
        /* Nearby different offsets keep all 24 sources active at the target
         * band. Two adjacent mixed morph segments per voice. */
        s1_oscillator_field p[8];memcpy(p,group,sizeof p);
        p[3].physical=-12;p[4].physical=-11;
        p[7].physical=(float)(enabled==1);set(&a,p);p[0].physical=.5f;p[1].physical=.16f;set(&b,p);
    }
#else
    (void)enabled;
#endif
    for(unsigned i=0;i<6;i++){note(&a,key+i,96+i);note(&b,key+i,100+i);}
    double totalcpu=0, maxcpu=0, maxwall=0;unsigned miss=0;uint64_t hash=1;
    for(unsigned block=0;block<144;block++) {
        double c=ns(CLOCK_THREAD_CPUTIME_ID),w=ns(CLOCK_MONOTONIC);
        for(unsigned half=0;half<2;half++){render(&a);render(&b);}
        c=(ns(CLOCK_THREAD_CPUTIME_ID)-c)/1e3;w=(ns(CLOCK_MONOTONIC)-w)/1e3;
        if(block>=16){totalcpu+=c;if(c>maxcpu)maxcpu=c;if(w>maxwall)maxwall=w;miss+=w>=512000000./44100.;}
        hash=hash_samples(hash);
    }
    printf("timing %u %u frames=512 blocks=128 parts=2 voices=12 mean_cpu_us=%.3f max_cpu_us=%.3f max_wall_us=%.3f deadline_misses=%u hash=%llx\n",enabled,key,totalcpu/128,maxcpu,maxwall,miss,(unsigned long long)hash);
}
int main(int argc,char **argv) {
    CHECK(argc==2);CHECK(original_sound_init(cell,ORIGINAL_SOUND_SYNTHONE,0)==1);init(&a);
    printf("init_pcm=%llx runtime_bytes=%zu s1_bytes=%zu\n",(unsigned long long)stream(&a),sizeof a,sizeof a.dsp.synthone);
#ifdef TEST_MORPH
    invalid_cases();read_catalog(argv[1]);signal_checks();puts("catalog=694 rows=5552 aliases=0 all_8_fields_audible=1 inactive_osc_isolation=2 invalid_no_mutation=20 panic_patch_preserved=1");
#else
    (void)argv;
#endif
    for(unsigned i=0;i<3;i++){unsigned key=i==0?0:i==1?60:106;timing(key,0);
#ifdef TEST_MORPH
        timing(key,1);timing(key,2);
#endif
    }
    return 0;
}
