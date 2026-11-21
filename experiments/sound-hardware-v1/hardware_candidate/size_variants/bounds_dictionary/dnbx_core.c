/* SPDX-License-Identifier: GPL-3.0-only */
/* Clean analytic candidate using permitted DnBOX juno6.c oscillator, two-pole
 * filter, and allocator primitives plus permitted DnBOX fastmath.h. No original
 * SuperJ6 engine source or earlier adapted envelope was used.
 *
 * The generic finite-time curved envelope is independently derived from the
 * numeric capture endpoints: a segment approaches an asymptote beyond its
 * endpoint, with retention chosen to reach that endpoint in a stated duration.
 * See evidence/envelope_fit.json and README.md for observations and limits. */
#include "dnbx_core.h"
#include "fastmath.h"
#include <string.h>

#define DNBX_MAGIC 0x444e4258u
#define ATTACK_OVERSHOOT 0.2231306f
#define FALL_OVERSHOOT 0.00708345f
#if DNBX_DIAGNOSTIC_COUNTERS
#define COUNT(w, member, value) do { if (w) (w)->member += (value); } while (0)
#else
#define COUNT(w, member, value) do { (void)(w); } while (0)
#endif

_Static_assert(sizeof(float) == 4, "ABI requires binary32-sized float");
_Static_assert(sizeof(dnbx_envelope_state) == 36, "envelope ABI changed");
_Static_assert(sizeof(dnbx_layer_params) == 96, "parameter ABI changed");

static int finite_float(float x)
{
    fm_bits_t v; v.f = x;
    return (v.u & 0x7f800000u) != 0x7f800000u;
}
static int range(float x, float lo, float hi)
{ return x >= lo && x <= hi; }
static int valid(const dnbx_part *s)
{ return s && s->magic == DNBX_MAGIC; }
static uint32_t sample_count(float seconds, float sr)
{
    uint32_t n = (uint32_t)(seconds * sr + .5f);
    return seconds > 0.0f && n == 0 ? 1u : n;
}
/* Strict bounded comparisons reject NaN and infinity without a duplicate
 * exponent-bit test. Each physical field retains its own explicit units/range;
 * this is validation only, never an all-controls normalized clamp. */
static int params_valid(const dnbx_layer_params *p)
{
    /* Reuse identical physical ranges without changing their binary32 bounds.
     * Field order remains guarded by the offset assertion below. */
    static const float bounds[10][2]={
        {0,16},{0,1},{.03f,.5f},{0,4},{0,.5f},
        {20,20000},{-20000,20000},{0,.47f},{-48,48},{-1,1}
    };
    static const uint8_t field_range[23]={
        0,0,0,1,0,0,0,0,0,1,0,0,1,2,3,4,5,1,6,0,7,8,9
    };
    _Static_assert(offsetof(dnbx_layer_params,oscillator_mode)==23*sizeof(float),
                   "parameter fields must match typed bounds table");
    if (!p || p->oscillator_mode>DNBX_OSC_RAW) return 0;
    for (unsigned i=0;i<23;i++) {
        float value;
        memcpy(&value,(const unsigned char *)p+i*sizeof(float),sizeof value);
        const float *limit=bounds[field_range[i]];
        if (!range(value,limit[0],limit[1])) return 0;
    }
    return 1;
}
static int voice_active(const dnbx_voice *v)
{
    return v->layer[0].amp.stage || v->layer[0].filter.stage ||
           v->layer[1].amp.stage || v->layer[1].filter.stage;
}
static float increment(unsigned note, float tune, float sr)
{
    return fm_clampf(440.0f * fm_exp2f(((float)note-69.0f+tune)/12.0f)/sr,
                     .000001f,.45f);
}

/* DnBOX's existing polynomial BLEP primitive. */
static float blep(float t, float dt, dnbx_work *w)
{
    COUNT(w,blep_calls,1);
    if (t < dt) { t /= dt; return t+t-t*t-1.0f; }
    if (t > 1.0f-dt) { t=(t-1.0f)/dt; return t*t+t+t+1.0f; }
    return 0.0f;
}
static float shape(unsigned which, float t, float dt, float width,unsigned mode,dnbx_work *w)
{
    switch (which) {
    case 0: COUNT(w,sine_calls,1); return fm_sin_turns(t);
    case 1: return 1.0f-4.0f*fm_fabsf(t-.5f);
    case 2: return 2.0f*t-1.0f-(mode==DNBX_OSC_RAW?0:blep(t,dt,w));
    default: {
        float q=t-width;
        if (q < 0) q += 1;
        return (t<width ? 1.0f:-1.0f)+(mode==DNBX_OSC_RAW?0:blep(t,dt,w)-blep(q,dt,w));
    }
    }
}
static float oscillator(float t,float dt,float wave,float width,unsigned mode,dnbx_work *w)
{
    float pos=wave*3.0f;
    unsigned first=(unsigned)pos;
    float frac=pos-(float)first;
    float a=shape(first,t,dt,width,mode,w);
    if (first >= 3 || frac == 0.0f) return a;
    return a+(shape(first+1,t,dt,width,mode,w)-a)*frac;
}

static void enter_stage(dnbx_envelope_state *e, const dnbx_envelope_params *p,
                        unsigned stage, float sr, dnbx_work *w)
{
    float seconds=0.0f;
    e->stage=(uint8_t)stage; e->tick_left=0; e->delta=0; e->curve=e->level;
    e->endpoint=e->level; e->asymptote=e->level; e->step_rate=0; e->correction=0;
    COUNT(w,stage_transitions,1);
    switch (stage) {
    case DNBX_ENV_ATTACK:
        seconds=p->attack_s; e->endpoint=1; e->asymptote=1+ATTACK_OVERSHOOT; break;
    case DNBX_ENV_HOLD: seconds=p->hold_s; break;
    case DNBX_ENV_DECAY:
        seconds=p->decay_s; e->endpoint=p->sustain;
        e->asymptote=p->sustain-FALL_OVERSHOOT; break;
    case DNBX_ENV_SUSTAIN:
        e->endpoint=e->curve=e->level=p->sustain; break;
    case DNBX_ENV_RELEASE_HOLD: seconds=p->release_hold_s; break;
    case DNBX_ENV_RELEASE:
        seconds=p->release_s; e->endpoint=0; e->asymptote=-FALL_OVERSHOOT; break;
    default: e->stage=DNBX_ENV_IDLE; e->endpoint=e->curve=e->level=0; break;
    }
    e->remaining=sample_count(seconds,sr);
    e->timed_stage=e->remaining!=0;
    if (e->remaining && e->endpoint != e->level) {
        float ratio=(e->endpoint-e->asymptote)/(e->level-e->asymptote);
        /* All configured segments have a positive ratio in (0,1]. */
        float exponent=fm_log2f(ratio)*(16.0f/(float)e->remaining);
        float x=exponent*FM_LN2;
        /* Stable 1-exp(x) near zero, avoiding loss of a tiny long-duration
         * step through subtraction of two nearly equal floats. */
        if (x>-.01f) {
            e->step_rate=-x*(1+x*(.5f+x*(1.0f/6.0f+x*(1.0f/24.0f))));
            COUNT(w,small_exp_calls,1);
        } else {
            e->step_rate=1-fm_exp2f(exponent); COUNT(w,exp2_calls,1);
        }
        COUNT(w,log2_calls,1);
    }
}
static unsigned next_stage(unsigned stage)
{
    switch (stage) {
    case DNBX_ENV_ATTACK: return DNBX_ENV_HOLD;
    case DNBX_ENV_HOLD: return DNBX_ENV_DECAY;
    case DNBX_ENV_DECAY: return DNBX_ENV_SUSTAIN;
    case DNBX_ENV_RELEASE_HOLD: return DNBX_ENV_RELEASE;
    default: return DNBX_ENV_IDLE;
    }
}
/* At most six transitions, including an instantaneous attack/hold/decay.
 * No elapsed-time loop and no dependence on the host's callback boundary. */
static void settle(dnbx_envelope_state *e,const dnbx_envelope_params *p,
                    float sr,dnbx_work *w)
{
    unsigned chain=0;
    for (unsigned i=0;i<6;i++) {
        if (e->stage==DNBX_ENV_IDLE || e->stage==DNBX_ENV_SUSTAIN ||
            e->remaining || e->tick_left) break;
        int completed_attack=e->stage==DNBX_ENV_ATTACK && e->timed_stage;
        e->level=e->curve=e->endpoint;
        enter_stage(e,p,next_stage(e->stage),sr,w);
        if (completed_attack && e->stage==DNBX_ENV_HOLD && !e->remaining) {
            e->remaining=DNBX_ENVELOPE_TICK; e->timed_stage=1;
        }
        chain++;
    }
#if DNBX_DIAGNOSTIC_COUNTERS
    if (w && chain>w->maximum_transition_chain) w->maximum_transition_chain=chain;
#else
    (void)chain;
#endif
}
static void prepare_tick(dnbx_envelope_state *e,int first_attack,dnbx_work *w)
{
    uint32_t take=e->remaining<16 ? e->remaining:16;
    if (!take) return;
    if (take==e->remaining) { e->curve=e->endpoint; e->correction=0; }
    else {
        /* Compensated accumulation bounds long-segment float drift. */
        float change=(e->asymptote-e->curve)*e->step_rate-e->correction;
        float next=e->curve+change;
        e->correction=(next-e->curve)-change;
        e->curve=next;
    }
    e->curve=fm_clampf(e->curve,0,1);
    e->delta=(e->curve-e->level)/(float)take;
    e->remaining-=take; e->tick_left=(uint8_t)take;
    /* The first attack tick is flat at its first endpoint, an observation
     * from the provided cancellation traces, not a host-block convention. */
    if (first_attack) { e->level=e->curve; e->delta=0; }
    COUNT(w,envelope_ticks,1);
}
static void envelope_on(dnbx_envelope_state *e,const dnbx_envelope_params *p,float sr)
{
    enter_stage(e,p,DNBX_ENV_ATTACK,sr,0);
    settle(e,p,sr,0);
    if (e->stage==DNBX_ENV_ATTACK) prepare_tick(e,1,0);
}
static void envelope_off(dnbx_envelope_state *e,const dnbx_envelope_params *p,float sr)
{
    if (!e->stage) return;
    enter_stage(e,p,DNBX_ENV_RELEASE_HOLD,sr,0);
    settle(e,p,sr,0);
}
static float envelope_sample(dnbx_envelope_state *e,const dnbx_envelope_params *p,
                              float sr,dnbx_work *w)
{
    COUNT(w,envelope_samples,1);
    if (!e->tick_left) {
        settle(e,p,sr,w);
        if (e->stage==DNBX_ENV_IDLE || e->stage==DNBX_ENV_SUSTAIN) return e->level;
        prepare_tick(e,0,w);
    }
    e->level=fm_clampf(e->level+e->delta,0,1);
    if (--e->tick_left==0) e->level=e->curve;
    return e->level;
}

uint32_t dnbx_abi_version(void) { return DNBX_ABI_VERSION; }
uint32_t dnbx_diagnostic_counters_enabled(void) { return DNBX_DIAGNOSTIC_COUNTERS; }
size_t dnbx_part_size(void) { return sizeof(dnbx_part); }
size_t dnbx_layer_params_size(void) { return sizeof(dnbx_layer_params); }
size_t dnbx_work_size(void) { return sizeof(dnbx_work); }
void dnbx_default_layer_params(dnbx_layer_params *p)
{
    if (!p) return;
    memset(p,0,sizeof *p);
    p->amp.attack_s=.008f; p->amp.decay_s=.28f; p->amp.sustain=.72f; p->amp.release_s=.24f;
    p->filter.attack_s=.02f; p->filter.decay_s=.2f; p->filter.sustain=.4f; p->filter.release_s=.3f;
    p->waveform=2.0f/3.0f; p->pulse_width=.5f; p->gain=1;
    p->cutoff_hz=4000; p->resonance=.12f; p->filter_env_hz=4000;
}
int dnbx_init(dnbx_part *s,float sr)
{
    if (!s) return DNBX_E_ARGUMENT;
    if (!range(sr,8000,192000)) return DNBX_E_RANGE;
    memset(s,0,sizeof *s); s->magic=DNBX_MAGIC; s->sample_rate=sr;
    for (unsigned l=0;l<DNBX_LAYERS;l++) dnbx_default_layer_params(&s->layer[l].params);
    return DNBX_OK;
}
int dnbx_set_layer(dnbx_part *s,unsigned layer,const dnbx_layer_params *p)
{
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
    if (layer>=DNBX_LAYERS || !p) return DNBX_E_ARGUMENT;
    if (!params_valid(p)) return DNBX_E_RANGE;
    s->layer[layer].params=*p;
    for (unsigned i=0;i<DNBX_VOICES;i++) {
        dnbx_layer_voice *v=&s->voice[i].layer[layer];
        v->increment=increment(s->voice[i].note,p->tune_semitones,s->sample_rate);
        /* A running timed segment retains its cached endpoint/rate. */
        if (v->amp.stage==DNBX_ENV_SUSTAIN) v->amp.level=v->amp.curve=p->amp.sustain;
        if (v->filter.stage==DNBX_ENV_SUSTAIN) v->filter.level=v->filter.curve=p->filter.sustain;
    }
    return DNBX_OK;
}
int dnbx_get_layer(const dnbx_part *s,unsigned l,dnbx_layer_params *p)
{
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
    if (l>=DNBX_LAYERS || !p) return DNBX_E_ARGUMENT;
    *p=s->layer[l].params; return DNBX_OK;
}
int dnbx_note_on(dnbx_part *s,unsigned note,unsigned velocity)
{
    unsigned chosen=0; int found=0;
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
    if (note>127 || velocity>127) return DNBX_E_RANGE;
    if (!velocity) { dnbx_note_off(s,note); return DNBX_NOTE_OFF_ONLY; }
    /* DnBOX allocator policy: same held note, silent slot, first released
     * slot, then oldest held slot. Bounded ranks avoid timestamp rollover. */
    for (unsigned i=0;i<DNBX_VOICES;i++)
        if (s->voice[i].gate && s->voice[i].note==note) { chosen=i; found=1; break; }
    if (!found) for (unsigned i=0;i<DNBX_VOICES;i++)
        if (!voice_active(&s->voice[i])) { chosen=i; found=1; break; }
    if (!found) for (unsigned i=0;i<DNBX_VOICES;i++)
        if (!s->voice[i].gate) { chosen=i; found=1; break; }
    if (!found) for (unsigned i=1;i<DNBX_VOICES;i++)
        if (s->voice[i].age_rank>s->voice[chosen].age_rank) chosen=i;
    dnbx_voice *v=&s->voice[chosen];
    int retrigger=v->gate && v->note==note;
    uint8_t next_rank[DNBX_VOICES]={0};
    /* Retirement can leave gaps, so compact the surviving order rather than
     * incrementing stale ranks. At most 36 comparisons per note-on, outside
     * render; ranks stay unique and bounded by five indefinitely. */
    for (unsigned i=0;i<DNBX_VOICES;i++) if (i!=chosen && voice_active(&s->voice[i])) {
        next_rank[i]=1;
        for (unsigned j=0;j<DNBX_VOICES;j++)
            if (j!=chosen && j!=i && voice_active(&s->voice[j]) &&
                (s->voice[j].age_rank<s->voice[i].age_rank ||
                 (s->voice[j].age_rank==s->voice[i].age_rank && j<i))) next_rank[i]++;
    }
    for (unsigned i=0;i<DNBX_VOICES;i++) s->voice[i].age_rank=next_rank[i];
    if (!retrigger) memset(v,0,sizeof *v);
    v->note=(uint8_t)note; v->gate=1; v->age_rank=0; v->pedal_held=0;
    v->velocity=(float)velocity/127.0f;
    for (unsigned l=0;l<DNBX_LAYERS;l++) {
        const dnbx_layer_params *p=&s->layer[l].params;
        v->layer[l].increment=increment(note,p->tune_semitones,s->sample_rate);
        envelope_on(&v->layer[l].amp,&p->amp,s->sample_rate);
        envelope_on(&v->layer[l].filter,&p->filter,s->sample_rate);
    }
    return (int)chosen;
}
static void release_voice(dnbx_part *s,dnbx_voice *v)
{
    for (unsigned l=0;l<DNBX_LAYERS;l++) {
        envelope_off(&v->layer[l].amp,&s->layer[l].params.amp,s->sample_rate);
        envelope_off(&v->layer[l].filter,&s->layer[l].params.filter,s->sample_rate);
    }
}
int dnbx_note_off(dnbx_part *s,unsigned note)
{
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
    if (note>127) return DNBX_E_RANGE;
    for (unsigned i=0;i<DNBX_VOICES;i++) {
        dnbx_voice *v=&s->voice[i];
        if (v->gate && v->note==note) {
            v->gate=0; v->pedal_held=s->damper;
            if (!s->damper) release_voice(s,v);
        }
    }
    return DNBX_OK;
}
int dnbx_set_sustain(dnbx_part *s,unsigned down)
{
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
    if (down>1) return DNBX_E_RANGE;
    if (s->damper && !down) for (unsigned i=0;i<DNBX_VOICES;i++)
        if (s->voice[i].pedal_held) {
            s->voice[i].pedal_held=0; release_voice(s,&s->voice[i]);
        }
    s->damper=(uint8_t)down; return DNBX_OK;
}
int dnbx_set_velocity_mode(dnbx_part *s,unsigned mode)
{
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
    if (mode>DNBX_VELOCITY_SQUARED) return DNBX_E_RANGE;
    s->velocity_mode=(uint8_t)mode; return DNBX_OK;
}
int dnbx_all_off(dnbx_part *s)
{
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
    memset(s->voice,0,sizeof s->voice); s->damper=0;
    for (unsigned l=0;l<DNBX_LAYERS;l++) s->layer[l].lfo_phase=0;
    return DNBX_OK;
}
int dnbx_active_voices(const dnbx_part *s)
{
    int n=0;
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
    for (unsigned i=0;i<DNBX_VOICES;i++) n+=voice_active(&s->voice[i])!=0;
    return n;
}
int dnbx_read_voice(const dnbx_part *s,unsigned i,dnbx_voice_snapshot *out)
{
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
    if (i>=DNBX_VOICES || !out) return DNBX_E_ARGUMENT;
    const dnbx_voice *v=&s->voice[i];
    memset(out,0,sizeof *out);
    for (unsigned l=0;l<DNBX_LAYERS;l++) {
        out->amp[l]=v->layer[l].amp.level; out->filter[l]=v->layer[l].filter.level;
        out->phase[l]=v->layer[l].phase;
        out->amp_stage[l]=v->layer[l].amp.stage; out->filter_stage[l]=v->layer[l].filter.stage;
    }
    out->note=v->note; out->gate=v->gate; out->age_rank=v->age_rank;
    out->active=(uint8_t)(voice_active(v)!=0); return DNBX_OK;
}
int dnbx_render(dnbx_part *s,float *left,float *right,unsigned n,dnbx_work *w)
{
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
#if !DNBX_DIAGNOSTIC_COUNTERS
    if (w) return DNBX_E_UNSUPPORTED;
#endif
    if (n>DNBX_MAX_FRAMES) return DNBX_E_RANGE;
    if (!n) return DNBX_OK;
    if (!left || !right || left==right) return DNBX_E_ARGUMENT;
    COUNT(w,frames,n);
    for (unsigned k=0;k<n;k++) {
        float mix[2]={0,0},width[2];
        for (unsigned l=0;l<DNBX_LAYERS;l++) {
            dnbx_layer *ls=&s->layer[l];
            float lfo=1-4*fm_fabsf(ls->lfo_phase-.5f);
            width[l]=fm_clampf(ls->params.pulse_width+lfo*ls->params.lfo_pwm_depth,.03f,.5f);
            ls->lfo_phase+=ls->params.lfo_hz/s->sample_rate;
            if (ls->lfo_phase>=1) ls->lfo_phase-=1;
            COUNT(w,lfo_samples,1);
        }
        for (unsigned i=0;i<DNBX_VOICES;i++) {
            dnbx_voice *v=&s->voice[i];
            COUNT(w,voice_slots,1);
            if (!voice_active(v)) continue;
            COUNT(w,active_voice_samples,1);
            float velocity=v->velocity;
            if (s->velocity_mode==DNBX_VELOCITY_SQUARED) velocity*=velocity;
            for (unsigned l=0;l<DNBX_LAYERS;l++) {
                dnbx_layer_voice *lv=&v->layer[l];
                const dnbx_layer_params *p=&s->layer[l].params;
                float amp=envelope_sample(&lv->amp,&p->amp,s->sample_rate,w);
                float env=envelope_sample(&lv->filter,&p->filter,s->sample_rate,w);
                float x=oscillator(lv->phase,lv->increment,p->waveform,width[l],p->oscillator_mode,w);
                x+=(lv->subphase<.5f?1.0f:-1.0f)*p->sub_gain;
                /* Existing DnBOX two-pole recurrence, with explicit saturation
                 * guards to make a finite bound part of this candidate ABI. */
                float a=fm_clampf(FM_TWO_PI*(p->cutoff_hz+env*p->filter_env_hz)/s->sample_rate,.001f,.75f);
                lv->filter_first=fm_clampf(lv->filter_first+a*(x-lv->filter_first-p->resonance*.8f*lv->filter_second),-4,4);
                lv->filter_second=fm_clampf(lv->filter_second+a*(lv->filter_first-lv->filter_second),-4,4);
                mix[l]+=velocity*amp*lv->filter_second;
                lv->phase+=lv->increment; if (lv->phase>=1) lv->phase-=1;
                lv->subphase+=lv->increment*.5f; if (lv->subphase>=1) lv->subphase-=1;
                COUNT(w,layer_samples,1);
            }
        }
        left[k]=0; right[k]=0;
        for (unsigned l=0;l<DNBX_LAYERS;l++) {
            const dnbx_layer_params *p=&s->layer[l].params;
            float x=mix[l]*(p->gain/6.0f);
            left[k]+=x*(.5f-.5f*p->pan); right[k]+=x*(.5f+.5f*p->pan);
        }
    }
    return DNBX_OK;
}
int dnbx_store_raw(dnbx_part *s,unsigned id,float raw)
{
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
    if (id>=DNBX_RAW_CONTROLS || !finite_float(raw)) return DNBX_E_RANGE;
    s->raw[id]=raw; s->raw_present[id/32u]|=1u<<(id%32u);
    if (id==207) return DNBX_RAW_RESERVED;
    if (id>=212 && id<=214) return DNBX_RAW_EXTERNALLY_OWNED;
    return DNBX_RAW_STORED_ONLY;
}
int dnbx_read_raw(const dnbx_part *s,unsigned id,float *raw)
{
    if (!valid(s)) return DNBX_E_UNINITIALIZED;
    if (id>=DNBX_RAW_CONTROLS || !raw) return DNBX_E_ARGUMENT;
    if (!(s->raw_present[id/32u]&(1u<<(id%32u)))) return DNBX_E_ABSENT;
    *raw=s->raw[id]; return DNBX_OK;
}
int dnbx_attack_from_unit(float u,float *s)
{
    if (!s) return DNBX_E_ARGUMENT;
    if (!range(u,0,1)) return DNBX_E_RANGE;
    *s=.002f+15.998f*fm_powf(u,3.33f); return DNBX_OK;
}
int dnbx_lfo_from_unit(float u,float *hz)
{
    if (!hz) return DNBX_E_ARGUMENT;
    if (!range(u,0,1)) return DNBX_E_RANGE;
    *hz=16*fm_powf(u,3.33f); return DNBX_OK;
}
int dnbx_pwm_from_unit(float u,float *duty)
{
    if (!duty) return DNBX_E_ARGUMENT;
    if (!range(u,0,1)) return DNBX_E_RANGE;
    *duty=.5f-.47f*u; return DNBX_OK;
}
int dnbx_range_from_stop(float stop,float *ratio)
{
    if (!ratio) return DNBX_E_ARGUMENT;
    if (stop==0) *ratio=.5f;
    else if (stop==.5f) *ratio=1;
    else if (stop==1) *ratio=2;
    else return DNBX_E_RANGE;
    return DNBX_OK;
}
int dnbx_wave_probe(float phase,float dt,float wave,float width,float *sample)
{
    return dnbx_wave_probe_mode(phase,dt,wave,width,DNBX_OSC_STOCK_BLEP,sample);
}
int dnbx_wave_probe_mode(float phase,float dt,float wave,float width,unsigned mode,float *sample)
{
    if (!sample) return DNBX_E_ARGUMENT;
    if (!range(phase,0,1) || phase==1 || !range(dt,.000001f,.45f) ||
        !range(wave,0,1) || !range(width,.03f,.5f) || mode>DNBX_OSC_RAW) return DNBX_E_RANGE;
    *sample=oscillator(phase,dt,wave,width,mode,0); return DNBX_OK;
}
