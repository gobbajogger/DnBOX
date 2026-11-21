#define _POSIX_C_SOURCE 200809L
#include "dnbx_core.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* This is the entire selected candidate workload, not firmware audio_half_render.
 * Both candidate parts, all twelve voices and24 layer histories are caller owned.
 * Native effects, drums, UAC, output mixing and interrupts are outside this probe. */
#define ITERATIONS 2000
static volatile float observable;
typedef struct { dnbx_part part[2]; dnbx_layer_params params[2][2]; } workload;
static float left[2][256], right[2][256];

#ifndef REVIEW_TRACE
static uint64_t nanos(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW,&ts);
    return (uint64_t)ts.tv_sec*1000000000u+(uint64_t)ts.tv_nsec;
}
static int cmp_u64(const void *a,const void *b) {
    uint64_t x=*(const uint64_t *)a,y=*(const uint64_t *)b;
    return (x>y)-(x<y);
}
#endif
static void setup(workload *x,unsigned scenario) {
    memset(x,0,sizeof *x);
    for (unsigned p=0;p<2;p++) {
        dnbx_init(&x->part[p],44100);
        for (unsigned l=0;l<2;l++) {
            dnbx_layer_params *q=&x->params[p][l];
            dnbx_default_layer_params(q);
            q->amp=(dnbx_envelope_params){0,0,0,1,0,.0001f};
            q->filter=(dnbx_envelope_params){0,0,0,1,0,.0001f};
            q->waveform=scenario==1 ? 1.0f/6.0f : 5.0f/6.0f;
            q->gain=4;q->sub_gain=.5f;q->cutoff_hz=20000;q->resonance=1;
            q->filter_env_hz=20000;q->lfo_hz=16;q->lfo_pwm_depth=.47f;
            q->tune_semitones=48;q->pan=(l?1.f:-1.f);
            dnbx_set_layer(&x->part[p],l,q);
        }
        for (unsigned i=0;i<6;i++) dnbx_note_on(&x->part[p],60+i,127);
    }
}
static void burst(workload *x,unsigned i) {
    for (unsigned p=0;p<2;p++) {
        for (unsigned l=0;l<2;l++) {
            dnbx_layer_params *q=&x->params[p][l];
            q->amp.attack_s=1.0f/44100;q->amp.decay_s=1.0f/44100;
            q->filter.attack_s=1.0f/44100;q->filter.decay_s=1.0f/44100;
            q->amp.sustain=.25f+(i%4)*.25f;q->filter.sustain=.25f;
            q->cutoff_hz=20.f+(i%2)*19980.f;
            dnbx_set_layer(&x->part[p],l,q);
        }
        for (unsigned id=0;id<DNBX_RAW_CONTROLS;id++)
            dnbx_store_raw(&x->part[p],id,(float)id*1.32f-.008403362f);
        dnbx_set_sustain(&x->part[p],1);
        for (unsigned j=0;j<8;j++) {
            unsigned note=60+(j+i)%8;
            dnbx_note_off(&x->part[p],note);
            dnbx_note_on(&x->part[p],note,127);
        }
        dnbx_set_sustain(&x->part[p],0);
        dnbx_set_velocity_mode(&x->part[p],i%2);
    }
}
static void callback(workload *x,unsigned scenario,unsigned frames,unsigned i,dnbx_work *w) {
    if(scenario==2) burst(x,i);
    for(unsigned pos=0;pos<frames;pos+=256) {
        unsigned n=frames-pos<256?frames-pos:256;
        for(unsigned p=0;p<2;p++) {
            dnbx_render(&x->part[p],left[p],right[p],n,w);
            observable+=left[p][n-1]*.000001f+right[p][n-1]*.000001f;
        }
    }
}
static void print_work(const char *name,const dnbx_work *w) {
    printf("\"%s\":{\"frames\":%"PRIu64",\"voice_slots\":%"PRIu64",\"active_voice_samples\":%"PRIu64",\"layer_samples\":%"PRIu64",\"envelope_samples\":%"PRIu64",\"envelope_ticks\":%"PRIu64",\"stage_transitions\":%"PRIu64",\"exp2_calls\":%"PRIu64",\"log2_calls\":%"PRIu64",\"small_exp_calls\":%"PRIu64",\"lfo_samples\":%"PRIu64",\"blep_calls\":%"PRIu64",\"sine_calls\":%"PRIu64",\"maximum_transition_chain\":%u}",name,w->frames,w->voice_slots,w->active_voice_samples,w->layer_samples,w->envelope_samples,w->envelope_ticks,w->stage_transitions,w->exp2_calls,w->log2_calls,w->small_exp_calls,w->lfo_samples,w->blep_calls,w->sine_calls,w->maximum_transition_chain);
}
#ifdef REVIEW_TRACE
extern void review_reset(void);
extern void review_get_trace(dnbx_work *,uint64_t *);
#endif
int main(void) {
    static workload x;
#ifdef REVIEW_TRACE
    setup(&x,2);dnbx_work render={0},total={0};uint64_t math_calls[2];
    review_reset();callback(&x,2,512,0,&render);review_get_trace(&total,math_calls);
    printf("{");print_work("public_render_counters",&render);printf(",");print_work("trace_all_count_sites",&total);
    printf(",\"trace_direct_math\":{\"exp2_calls\":%"PRIu64",\"log2_calls\":%"PRIu64"},\"observable\":%.9g}\n",math_calls[0],math_calls[1],(double)observable);
#else
    const char *names[]={"max_blep_steady","sine_triangle_steady","max_blep_event_control_burst"};
    uint64_t elapsed[ITERATIONS];int first=1;
    printf("{\"timer\":\"CLOCK_MONOTONIC_RAW\",\"iterations_per_case\":%u,\"warmup_callbacks\":64,\"sample_rate\":44100,\"parts\":2,\"voices_per_part\":6,\"layers_per_voice\":2,\"render_chunk_frames\":256,\"cases\":[",ITERATIONS);
    for(unsigned frames=256;frames<=512;frames*=2) for(unsigned scenario=0;scenario<3;scenario++) for(unsigned counted=0;counted<2;counted++) {
        setup(&x,scenario);dnbx_work w={0};
        for(unsigned i=0;i<64;i++) callback(&x,scenario,frames,i,counted?&w:NULL);
        memset(&w,0,sizeof w);
        for(unsigned i=0;i<ITERATIONS;i++) {
            uint64_t t=nanos();callback(&x,scenario,frames,i,counted?&w:NULL);elapsed[i]=nanos()-t;
        }
        qsort(elapsed,ITERATIONS,sizeof elapsed[0],cmp_u64);
        if(!first)printf(",");
        first=0;
        printf("{\"scenario\":\"%s\",\"frames_per_part\":%u,\"counters_enabled\":%s,\"observed_min_ns\":%"PRIu64",\"median_ns\":%"PRIu64",\"p95_ns\":%"PRIu64",\"p99_ns\":%"PRIu64",\"observed_max_ns\":%"PRIu64",",names[scenario],frames,counted?"true":"false",elapsed[0],elapsed[ITERATIONS/2],elapsed[ITERATIONS*95/100],elapsed[ITERATIONS*99/100],elapsed[ITERATIONS-1]);
        print_work("accumulated_render_work",&w);printf("}");
    }
    printf("],\"observable\":%.9g}\n",(double)observable);
#endif
    return !isfinite(observable);
}
