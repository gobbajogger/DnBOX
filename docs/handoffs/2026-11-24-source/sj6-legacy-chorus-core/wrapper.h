#pragma once
#include "classic.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    float pending,target,current,initial;
    uint64_t start;
    unsigned duration;
} classic_ramper;
typedef struct {
    classic_i core;
    classic_ramper p[4]; /* public AU frequency, depth, feedback, dryWetMix */
    unsigned started, mode_ii;
} classic_mode_i;
void classic_wrapper_prepare(classic_mode_i *,const float incoming[4],unsigned retained_ramp_frames);
void classic_wrapper_start(classic_mode_i *,int);
int classic_wrapper_command(classic_mode_i *,unsigned id,float unit);
void classic_wrapper_process(classic_mode_i *,uint64_t absolute_frame,float *,float *,size_t);
/* Separate source-verified legacy musical mode 2; never the private alternate AU renderer. */
void classic_wrapper_enter_mode_ii(classic_mode_i *);
