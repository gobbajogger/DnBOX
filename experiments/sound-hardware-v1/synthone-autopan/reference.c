/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/original_runtime.h"
#include "soundpipe.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    original_runtime_part wet, dry;
    sp_port rate_port, depth_port;
    sp_osc osc;
    sp_pan2 pan;
    float rate, depth;
    unsigned enabled, mix, source_frames, dry_frames, transition_frames, silent_frames;
    double peak_error, sum_squared_error, wrong_linear_error;
} owner;

static owner parts[2];
static original_runtime_workspace workspace;

static void parameter(owner *p, unsigned id, unsigned position)
{
    assert(original_runtime_param(&p->wet, id, (float)position / 127.f) == ORIGINAL_RUNTIME_OK);
    if (id == ORIGINAL_SYNTHONE_PAN_RATE) {
        float minimum = 1.f / 1920.f;
        p->rate = position <= 3 ? minimum + (.25f - minimum) * (float)position / 3.f
                                : .25f + (float)(position - 3) * (9.75f / 124.f);
    } else {
        p->enabled = position != 0;
        p->depth = position ? (float)(position - 1) / 126.f : 0.f;
    }
}

static void note(owner *p, unsigned key, unsigned velocity)
{
    assert(original_runtime_key(&p->wet, ORIGINAL_INPUT_KEYS, key, velocity) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_key(&p->dry, ORIGINAL_INPUT_KEYS, key, velocity) == ORIGINAL_RUNTIME_OK);
}

static void score(owner *p, sp_data *sp, unsigned frames)
{
    float left[256], right[256], input[256], unused[256];
    assert(original_runtime_render(&p->wet, &workspace, left, right, frames) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_render(&p->dry, &workspace, input, unused, frames) == ORIGINAL_RUNTIME_OK);
    for (unsigned i = 0; i < frames; ++i) {
        float l, r, value;
        sp_port_compute(sp,&p->rate_port,&p->rate,&p->osc.freq);
        sp_port_compute(sp,&p->depth_port,&p->depth,&p->osc.amp);
        sp_osc_compute(sp,&p->osc,NULL,&value);
        p->pan.pan = value;
        sp_pan2_compute(sp,&p->pan,&input[i],&l,&r);
        if (p->enabled) { if (p->mix < 64) ++p->mix; }
        else if (p->mix) --p->mix;
        if (p->mix == 64) {
            ++p->source_frames;
            p->wrong_linear_error = fmax(p->wrong_linear_error, fabs(left[i] - input[i] * (.5f - .5f * value)));
        } else if (p->mix == 0) {
            ++p->dry_frames;
            assert(left[i] == input[i] && right[i] == input[i]);
            l = r = input[i];
        } else {
            ++p->transition_frames;
            float mix = (float)p->mix / 64.f;
            l = input[i] + mix * (l - input[i]);
            r = input[i] + mix * (r - input[i]);
        }
        if (input[i] == 0.f) {
            ++p->silent_frames;
            assert(left[i] == 0.f && right[i] == 0.f);
        }
        double el = (double)left[i] - l, er = (double)right[i] - r;
        assert(isfinite(el) && isfinite(er));
        p->peak_error = fmax(p->peak_error, fmax(fabs(el),fabs(er)));
        p->sum_squared_error += el*el + er*er;
    }
    /* Every block boundary includes the retired/silent intervals. */
    assert(p->wet.dsp.synthone.pan.phase == (unsigned)p->osc.lphs);
    assert(p->wet.dsp.synthone.pan.rate == p->rate_port.yt1);
    assert(p->wet.dsp.synthone.pan.depth == p->depth_port.yt1);
}

static void factory(sp_data *sp, sp_ftbl *sine, const uint8_t cell[32], const char *controls)
{
    FILE *input=fopen(controls,"r");
    assert(input);
    unsigned programs=0, nonzero=0, different=0, worst_program=0;
    double source_peak=0., panel_peak=0., panel_rms_worst=0.;
    float depth, rate;
    while (fscanf(input,"%a %a",&depth,&rate)==2) {
        owner *p=&parts[0];
        memset(p,0,sizeof *p);
        assert(original_runtime_init(&p->wet,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
        assert(original_runtime_init(&p->dry,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
        original_runtime_part *panel=&parts[1].wet;
        assert(original_runtime_init(panel,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
        sp_port_init(sp,&p->rate_port,.1f);
        sp_port_init(sp,&p->depth_port,.1f);
        sp_osc_init(sp,&p->osc,sine,0.f);
        sp_pan2_init(sp,&p->pan);
        /* The original full-resolution targets exercise the shipping stage
         * directly. A second instance loads its nearest physical panel values. */
        p->wet.dsp.synthone.pan.engaged=1;
        p->wet.dsp.synthone.pan.depth_target=p->depth=depth;
        p->wet.dsp.synthone.pan.rate_target=p->rate=rate;
        unsigned nearest=0;
        for(unsigned q=1;q<128;++q)
            if (fabsf(s1_pan_hz(q)-rate)<fabsf(s1_pan_hz(nearest)-rate)) nearest=q;
        unsigned amount=1u+(unsigned)(depth*126.f+.5f);
        assert(original_runtime_param(panel,ORIGINAL_SYNTHONE_PAN,(float)amount/127.f)==ORIGINAL_RUNTIME_OK);
        assert(original_runtime_param(panel,ORIGINAL_SYNTHONE_PAN_RATE,(float)nearest/127.f)==ORIGINAL_RUNTIME_OK);
        different += panel->dsp.synthone.pan.depth_target!=depth || panel->dsp.synthone.pan.rate_target!=rate;
        nonzero += depth>0;
        note(p,60,110);
        assert(original_runtime_key(panel,ORIGINAL_INPUT_KEYS,60,110)==ORIGINAL_RUNTIME_OK);
        double sum=0.;
        for (unsigned at=0;at<44100;) {
            unsigned n=44100-at>64 ? 64 : 44100-at;
            float l[64], r[64], pl[64], pr[64], mono[64], other[64];
            assert(original_runtime_render(&p->wet,&workspace,l,r,n)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_render(&p->dry,&workspace,mono,other,n)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_render(panel,&workspace,pl,pr,n)==ORIGINAL_RUNTIME_OK);
            for(unsigned i=0;i<n;++i) {
                float pan, sl, sr;
                sp_port_compute(sp,&p->rate_port,&rate,&p->osc.freq);
                sp_port_compute(sp,&p->depth_port,&depth,&p->osc.amp);
                sp_osc_compute(sp,&p->osc,NULL,&pan);
                p->pan.pan=pan;
                sp_pan2_compute(sp,&p->pan,&mono[i],&sl,&sr);
                if(at+i<63) {
                    float mix=(float)(at+i+1)/64.f;
                    sl=mono[i]+mix*(sl-mono[i]); sr=mono[i]+mix*(sr-mono[i]);
                }
                double source=fmax(fabs((double)l[i]-sl),fabs((double)r[i]-sr));
                double el=(double)pl[i]-sl, er=(double)pr[i]-sr;
                assert(isfinite(source) && isfinite(el) && isfinite(er));
                source_peak=fmax(source_peak,source);
                panel_peak=fmax(panel_peak,fmax(fabs(el),fabs(er)));
                sum+=el*el+er*er;
            }
            assert(p->wet.dsp.synthone.pan.phase==(unsigned)p->osc.lphs);
            at+=n;
        }
        double rms=sqrt(sum/(2.*44100.));
        if(rms>panel_rms_worst) {panel_rms_worst=rms;worst_program=programs;}
        ++programs;
    }
    fclose(input);
    printf("programs=%u nonzero_depth=%u frames=%u source_stage_peak=%.9g panel_rounded=%u panel_peak=%.9g panel_worst_rms=%.9g panel_worst_index=%u\n",
        programs,nonzero,programs*44100u,source_peak,different,panel_peak,panel_rms_worst,worst_program);
    assert(programs==694 && nonzero==342 && source_peak<1.e-6);
}

int main(int argc, char **argv)
{
    uint8_t cell[32];
    sp_data sp = {.sr=44100,.nchan=2};
    sp_ftbl *sine = NULL;
    sp_ftbl_create(&sp,&sine,4096);
    sp_gen_sine(&sp,sine);
    assert(original_sound_init(cell,ORIGINAL_SOUND_SYNTHONE,0));
    for (unsigned v=0; v<2; ++v) {
        owner *p = &parts[v];
        assert(original_runtime_init(&p->wet,cell,120,NULL,NULL) == ORIGINAL_RUNTIME_OK);
        assert(original_runtime_init(&p->dry,cell,120,NULL,NULL) == ORIGINAL_RUNTIME_OK);
        sp_port_init(&sp,&p->rate_port,.1f);
        sp_port_init(&sp,&p->depth_port,.1f);
        sp_osc_init(&sp,&p->osc,sine,0.f);
        sp_pan2_init(&sp,&p->pan);
        p->rate = .25f;
    }
    for (unsigned block=0; block<2200; ++block)
        for (unsigned v=0; v<2; ++v) {
            owner *p = &parts[v];
            if (block == 1) note(p,48 + 13*v,113 - 23*v);
            if (block == 70) parameter(p,ORIGINAL_SYNTHONE_PAN,1); /* explicit source center */
            if (block == 160) parameter(p,ORIGINAL_SYNTHONE_PAN_RATE,v ? 7 : 127);
            if (block == 200) parameter(p,ORIGINAL_SYNTHONE_PAN,v ? 79 : 127);
            if (block == 420) parameter(p,ORIGINAL_SYNTHONE_PAN_RATE,v ? 119 : 0);
            if (block == 452) parameter(p,ORIGINAL_SYNTHONE_PAN,v ? 14 : 83);
            if (block == 600) note(p,48+13*v,0);
            if (block == 1100) {
                assert(original_runtime_all_off(&p->wet) == ORIGINAL_RUNTIME_OK);
                assert(original_runtime_all_off(&p->dry) == ORIGINAL_RUNTIME_OK);
            }
            if (block == 1200) parameter(p,ORIGINAL_SYNTHONE_PAN_RATE,v ? 127 : 64);
            if (block == 1450) note(p,72 - 5*v,104);
            if (block == 1600) parameter(p,ORIGINAL_SYNTHONE_PAN,0); /* return to unity */
            if (block == 1800) parameter(p,ORIGINAL_SYNTHONE_PAN,v ? 115 : 32);
            score(p,&sp,1 + (block * 19u + v * 23u) % 128u);
        }
    for (unsigned v=0; v<2; ++v) {
        owner *p=&parts[v];
        unsigned n=p->source_frames+p->dry_frames+p->transition_frames;
        printf("owner=%c frames=%u source_frames=%u dry_frames=%u transition_frames=%u silent_frames=%u peak_error=%.9g rms_error=%.9g wrong_linear_peak=%.9g part_bytes=%zu union_bytes=%zu\n",
            'A'+v,n,p->source_frames,p->dry_frames,p->transition_frames,p->silent_frames,p->peak_error,
            sqrt(p->sum_squared_error / (2*n)),p->wrong_linear_error,sizeof(s1_dry_part),sizeof(original_runtime_backend));
        assert(p->source_frames>90000 && p->dry_frames>15000 && p->silent_frames>10000);
        assert(p->peak_error<1.e-6 && p->wrong_linear_error>.015);
    }
    if (argc==2) factory(&sp,sine,cell,argv[1]);
    sp_ftbl_destroy(&sine);
    return 0;
}
