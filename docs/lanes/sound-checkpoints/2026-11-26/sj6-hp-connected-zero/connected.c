#include "original_runtime.c"
static original_runtime_part p[2];
#define original_bass p
#define NBASS 2
#include "engine-owner.inc"
static const float exact[]={10.f,0.f,.44387006759643555f,0.f,0.f,0.f,0.f,1.f};
int route_begin(unsigned active_exact)
{
    original_runtime_part *o=p;
    memset(o,0,sizeof *o);
    engine_sj6_zero_clear(o,1);
    o->kind=ORIGINAL_SOUND_JUNO;o->status=ORIGINAL_RUNTIME_OK;
    juno6_init(&o->dsp.juno.synth,44100.f);
    juno6_set_sj6_hpf(&o->dsp.juno.synth,0.f);
    juno6_chorus_init(&o->dsp.juno.chorus,44100.f);
    o->dsp.juno.chorus.factory_mode=JUNO6_CHORUS_OFF;
    o->dsp.juno.crush.step=1.f;o->dsp.juno.crush.active=0;
    return !active_exact || engine_sj6_zero_pin(o,exact);
}
void route_span(float *l,float *r,unsigned frames)
{ sj6_run_effects(p,l,r,frames,NULL); }
