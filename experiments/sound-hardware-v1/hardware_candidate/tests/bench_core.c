/* SPDX-License-Identifier: GPL-3.0-only */
#define _POSIX_C_SOURCE 200809L
#include "dnbx_core.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { PARTS=2, CHUNK=256, FRAMES=512, RUNS=2000 };
static dnbx_part part[PARTS];
static float left[PARTS][CHUNK],right[PARTS][CHUNK];
static double checksum;

static uint64_t now(clockid_t clock)
{
    struct timespec t;
    if (clock_gettime(clock,&t)) { perror("clock_gettime"); exit(1); }
    return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;
}
static void need(int result)
{ if (result<0) { fprintf(stderr,"benchmark API failure %d\n",result); exit(1); } }
static dnbx_layer_params parameters(unsigned scenario)
{
    dnbx_layer_params p; dnbx_default_layer_params(&p);
    memset(&p.amp,0,sizeof p.amp); memset(&p.filter,0,sizeof p.filter);
    p.amp.sustain=p.filter.sustain=1;
    p.waveform=.85f; p.pulse_width=.03f; p.lfo_hz=16; p.lfo_pwm_depth=.47f;
    p.gain=4; p.sub_gain=.5f; p.cutoff_hz=20; p.resonance=1; p.filter_env_hz=20000;
    if (scenario==1) p.oscillator_mode=DNBX_OSC_RAW;
    if (scenario==2) p.amp.attack_s=p.filter.attack_s=16;
    if (scenario==3) {
        p.amp.attack_s=p.filter.attack_s=1.0f/44100;
        p.amp.hold_s=p.filter.hold_s=1.0f/44100;
        p.amp.decay_s=p.filter.decay_s=1.0f/44100;
        p.amp.sustain=p.filter.sustain=.4f;
    }
    return p;
}
static void events(unsigned scenario)
{
    static const unsigned note[6]={0,36,60,84,108,127};
    dnbx_layer_params p=parameters(scenario);
    for (unsigned k=0;k<PARTS;k++) {
        need(dnbx_all_off(&part[k]));
        p.tune_semitones=k?-48:48;
        for (unsigned layer=0;layer<2;layer++) need(dnbx_set_layer(&part[k],layer,&p));
        for (unsigned v=0;v<6;v++) need(dnbx_note_on(&part[k],note[v],127));
    }
}
static void callback(unsigned scenario,dnbx_work *w)
{
    /* Scenario3 includes all-off, four full layer-control applications and
     * twelve note-ons inside the timed callback, plus the same 512 frames. */
    if (scenario==3) events(scenario);
    for (unsigned block=0;block<FRAMES/CHUNK;block++)
        for (unsigned k=0;k<PARTS;k++) need(dnbx_render(&part[k],left[k],right[k],CHUNK,w));
    checksum+=left[0][0]+right[1][CHUNK-1];
}
static void print_work(const dnbx_work *w)
{
    printf("{\"frames\":%"PRIu64",\"voice_slots\":%"PRIu64",\"active_voice_samples\":%"PRIu64
           ",\"layer_samples\":%"PRIu64",\"envelope_samples\":%"PRIu64",\"envelope_ticks\":%"PRIu64
           ",\"stage_transitions\":%"PRIu64",\"exp2_calls\":%"PRIu64",\"log2_calls\":%"PRIu64
           ",\"small_exp_calls\":%"PRIu64",\"lfo_samples\":%"PRIu64",\"blep_calls\":%"PRIu64
           ",\"sine_calls\":%"PRIu64",\"maximum_transition_chain\":%u}",
           w->frames,w->voice_slots,w->active_voice_samples,w->layer_samples,w->envelope_samples,
           w->envelope_ticks,w->stage_transitions,w->exp2_calls,w->log2_calls,w->small_exp_calls,
           w->lfo_samples,w->blep_calls,w->sine_calls,w->maximum_transition_chain);
}
int main(void)
{
    static const char *names[]={"two_parts_steady_stock_blep","two_parts_steady_raw",
                               "two_parts_long_curved_stock_blep","two_parts_control_note_transition_burst"};
    printf("{\"scope\":\"host wall and calling-thread CPU observations; not device timing\","
           "\"sample_rate\":44100,\"callback_frames\":%u,\"subblock_frames\":%u,\"parts\":%u,"
           "\"voices_per_part\":6,\"layers_per_voice\":2,\"measured_callbacks_per_case\":%u,"
           "\"deadline_ns\":%.6f,\"clock_calls_included_in_measurements\":true,"
           "\"semantic_counters_exclude_events_and_controls\":true,\"cases\":[",
           FRAMES,CHUNK,PARTS,RUNS,1e9*FRAMES/44100.0);
    for (unsigned scenario=0;scenario<4;scenario++) {
        uint64_t wall_max=0,cpu_max=0,wall_total=0,cpu_total=0;
        unsigned wall_index=0,cpu_index=0;
        for (unsigned k=0;k<PARTS;k++) need(dnbx_init(&part[k],44100));
        events(scenario);
        for (unsigned warm=0;warm<20;warm++) callback(scenario,0);
        for (unsigned run=0;run<RUNS;run++) {
            if (scenario==2 && run%256==0) events(scenario);
            uint64_t ws=now(CLOCK_MONOTONIC),cs=now(CLOCK_THREAD_CPUTIME_ID);
            callback(scenario,0);
            uint64_t ce=now(CLOCK_THREAD_CPUTIME_ID),we=now(CLOCK_MONOTONIC);
            uint64_t wd=we-ws,cd=ce-cs;
            if (wd>wall_max) { wall_max=wd; wall_index=run; }
            if (cd>cpu_max) { cpu_max=cd; cpu_index=run; }
            wall_total+=wd; cpu_total+=cd;
        }
        dnbx_work work={0}; events(scenario); callback(scenario,&work);
        if (scenario) putchar(',');
        printf("{\"name\":\"%s\",\"includes_control_note_burst\":%s,"
               "\"max_wall_ns\":%"PRIu64",\"max_wall_callback_index\":%u,"
               "\"max_thread_cpu_ns\":%"PRIu64",\"max_thread_cpu_callback_index\":%u,"
               "\"mean_wall_ns\":%.3f,\"mean_thread_cpu_ns\":%.3f,\"one_callback_render_work\":",
               names[scenario],scenario==3?"true":"false",wall_max,wall_index,cpu_max,cpu_index,
               (double)wall_total/RUNS,(double)cpu_total/RUNS);
        print_work(&work); putchar('}');
    }
    printf("],\"checksum\":%.17g}\n",checksum);
    return 0;
}
