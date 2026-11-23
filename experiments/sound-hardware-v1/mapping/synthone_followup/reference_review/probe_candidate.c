/* Diagnostic against an existing, independently implemented candidate.
 * No original S1 or SJ6 engine source is linked. No production writes. */
#include "dnbx_core.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

static void init(dnbx_part *p,float sustain,unsigned velocity,unsigned mode) {
    dnbx_layer_params controls;
    assert(dnbx_init(p,44100)==0);
    dnbx_default_layer_params(&controls);
    controls.amp=(dnbx_envelope_params){0,0,0,sustain,0,0};
    controls.filter=(dnbx_envelope_params){0,0,0,0,0,0};
    controls.waveform=2.f/3.f; controls.oscillator_mode=DNBX_OSC_RAW;
    controls.cutoff_hz=500;controls.resonance=0;controls.filter_env_hz=0;
    controls.sub_gain=0;controls.gain=1;controls.pan=-1;
    controls.lfo_hz=0;controls.lfo_pwm_depth=0;
    assert(dnbx_set_layer(p,0,&controls)==0);
    controls.gain=0;assert(dnbx_set_layer(p,1,&controls)==0);
    assert(dnbx_set_velocity_mode(p,mode)==0);
    assert(dnbx_note_on(p,60,velocity)>=0);
}

int main(void) {
    dnbx_part full,quarter,low;float lf,rf,lq,rq,ll,rl;
    init(&full,1,127,DNBX_VELOCITY_SQUARED);
    init(&quarter,.25f,127,DNBX_VELOCITY_SQUARED);
    init(&low,1,20,DNBX_VELOCITY_SQUARED);
    assert(dnbx_render(&full,&lf,&rf,1,0)==0);
    assert(dnbx_render(&quarter,&lq,&rq,1,0)==0);
    assert(dnbx_render(&low,&ll,&rl,1,0)==0);
    const float fh=full.voice[0].layer[0].filter_second;
    const float qh=quarter.voice[0].layer[0].filter_second;
    const float lh=low.voice[0].layer[0].filter_second;
    assert(fh!=0 && fh==qh && fh==lh);
    assert(fabsf(lq/lf-.25f)<1e-6f);
    assert(fabsf(ll/lf-(20.f/127)*(20.f/127))<1e-6f);
    dnbx_layer_params controls;dnbx_default_layer_params(&controls);
    controls.cutoff_hz=22050;
    int cutoff_status=dnbx_set_layer(&full,0,&controls);
    assert(cutoff_status==DNBX_E_RANGE);
    printf("{\"probe_passed\":true,\"scope\":\"Existing candidate routing diagnosis; not original audio parity\","
           "\"amp_quarter_output_ratio\":%.9g,\"amp_quarter_filter_state_ratio\":%.9g,"
           "\"velocity20_output_ratio\":%.9g,\"velocity20_filter_state_ratio\":%.9g,"
           "\"s1_max_cutoff_22050_status\":%d,\"part_bytes\":%zu}\n",
           lq/lf,qh/fh,ll/lf,lh/fh,cutoff_status,sizeof(dnbx_part));
    return 0;
}
