/* SPDX-License-Identifier: GPL-3.0-only */
void juno6_init(juno6_t *s,float sr)
{
#if X0X_SUPERJ6_BUILTIN_INIT
 /* This profile never runs the retired renderer. Keep only the controls
  * that the typed setters and host/UI read, not its private voice state. */
 memset(s,0,sizeof *s);
 s->sample_rate=sr>8000?sr:44100;
 s->lfo2_entropy_state=UINT64_C(0xc42735cc98dbcb83);
 s->macro[0]=.35f;s->macro[4]=.35f;s->macro[10]=.12f;
 s->macro[15]=1.0f;s->macro[16]=.5f;
#else
 stock_init(s,sr);
#endif
 (void)dnbx_init_superj6(&s->core,s->sample_rate);
 s->master_gain=1.f;
#if X0X_SUPERJ6_BUILTIN_INIT
 /* The prepared core already defaults to saw (2/3), centered and dry. */
 for(unsigned l=0;l<DNBX_LAYERS;l++)
  s->core.layer[l].params.oscillator_mode=DNBX_OSC_SUPERJ6;
 s->typed=1;
#endif
}
#if defined(X0X_SJ6_MONO32_PROBE) && X0X_SJ6_MONO32_PROBE
#if !defined(X0X_HOST) && !DNBOX_TYPED_CONTROLS_NATIVE_TEST
#error scoped SuperJ6 VOICE requires explicit read-only native trial
#endif
int juno6_mono32_set(juno6_t *s,unsigned on)
{
 if(!s||on>1u)return 0;
 if(s->mono32_on==on)return 1;
 /* The Build9 conductor sends ordinary note-offs, not a hard kill.
  * A released envelope and a pedal-held tail survive the mode switch. */
 for(unsigned i=0;i<DNBX_VOICES;i++)if(s->core.voice[i].gate)
   (void)dnbx_note_off(&s->core,s->core.voice[i].note);
 memset(s->mono32_keys,0,sizeof s->mono32_keys);
 s->mono32_on=(uint8_t)on;
 return 1;
}
int juno6_legato33_set(juno6_t *s,unsigned on)
{
 if(!s||on>1u)return 0;
 s->legato33_on=(uint8_t)on;return 1;
}
int juno6_glide35_set(juno6_t *s,unsigned pos)
{
 if(!s||pos>127u)return 0;
 s->glide35_detent=(uint8_t)pos;
 s->core.sj6_glide_secs=(float)((double)((float)pos/127.f)*.5);
 /* Rate zero stops advancement, not the already-active pitch offset. */
 return 1;
}
static int mono_sounding_key(const juno6_t *s,unsigned note)
{
 return (s->mono32_keys[note>>5]>>(note&31u))&1u;
}
static int mono_has_keys(const juno6_t *s)
{
 return !!(s->mono32_keys[0]|s->mono32_keys[1]|s->mono32_keys[2]|s->mono32_keys[3]);
}
static void sj6_mono_glide_start(juno6_t *s,unsigned note,int overlap)
{
 /* The original Mono control forwards its own value to Glide Enable. */
 const float next=overlap&&s->core.sj6_glide_secs>0.f ?
     (float)s->core.voice[0].note-(float)note+s->core.sj6_glide_offset:0.f;
 s->core.sj6_glide_offset=(next>-.01f&&next<.01f)?0.f:next;
 if(!s->core.sj6_glide_offset)s->core.sj6_glide_ratio=1.f;
 s->core.sj6_glide_clock=0;
}
static void sj6_mono_released(juno6_t *s,unsigned note)
{
 if(!s->core.voice[0].gate||s->core.voice[0].note!=note)return;
 for(unsigned k=0;k<128;k++)if(mono_sounding_key(s,k)){
   sj6_mono_glide_start(s,k,1);
   if(s->legato33_on)(void)dnbx_superj6_legato_voice(&s->core,k);
   else (void)dnbx_superj6_mono_voice(&s->core,k,100);
   return;
 }
 (void)dnbx_note_off(&s->core,note);
}
static int sj6_voice_pedal(juno6_t *s,unsigned down)
{
 const int was=s->core.damper;
 int ok=dnbx_set_sustain(&s->core,down)==DNBX_OK;
 if(ok&&was&&!down&&s->mono32_on&&s->core.voice[0].gate&&!mono_sounding_key(s,s->core.voice[0].note))
    sj6_mono_released(s,s->core.voice[0].note);
 return ok;
}
#endif
int juno6_note_on(juno6_t *s,uint8_t note,uint8_t velocity)
{
#if !X0X_SUPERJ6_BUILTIN_INIT
 if (!s->typed) return stock_note_on(s,note,velocity);
#endif
#if defined(X0X_SJ6_MONO32_PROBE) && X0X_SJ6_MONO32_PROBE
 if(s->mono32_on){
  if(note>127||velocity>127)return 0;
  if(!velocity){juno6_note_off(s,note);return 1;}
  /* The app coalesces duplicate ons by dispatching only the final off. */
  const int continuing=s->core.voice[0].gate&&mono_has_keys(s);
  s->mono32_keys[note>>5]|=UINT32_C(1)<<(note&31u);
  sj6_mono_glide_start(s,note,continuing);
  if(continuing&&s->legato33_on)return dnbx_superj6_legato_voice(&s->core,note)==0;
  return dnbx_superj6_mono_voice(&s->core,note,velocity)==0;
 }
#endif
 return dnbx_note_on(&s->core,note,velocity)>=0;
}
void juno6_note_off(juno6_t *s,uint8_t note)
{
#if defined(X0X_SJ6_MONO32_PROBE) && X0X_SJ6_MONO32_PROBE
 if(s->mono32_on) {
  if(note>127||!mono_sounding_key(s,note))return;
  s->mono32_keys[note>>5]&=~(UINT32_C(1)<<(note&31u));
  /* Actual original pedal defers dispatch of the physical key's off. */
  if(!s->core.damper)sj6_mono_released(s,note);
  return;
 }
#endif
#if X0X_SUPERJ6_BUILTIN_INIT
 (void)dnbx_note_off(&s->core,note);
#else
 if (!s->typed) stock_note_off(s,note);
 else (void)dnbx_note_off(&s->core,note);
#endif
}
void juno6_all_off(juno6_t *s)
{
#if X0X_SUPERJ6_BUILTIN_INIT
 s->macro[17]=0;
#else
 stock_all_off(s);
#endif
#if defined(X0X_SJ6_MONO32_PROBE) && X0X_SJ6_MONO32_PROBE
 memset(s->mono32_keys,0,sizeof s->mono32_keys);
 s->core.sj6_glide_offset=0.f;s->core.sj6_glide_ratio=1.f;s->core.sj6_glide_clock=0;
#endif
 (void)dnbx_all_off(&s->core);
 memset(s->typed_hp_state,0,sizeof s->typed_hp_state);
 memset(s->typed_hp_input,0,sizeof s->typed_hp_input);
}
int juno6_active(const juno6_t *s)
{
#if X0X_SUPERJ6_BUILTIN_INIT
 return dnbx_active_voices(&s->core)>0;
#else
 return s->typed ? dnbx_active_voices(&s->core)>0 : stock_active(s);
#endif
}

int juno6_can_edit_amp2(const juno6_t *s)
{
#if X0X_SUPERJ6_BUILTIN_INIT
 (void)s;
 return 1;
#else
 return s->typed || !stock_active(s);
#endif
}

/* Direct AMP2 events target only layer 2, including while AMP link is on.
 * The control converters and the envelope sample renderer are owned by DnBOX. */
static float amp_stage_from_unit(unsigned stage, float raw)
{
    if (stage == 0) {
        float seconds = 0.f;
        (void)dnbx_attack_from_unit(raw, &seconds);
        return seconds;
    }
    if (stage == 2) return fm_powf(raw, 2.5f);
    return .002f + 23.998f * fm_powf(raw, 3.35f);
}
static int set_amp2(dnbx_part *core, unsigned id, float raw)
{
    dnbx_layer_params layer;
    if (dnbx_get_layer(core, 1, &layer) != DNBX_OK) return 0;
    unsigned stage = id - 119u;
    if (stage >= 4) return 0;
    float *const adsr[] = {&layer.amp.attack_s, &layer.amp.decay_s, &layer.amp.sustain, &layer.amp.release_s};
    *adsr[stage] = amp_stage_from_unit(stage, raw);
    if (dnbx_set_layer(core, 1, &layer) != DNBX_OK) return 0;
    return dnbx_store_raw(core, id, raw) == DNBX_RAW_STORED_ONLY;
}

/* Public ID36, only for an explicitly independent pre-note layer 1.
 * Use the qualified scalar law and leave layer 2 and the filter EG alone. */
int juno6_apply_independent_amp1_attack(juno6_t *s, float raw)
{
    dnbx_layer_params layer;
    if (dnbx_get_layer(&s->core, 0, &layer) != DNBX_OK ||
        dnbx_attack_from_unit(raw, &layer.amp.attack_s) != DNBX_OK ||
        dnbx_set_layer(&s->core, 0, &layer) != DNBX_OK)
        return 0;
    return dnbx_store_raw(&s->core, 36, raw) == DNBX_RAW_STORED_ONLY;
}

/* Called only for a validated pre-note snapshot. Missing legacy PWM fields
 * are irrelevant for sine/triangle/saw. Square requires an explicit gate. */
void juno6_apply_pre_note_wave(juno6_t *s, unsigned layer, const juno6_oscillator_edit *edit)
{
    dnbx_layer_params params;
    if (dnbx_get_layer(&s->core, layer, &params) != DNBX_OK) return;
    /* Adjacent binary32 threshold points bound truncation of binary64 3*x. */
    unsigned stop = (edit->selector >= 0x1.555556p-2f)
                  + (edit->selector >= 0x1.555556p-1f) + (edit->selector == 1);
    params.oscillator_mode = DNBX_OSC_SUPERJ6;
    params.waveform = stop * (1.f / 3.f);
    params.pulse_width = .5f;
    if (stop == 3 && edit->enable > .5f)
        (void)dnbx_pwm_from_unit(edit->amount, &params.pulse_width);
    if (dnbx_set_layer(&s->core, layer, &params) != DNBX_OK) return;
    s->typed = 1;
    (void)dnbx_store_raw(&s->core, 82 + layer, edit->selector);
    if (edit->enable >= 0) {
        (void)dnbx_store_raw(&s->core, 78 + layer, edit->amount);
        (void)dnbx_store_raw(&s->core, 210 + layer, edit->enable);
    }
}

/* A new factory program owns both base waveforms. Modulation depth from
 * the previous stock Juno program is unrelated to its original LFO routes. */
void juno6_apply_factory_waves(juno6_t *s, const juno6_oscillator_edit waves[2])
{
    for (unsigned layer = 0; layer < DNBX_LAYERS; ++layer) {
        s->core.layer[layer].params.lfo_pwm_depth = 0.f;
        juno6_apply_pre_note_wave(s, layer, &waves[layer]);
    }
}

/* Setters run at block boundaries on a single-precision target. Scale the
 * significand by 101/250 and round once, as the original Double setter does.
 * All factory inputs are zero or normal with exponent at least three. */
static float master_from_raw(float raw)
{
    fm_bits_t value = {.f=raw};
    unsigned exponent = value.u >> 23;
    if (raw == 0) return 0.f;
    if (exponent < 3) return raw * .404f;
    uint32_t significand = (value.u & 0x7fffffu) | 0x800000u;
    uint32_t numerator = significand * 101u;
    unsigned result_exponent = exponent - 2;
    if (numerator < (125u << 23)) {
        numerator *= 2;
        --result_exponent;
    }
    /* The normal-result denominator 125 is odd, so a tie is impossible. */
    value.u = (result_exponent << 23) + (numerator + 62u) / 125u;
    return value.f;
}

static float range_pitch(float x) { return x * 24.f - 12.f; }
static float typed_range_pitch(const juno6_t *s,float range)
{
#if X0X_SUPERJ6_BUILTIN_INIT
#if defined(X0X_SJ6_BEND_TYPED_PROBE) && X0X_SJ6_BEND_TYPED_PROBE
 const float bend=s->pb_typed ? 0.f : (s->macro[16]-.5f)*4;
#endif
#if !(defined(X0X_SJ6_BEND_TYPED_PROBE) && X0X_SJ6_BEND_TYPED_PROBE)
 const float bend=(s->macro[16]-.5f)*4;
#endif
 float raw;
 if (dnbx_read_raw(&s->core, SJ6_A_REFERENCE_ID, &raw) == DNBX_OK && raw != .6f)
  return bend + range_pitch(range) + 12.f * fm_log2f(juno6_a_reference_hz(raw) / 440.f);
#else
 const float bend=s->bend;
#endif
 return bend+range_pitch(range);
}
#if X0X_SUPERJ6_BUILTIN_INIT
unsigned juno6_layer_fine(const juno6_t *s, unsigned l)
{
 float raw;
 return dnbx_read_raw(&s->core, SJ6_LAYER1_FINE_ID + l, &raw) == DNBX_OK ?
        (unsigned)(raw * SJ6_FINE_MAX + .5f) : SJ6_FINE_CENTER;
}

unsigned juno6_layer_semi(const juno6_t *s, unsigned l)
{
 float raw;
 return dnbx_read_raw(&s->core, SJ6_LAYER1_SEMI_ID + l, &raw) == DNBX_OK ?
        (unsigned)(raw * SJ6_SEMI_MAX + .5f) : SJ6_SEMI_CENTER;
}
#endif
static float layer_tuning(const juno6_t *s, unsigned l, float pitch)
{
#if X0X_SUPERJ6_BUILTIN_INIT
 const int semi = (int)juno6_layer_semi(s, l) - SJ6_SEMI_CENTER;
 if (semi) pitch += semi;
 const int cents = (int)juno6_layer_fine(s, l) - SJ6_FINE_CENTER;
 if (cents) pitch += (float)cents / 100.f;
#endif
 return pitch;
}

static int apply_voice_tuning(juno6_t *s, float range)
{
 const float pitch=typed_range_pitch(s,range);
 for(unsigned l=0;l<DNBX_LAYERS;l++) {
  dnbx_layer_params p;
  if(dnbx_get_layer(&s->core,l,&p)!=DNBX_OK) return 0;
  p.tune_semitones=layer_tuning(s,l,pitch);
  if(dnbx_set_layer(&s->core,l,&p)!=DNBX_OK)return 0;
 }
 return 1;
}

#if defined(X0X_SJ6_BEND_TYPED_PROBE) && X0X_SJ6_BEND_TYPED_PROBE
/* Original lower is a magnitude; the sign comes from normalized travel. */
static int sj6_pb_refresh(juno6_t *s)
{
    float x=s->macro[16]-.5f;
    return dnbx_set_superj6_bend(&s->core,2.f*x*(x<0.f?s->pb_down:s->pb_up))==DNBX_OK;
}
int juno6_set_bend_ranges(juno6_t *s, unsigned down, unsigned up)
{
    if (!s || !s->typed || down>48u || up>48u) return 0;
    /* First opt-in happens at adoption. Retain separate authored tuning. */
    if (!s->pb_typed) {
        float range=.5f;
        if (s->macro[16]!=.5f) return 0;
        (void)dnbx_read_raw(&s->core,86u,&range);
        s->pb_typed=1;
        if (!apply_voice_tuning(s,range)) { s->pb_typed=0; return 0; }
    }
    s->pb_down=(uint8_t)down;
    s->pb_up=(uint8_t)up;
    return sj6_pb_refresh(s);
}
#endif
void juno6_apply_pre_note_range(juno6_t *s,float x)
{
 const unsigned id=86u;
 if (!apply_voice_tuning(s, x < 0.f ? .5f : x)) return;
 if(x<0.f) {
  s->core.raw[id]=0.f;
  s->core.raw_present[id/32u]&=~(1u<<(id%32u));
 } else {
  (void)dnbx_store_raw(&s->core,id,x);
  s->typed=1;
 }
}

#if X0X_SUPERJ6_BUILTIN_INIT
static const uint8_t sj6_lfo_rate_id[SJ6_LFO_COUNT] = {88, 96, 100};
#define SJ6_LFO_TURN (UINT64_C(1) << 40)

/* Build-9 normalized rate uses reversed, nearest division index. Keep the
 * original near-third factors and the binary32 operation order. */
static const float sj6_lfo_factors[SJ6_LFO_DIVISIONS] = {
    64.f, 48.f, 32.f, 24.f, 16.f, 12.f, 8.f, 6.f, 4.f, 3.f, 2.f, 1.5f,
    1.f, .5f, .3333f, .25f, .1667f, .125f
};

static void read_lfo(const juno6_t *s, unsigned lfo, unsigned wheel, juno6_lfo_view *out)
{
    *out = (juno6_lfo_view){0};
    out->phase = s->lfo_phase_q40[lfo];
    if (dnbx_read_raw(&s->core, sj6_lfo_rate_id[lfo], &out->rate) == DNBX_OK) out->present |= 1u;
    if (dnbx_read_raw(&s->core, 195u + lfo, &out->mode) == DNBX_OK) out->present |= 2u;
    if (wheel) {
        out->rate=(float)(wheel-1u)/127.f;
        if (!(out->present&2u)) out->mode=1.f; /* Build-9 default Hz mode */
        out->present=3u;
    }
    if (out->present == 3u) {
        if (out->mode == 1.f) (void)dnbx_lfo_from_unit(out->rate, &out->hz);
        else {
            out->division = (unsigned)((1.f - out->rate) * 17.f + .5f);
            out->hz = ((s->lfo_tempo / 60.f) * sj6_lfo_factors[out->division]) * .25f;
        }
    }
}

void juno6_read_lfo(const juno6_t *s,unsigned lfo,juno6_lfo_view *out)
{ read_lfo(s,lfo,0,out); }

static void set_lfo_word(dnbx_part *core, unsigned id, int present, float raw)
{
    if (present) (void)dnbx_store_raw(core, id, raw);
    else {
        core->raw[id] = 0.f;
        core->raw_present[id / 32u] &= ~(1u << (id % 32u));
    }
}

#include "juno6_lfo_phase.h"

float juno6_a_reference_unit(unsigned position)
{
    /* Correctly rounded hundredths through the same fixed-point strategy
     * as the 7-bit unit control. 2^32 / 100 has quotient 42949672 and remainder 96. */
    if (position >= SJ6_A_REFERENCE_STEPS) return 1.f;
    return (float)(position * 42949672u + (position * 96u + 50u) / 100u) * 0x1p-32f;
}

static void refresh_layer_tune(juno6_t *s, unsigned l)
{
    float range = .5f;
    (void)dnbx_read_raw(&s->core, 86u, &range);
    dnbx_layer_params p;
    if (dnbx_get_layer(&s->core, l, &p) != DNBX_OK) return;
    p.tune_semitones = layer_tuning(s, l, typed_range_pitch(s, range));
    (void)dnbx_set_layer(&s->core, l, &p);
}

void juno6_set_layer_semi(juno6_t *s, unsigned l, unsigned position)
{
    if (l >= DNBX_LAYERS || position > SJ6_SEMI_MAX || position == juno6_layer_semi(s, l)) return;
    set_lfo_word(&s->core, SJ6_LAYER1_SEMI_ID + l, position != SJ6_SEMI_CENTER,
                 (float)position / SJ6_SEMI_MAX);
    refresh_layer_tune(s, l);
}
void juno6_set_layer_fine(juno6_t *s, unsigned l, unsigned position)
{
    if (l >= DNBX_LAYERS || position > SJ6_FINE_MAX || position == juno6_layer_fine(s, l)) return;
    set_lfo_word(&s->core, SJ6_LAYER1_FINE_ID + l, position != SJ6_FINE_CENTER,
                 (float)position / SJ6_FINE_MAX);
    refresh_layer_tune(s, l);
}

void juno6_set_a_reference(juno6_t *s, float normalized, unsigned present)
{
    set_lfo_word(&s->core, SJ6_A_REFERENCE_ID, present, normalized);
    float range = .5f;
    (void)dnbx_read_raw(&s->core, 86u, &range);
    (void)apply_voice_tuning(s, range);
}

static void prepare_lfo_step(juno6_t *s, unsigned lfo)
{
    juno6_lfo_view view;
    read_lfo(s,lfo,lfo==0 ? s->mod_wheel_lfo1 : lfo==1 ? s->mod_wheel_lfo2 : 0u,&view);
    float advance = view.hz * ((float)SJ6_LFO_TURN / s->sample_rate) + .5f;
    uint64_t step = sj6_lfo_unsigned_phase(advance);
    s->lfo_step_q40[lfo] = step;
    if (lfo == 1u) s->lfo2_hold_period = view.hz > 0.f ? 1.f / view.hz : 0.f;
    unsigned bit = 1u << lfo;
    s->lfo_running = (uint8_t)(step ? s->lfo_running | bit : s->lfo_running & ~bit);
    s->lfo_sync = (uint8_t)(view.present == 3u && view.mode == 0.f ?
                          s->lfo_sync | bit : s->lfo_sync & ~bit);
}

static float sj6_vibrato_depth(float unit) { return 12.f * fm_powf(unit,3.03f); }

/* The normal (non-OTA) build-9 direct cutoff target. Source computes
 * normalized division and square in Float, then multiplies and adds in Double. */
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
#include "sj6_cutoff_port.h"
#include "sj6_lfo_double_port.h"
#endif
static float sj6_normal_cutoff_hz(float x)
{
    static const float hi[]={.1f,.3f,.5f,.7f,.9f};
    static const uint16_t base[]={20,30,200,600,4100,14400,20000};
    unsigned seg=0;
    while (seg<5u && x>hi[seg]) ++seg;
    float u=(x-(seg ? hi[seg-1u]:0.f))/((seg==0u||seg==5u)?.1f:.2f);
    float squared=u*u;
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
    return sj6_cutoff_port_round(squared,base[seg],base[seg+1u]-base[seg]);
#else
    return (float)((double)base[seg]+(double)squared*(base[seg+1u]-base[seg]));
#endif
}

float juno6_mod_cutoff_hz(unsigned value)
{ return value<=127u ? sj6_normal_cutoff_hz((float)value/127.f):0.f; }

int juno6_configure_lfo1_cutoff(juno6_t *s,float base,float current,float amount)
{
    if(!s||!s->typed||!(base>=0.f&&base<=1.f&&current>=0.f&&current<=1.f&&amount>=0.f&&amount<=1.f))
        return 0;
    s->lfo1_pwm_route=3; /* selector 1 goes to global cutoff, not an oscillator PWM */
    s->lfo1_pwm_base=base;
    s->wheel_lfo1_amount=0;
    s->lfo1_cut_current=s->lfo1_cut_target=current;
    s->lfo1_cut_until=(int32_t)s->sample_rate;
    s->lfo1_cut_active=s->lfo1_cut_force=0;
    (void)dnbx_store_raw(&s->core,89u,amount);
    return 1;
}

int juno6_configure_lfo1_pwm(juno6_t *s,unsigned selector,unsigned enabled,float base,float amount)
{
    if(!s || !s->typed || (selector!=69u && selector!=70u) || enabled>1u ||
       !(base>=0.f && base<=1.f && amount>=0.f && amount<=1.f)) return 0;
    s->lfo1_pwm_route=enabled ? (uint8_t)(selector-68u):0;
    s->lfo1_pwm_base=base;
    s->wheel_lfo1_amount=0;
    (void)dnbx_store_raw(&s->core,89u,amount);
    return 1;
}

#if defined(X0X_HOST) && defined(X0X_SJ6_ALT3_WITNESS)
static float sj6_alt3_rate(float normalized,int synced,float tempo)
{
    if (!synced) return 16.0f*fm_powf(normalized,3.33f);
    static const double grid[18]={
        64,48,32,24,16,12,8,6,4,3,2,1.5,1,.5,.3333,.25,.1667,.125
    };
    const unsigned index=(unsigned)roundf((1.f-normalized)*17.f);
    return (float)(((double)tempo/60.0*grid[index])*.25);
}
int juno6_arm_alt3_running(juno6_t *s,unsigned verified_ramp_frames,int tempo_sync)
{
    /* Only constructor-default ramp profile has a source witness. Do not
     * choose it for an unknown live AU event or an unverified graph. */
    if (!s || !s->typed || s->alt3.armed || verified_ramp_frames!=10000u ||
        (unsigned)tempo_sync>1u || (tempo_sync && !(s->lfo_tempo>0.f))) return 0;
    s->alt3.duration=verified_ramp_frames;
    s->alt3.elapsed=s->alt3.depth_elapsed=verified_ramp_frames;
    /* KoFMInstrument initialization sends frequency=3. Depth starts zero. */
    s->alt3.rate=s->alt3.to_rate=3.f;
    s->alt3.tempo_sync=(uint8_t)tempo_sync;
    s->alt3.armed=1;
    return 1;
}
int juno6_arm_alt3_free(juno6_t *s,unsigned verified_ramp_frames,int tempo_sync)
{
    return !tempo_sync && juno6_arm_alt3_running(s,verified_ramp_frames,0);
}
static void sj6_alt3_set_rate(juno6_t *s,float normalized)
{
    float rate=sj6_alt3_rate(normalized,s->alt3.tempo_sync,s->lfo_tempo);
    if(rate!=s->alt3.to_rate){s->alt3.to_rate=rate;s->alt3.dirty|=1u;}
}
static void sj6_alt3_target(juno6_t *s,unsigned value)
{
    float u=(float)value/127.f;
    sj6_alt3_set_rate(s,.9f*u);
    if(u!=s->alt3.to_depth){s->alt3.to_depth=u;s->alt3.dirty|=2u;}
    s->alt3.last_wheel_code=(uint8_t)(value+1u);
}
static float sj6_alt3_table(unsigned i)
{
    i &= 4095u;
    float x=(float)i*(FM_TWO_PI/4096.f);
    return .5f*fm_sin_pi(i>2048u ? x-FM_TWO_PI : x)+.5f;
}
void juno6_alt3_process(juno6_t *s,float *left,float *right,unsigned n)
{
    if (!s->alt3.armed) return;
    for(unsigned i=0;i<n;i++){
        if (!s->alt3.tick) {
            if(s->alt3.dirty&1u){
                s->alt3.from_rate=s->alt3.rate;
                s->alt3.elapsed=0;
            }
            if(s->alt3.dirty&2u){
                s->alt3.from_depth=s->alt3.depth;
                s->alt3.depth_elapsed=0;
            }
            s->alt3.dirty=0;
            if(s->alt3.elapsed>=s->alt3.duration)s->alt3.rate=s->alt3.to_rate;
            else {
                float u=(float)s->alt3.elapsed/(float)s->alt3.duration;
                s->alt3.rate=fmaf(s->alt3.to_rate-s->alt3.from_rate,u,s->alt3.from_rate);
                s->alt3.elapsed+=8u;
            }
            if(s->alt3.depth_elapsed>=s->alt3.duration)s->alt3.depth=s->alt3.to_depth;
            else {
                float u=(float)s->alt3.depth_elapsed/(float)s->alt3.duration;
                s->alt3.depth=fmaf(s->alt3.to_depth-s->alt3.from_depth,u,s->alt3.from_depth);
                s->alt3.depth_elapsed+=8u;
            }
            s->alt3.increment=(uint32_t)nearbyintf((s->alt3.rate*.5f)*(0x1p24f/s->sample_rate));
        }
        s->alt3.tick=(uint8_t)((s->alt3.tick+1u)&7u);
        float *channel[2]={left+i,right+i};
        for(unsigned ch=0;ch<2;ch++){
            if(s->alt3.depth){
                unsigned p=s->alt3.phase,ind=p>>12;
                float a=sj6_alt3_table(ind),b=sj6_alt3_table(ind+1u);
                float wave=fmaf(b-a,(p&4095u)*(1.f/4096.f),a);
                *channel[ch]*=(1.f-s->alt3.depth*wave);
            }
            s->alt3.phase=(s->alt3.phase+s->alt3.increment)&0xffffffu;
        }
    }
}
#endif

int juno6_set_mod_destination(juno6_t *s,unsigned selected)
{
    if (selected==6u) selected=0; /* Build-9 final segment aliases the default. */
    if (!s || !s->typed) return 0;
#if defined(DNBOX_137_HOST_WITNESS) && defined(X0X_SJ6_MOD_TYPED_PROBE)
    if(selected>5u)return 0;
    /* Selection is configuration, not a performance event. Gate actual
     * wheel dispatch separately from restoring stored owner choice. */
    s->mod_destination=(uint8_t)selected;
    return 1;
#endif
#if defined(X0X_HOST) && defined(X0X_SJ6_ALT3_WITNESS)
    if(selected==3u) { if(!s->alt3.armed)return 0; }
    else
#endif
    if(selected>1u && selected!=4u && selected!=5u && (selected!=2u || !s->lfo1_pwm_route)) return 0;
    s->mod_destination=(uint8_t)selected;
    return 1;
}

int juno6_set_mod_wheel(juno6_t *s,unsigned value)
{
    if (!s || !s->typed || value>127u) return 0;
#if defined(X0X_HOST) && defined(X0X_SJ6_ALT3_WITNESS)
    if (s->mod_destination==3u) {
        if(!s->alt3.armed || (s->alt3.tempo_sync && !(s->lfo_tempo>0.f)))return 0;
        sj6_alt3_target(s,value);
        return 1;
    }
#else
    if(s->mod_destination==3u)return 0;
#endif
    if (s->mod_destination==1u) {
        s->vibrato_depth=sj6_vibrato_depth((float)value/127.f);
        return 1;
    }
    if (s->mod_destination==2u) {
        if(!s->lfo1_pwm_route) return 0;
        s->wheel_lfo1_amount=(uint8_t)(value+1u);
        return 1;
    }
    if (s->mod_destination==5u) {
        float u=(float)value/127.f;
        s->mod_wheel_cutoff_hz=sj6_normal_cutoff_hz(u);
        if(s->lfo1_pwm_route==3) {
            s->lfo1_cut_force=s->lfo1_cut_active;
            s->lfo1_cut_active=0;
            s->lfo1_cut_current=s->lfo1_cut_target=u;
        }
        return 1;
    }
    const unsigned lfo = s->mod_destination==4u;
    uint8_t *effective = lfo ? &s->mod_wheel_lfo2 : &s->mod_wheel_lfo1;
    if (*effective!=value+1u) {
        *effective=(uint8_t)(value+1u);
        prepare_lfo_step(s,lfo);
    }
    return 1;
}

void juno6_set_lfo_tempo(juno6_t *s, float bpm)
{
    if (bpm == s->lfo_tempo || bpm < 0.f || !sj6_lfo_finite(bpm)) return;
    s->lfo_tempo = bpm;
#if defined(X0X_HOST) && defined(X0X_SJ6_ALT3_WITNESS)
    if (s->alt3.armed && s->alt3.tempo_sync && s->alt3.last_wheel_code && bpm>0.f)
        sj6_alt3_set_rate(s,.9f*((float)(s->alt3.last_wheel_code-1u)/127.f));
#endif
    for (unsigned i = 0, sync = s->lfo_sync; sync; ++i, sync >>= 1)
        if (sync & 1u) prepare_lfo_step(s, i);
}

void juno6_set_lfo_words(juno6_t *s, const juno6_lfo_words *words, int reset_phase)
{
    if (reset_phase) {
        s->mod_wheel_lfo1=s->mod_wheel_lfo2=0;
        s->mod_wheel_cutoff_hz=0.f;
        s->wheel_lfo1_amount=0;
        s->lfo1_pwm_until_tick=0;
        s->lfo1_cut_active=s->lfo1_cut_force=0;
        s->lfo1_cut_until=(int32_t)s->sample_rate;
        s->lfo2_hold_sample = s->lfo2_hold_elapsed = 0.f;
        s->lfo2_pwm_until_tick = 0;
    }
    for (unsigned lfo = 0; lfo < SJ6_LFO_COUNT; ++lfo) {
        unsigned mask = words ? words->present >> lfo : 0u;
        unsigned presence = (mask & 1u) | ((mask >> 2u) & 2u);
        set_lfo_word(&s->core, sj6_lfo_rate_id[lfo], presence & 1u, words ? words->word[lfo] : 0.f);
        set_lfo_word(&s->core, 195u + lfo, presence & 2u,
                     words ? words->word[SJ6_LFO_COUNT + lfo] : 0.f);
        prepare_lfo_step(s, lfo);
        if (reset_phase) s->lfo_phase_q40[lfo] = 0;
    }
}

void juno6_set_panel_lfo(juno6_t *s, unsigned lfo, int position, unsigned hz_mode)
{
    if (position >= 0) {
        if (lfo==0) s->mod_wheel_lfo1=0;
        if (lfo==1) s->mod_wheel_lfo2=0;
        (void)dnbx_store_raw(&s->core, sj6_lfo_rate_id[lfo], position * (1.f / 127.f));
    }
    float old_mode;
    /* Leave an equal authored mode word intact, including signed zero. */
    if (dnbx_read_raw(&s->core, 195u + lfo, &old_mode) != DNBX_OK || old_mode != hz_mode)
        (void)dnbx_store_raw(&s->core, 195u + lfo, (float)hz_mode);
    prepare_lfo_step(s, lfo);
}

void juno6_set_lfo2_pwm(juno6_t *s, uint16_t setting)
{
    const unsigned mask = SJ6_LFO2_PWM_DEPTH_MASK | SJ6_LFO2_PWM_ROUTE_MASK;
    if ((s->lfo2_pwm >> SJ6_LFO2_PWM_SHAPE_SHIFT) != SJ6_LFO_HELD &&
        (!(setting & SJ6_LFO2_PWM_DEPTH_MASK) || !(setting & SJ6_LFO2_PWM_ROUTE_MASK)))
        s->lfo2_pwm_until_tick = 0;
    s->lfo2_pwm = (s->lfo2_pwm & ~mask) | (setting & mask);
}

void juno6_set_lfo2_shape(juno6_t *s, unsigned shape)
{
    const unsigned mask = SJ6_LFO2_SHAPE_MASK << SJ6_LFO2_PWM_SHAPE_SHIFT;
    if (shape <= SJ6_LFO2_SHAPE_MAX)
        s->lfo2_pwm = (s->lfo2_pwm & ~mask) | (shape << SJ6_LFO2_PWM_SHAPE_SHIFT);
}

static const uint8_t sj6_knob_ids[SJ6_OSC_KNOBS] = {82, 83, 86, 210, 78, 211, 79};

static float sj6_knob_raw(const juno6_t *s, unsigned slot)
{
    float raw = slot <= SJ6_WAVE2 ? 0x1.555556p-1f : slot == SJ6_RANGE ? .5f : 0.f;
    (void)dnbx_read_raw(&s->core, sj6_knob_ids[slot], &raw);
    return raw;
}

static void adsr_values_of(const dnbx_envelope_params *env, float out[4])
{
    out[0] = env->attack_s; out[1] = env->decay_s;
    out[2] = env->sustain; out[3] = env->release_s;
}

unsigned juno6_knob_position(const juno6_t *s, unsigned slot)
{
    if (slot == SJ6_LFO2_PWM_DEPTH) return s->lfo2_pwm & SJ6_LFO2_PWM_DEPTH_MASK;
    if (slot == SJ6_LFO2_PWM_ROUTE)
        return (s->lfo2_pwm & SJ6_LFO2_PWM_ROUTE_MASK) >> SJ6_LFO2_PWM_ROUTE_SHIFT;
    if (slot == SJ6_LFO2_SHAPE) return s->lfo2_pwm >> SJ6_LFO2_PWM_SHAPE_SHIFT;
    if (slot >= SJ6_LFO1_MODE && slot <= SJ6_LFO3_MODE) {
        float hz = 1.f;
        (void)dnbx_read_raw(&s->core, 195u + slot - SJ6_LFO1_MODE, &hz);
        return hz == 1.f;
    }
    if (slot >= SJ6_LFO1_RATE && slot <= SJ6_LFO3_RATE) {
        float raw = 0.f;
        (void)dnbx_read_raw(&s->core, sj6_lfo_rate_id[slot - SJ6_LFO1_RATE], &raw);
        return (unsigned)(raw * 127.f + .5f);
    }
    if (slot == SJ6_FILTER_LINK) return s->core.filter_link;
    if (slot == SJ6_MASTER_LEVEL) {
        float raw;
        if (dnbx_read_raw(&s->core, 0, &raw) != DNBX_OK) return SJ6_MASTER_INIT;
        return (unsigned)(raw * (127.f / JUNO6_FACTORY_MASTER_MAX) + .5f);
    }
    if (slot >= SJ6_AMP_ATTACK) {
        const dnbx_layer_params *p = &s->core.layer[0].params;
        unsigned filter = slot >= SJ6_FILTER_ATTACK;
        unsigned stage = slot - (filter ? SJ6_FILTER_ATTACK : SJ6_AMP_ATTACK);
        float values[4];
        adsr_values_of(filter ? &p->filter : &p->amp, values);
        float v = values[stage];
        if (filter) {
            if (stage != 2) v = fm_powf(v * .0625f, 1.f / 3.f);
        } else if (stage == 2) v = fm_powf(v, .4f);
        else v = fm_powf(fm_maxf(v - .002f, 0.f) /
                        (stage == 0 ? 15.998f : 23.998f), 1.f / (stage == 0 ? 3.33f : 3.35f));
        return (unsigned)(fm_clampf(v, 0.f, 1.f) * 127.f + .5f);
    }
    float raw = sj6_knob_raw(s, slot);
    if (slot <= SJ6_WAVE2)
        return (raw >= 0x1.555556p-2f) + (raw >= 0x1.555556p-1f) + (raw == 1);
    if (slot == SJ6_RANGE) return (unsigned)(raw * 2.f);
    if (slot == SJ6_PWM1_ON || slot == SJ6_PWM2_ON) return raw > .5f;
    return (unsigned)(raw * SJ6_PWM_STEPS + .5f);
}

/* Only apply the touched controls. Read neighboring authored words at the
 * audio boundary so a panel detent never re-quantizes another control. */
void juno6_apply_knobs(juno6_t *s, unsigned mask, const uint8_t p[SJ6_OSC_KNOBS])
{
    for (unsigned layer = 0; layer < DNBX_LAYERS; ++layer) {
        unsigned wave = SJ6_WAVE1 + layer, on = SJ6_PWM1_ON + 2u * layer, amount = on + 1u;
        if (!(mask & ((1u << wave) | (1u << on) | (1u << amount)))) continue;
        juno6_oscillator_edit e = {
            mask & (1u << wave) ? p[wave] * (1.f / SJ6_WAVE_STOPS) : sj6_knob_raw(s, wave),
            mask & (1u << amount) ? p[amount] * (1.f / SJ6_PWM_STEPS) : sj6_knob_raw(s, amount),
            mask & (1u << on) ? p[on] : sj6_knob_raw(s, on)
        };
        juno6_apply_pre_note_wave(s, layer, &e);
    }
    if (mask & (1u << SJ6_RANGE)) juno6_apply_pre_note_range(s, p[SJ6_RANGE] * .5f);
}

static void apply_adsr(dnbx_envelope_params *env, unsigned mask, const float *physical)
{
    if (mask & 1u) env->attack_s = physical[0];
    if (mask & 2u) env->decay_s = physical[1];
    if (mask & 4u) env->sustain = physical[2];
    if (mask & 8u) env->release_s = physical[3];
}

void juno6_apply_panel_envelopes(juno6_t *s, unsigned mask, const uint8_t p[SJ6_EXTRA_COUNT], int reset)
{
    float physical[8];
    for (unsigned i = 0; i < 8; ++i) if (mask & (1u << i)) {
        float u = (p[i] & 127u) * (1.f / 127.f);
        /* The common AMP uses the measured attack and secondary stage laws.
         * FILTER time is a physical panel taper across the compact core range. */
        physical[i] = i < 4 ? amp_stage_from_unit(i, u) : i == 6 ? u : 16.f * u * u * u;
    }
    unsigned linked = mask & (1u << 8) ? p[8] & 1u : reset ? 0 : s->core.filter_link;
    dnbx_layer_params defaults;
    if (reset) dnbx_default_layer_params(&defaults);
    for (unsigned l = 0; l < DNBX_LAYERS; ++l) {
        const dnbx_layer_params *layer = reset ? &defaults : &s->core.layer[l].params;
        dnbx_envelope_params amp = layer->amp, filter = layer->filter;
        apply_adsr(&amp, mask, physical);
        apply_adsr(&filter, mask >> 4, physical + 4);
        dnbx_apply_panel_envelopes(&s->core, l, &amp, &filter, linked);
    }
}
#endif

#if X0X_SUPERJ6_BUILTIN_INIT
float juno6_sj6_hpf_hz(float unit)
{
    if (unit <= 0.f) return 0.f;
    if (unit >= 1.f) return 800.f;
    return 800.f * fm_powf(unit, 2.42f);
}
void juno6_set_sj6_hpf(juno6_t *s, float unit)
{
    s->macro[9] = unit;
    /* Nonnegative hpf retains the exact historical normalized setting;
     * otherwise cache authored Hz here once per edit, not per audio block. */
    s->hpf = -1.f - juno6_sj6_hpf_hz(unit);
}
#endif

int juno6_set(juno6_t *s,unsigned id,float x)
{
 if(!s)return 0;
 if(id==JUNO6_APP_PARAM_BASE){
  if(!(x>=0 && x<=JUNO6_FACTORY_MASTER_MAX))return 0;
  if(dnbx_store_raw(&s->core,0,x)!=DNBX_RAW_STORED_ONLY)return 0;
  s->master_gain=master_from_raw(x);
  return 1;
 }
 if(!(x>=0&&x<=1))return 0;
 if(id>=JUNO6_APP_PARAM_BASE){
  if(!juno6_can_edit_amp2(s) || !set_amp2(&s->core,id-JUNO6_APP_PARAM_BASE,x))return 0;
  /* AMP2 explicitly selects the two-layer engine. Only a waveform snapshot
   * selects the original app oscillator on an individual layer. */
  s->typed=1;
  return 1;
 }
#if X0X_SUPERJ6_BUILTIN_INIT
 /* Existing AMP automation must not overwrite the separate FILTER values. */
 #define JUNO6_LEGACY_ADSR(field, value) (p.amp.field = (value))
 if(id>=18)return 0;
 s->macro[id]=x;
#if defined(X0X_SJ6_BEND_TYPED_PROBE) && X0X_SJ6_BEND_TYPED_PROBE
 if(id==16 && s->pb_typed) return sj6_pb_refresh(s);
#endif
 if(id==9)s->hpf=x;
 if(id==2)s->mod_wheel_cutoff_hz=0.f;
 #define JUNO6_APPLY_VALUE(stock,native) (native)
#else
 #define JUNO6_LEGACY_ADSR(field, value) (p.amp.field = p.filter.field = (value))
 if(!stock_set(s,id,x))return 0;
 #define JUNO6_APPLY_VALUE(stock,native) (stock)
#endif
 if(id==17){
#if X0X_SJ6_MONO32_PROBE
  return sj6_voice_pedal(s,x>=.5f);
#else
  return dnbx_set_sustain(&s->core,x>=.5f)==DNBX_OK;
#endif
 }
 if(id==9)return 1;
 if(id==13||id==14)return 1; /* Existing runtime owns stock chorus parameters. */
 for(unsigned l=0;l<DNBX_LAYERS;l++){
  dnbx_layer_params p;
  if(dnbx_get_layer(&s->core,l,&p)!=DNBX_OK)return 0;
  switch(id){
  case 0:
   if (p.oscillator_mode == DNBX_OSC_SUPERJ6) p.oscillator_mode = DNBX_OSC_STOCK_BLEP;
   p.waveform=(2+x)/3;break;
  case 1:p.sub_gain=JUNO6_APPLY_VALUE(s->sub_level,juno6_control_physical(1,x));break;
  case 2:p.cutoff_hz=35+x*x*12000;break;
  case 3:p.resonance=JUNO6_APPLY_VALUE(s->resonance,x);break;
  case 4:p.filter_env_hz=7000*x;break;
  case 5:JUNO6_LEGACY_ADSR(attack_s,JUNO6_APPLY_VALUE(s->attack,juno6_control_physical(5,x)));break;
  case 6:JUNO6_LEGACY_ADSR(decay_s,JUNO6_APPLY_VALUE(s->decay,juno6_control_physical(6,x)));break;
  case 7:JUNO6_LEGACY_ADSR(sustain,JUNO6_APPLY_VALUE(s->sustain,x));break;
  case 8:JUNO6_LEGACY_ADSR(release_s,JUNO6_APPLY_VALUE(s->release,juno6_control_physical(8,x)));break;
  case 10:p.lfo_hz=.1f+8*x*x;break;
  case 11:p.lfo_pwm_depth=x*.2f;break;
  case 12:p.pulse_width=clampf(JUNO6_APPLY_VALUE(s->pulse_width,juno6_control_physical(12,x)),.03f,.5f);break;
  case 15:p.gain=x;break;
  case 16:{
   float raw=.5f;
   (void)dnbx_read_raw(&s->core,86,&raw);
   p.tune_semitones=layer_tuning(s,l,typed_range_pitch(s,raw));
   break;
  }
  default:break;
  }
  if(dnbx_set_layer(&s->core,l,&p)!=DNBX_OK)return 0;
 }
 #undef JUNO6_APPLY_VALUE
 #undef JUNO6_LEGACY_ADSR
 return 1;
}

#if X0X_SUPERJ6_BUILTIN_INIT
void juno6_apply_panel_master(juno6_t *s, unsigned position)
{
    /* Preserve the pre-existing unattenuated DnBOX INIT as one higher detent.
     * Positions 0..127 cover the entire admitted build-9 raw domain. */
    if (position == SJ6_MASTER_INIT) {
        s->master_gain = 1.f;
        s->core.raw[0] = 0.f;
        s->core.raw_present[0] &= ~1u;
        return;
    }
    float raw = position == 127u ? JUNO6_FACTORY_MASTER_MAX :
                position * (JUNO6_FACTORY_MASTER_MAX / 127.f);
    (void)juno6_set(s, JUNO6_APP_PARAM_BASE, raw);
}

/* Build-9's steady Hz vibrato uses the existing LFO rate law and a separate
 * semitone taper. The original sampler holds a global sine value for 16 frames. */
enum { SJ6_VIBRATO_TICK = 16 };

void juno6_set_vibrato(juno6_t *s, float depth_unit, float rate_unit, int reset)
{
    float hz;
    if (!(depth_unit >= 0.f && depth_unit <= 1.f) ||
        dnbx_lfo_from_unit(rate_unit, &hz) != DNBX_OK) return;
    s->vibrato_depth = sj6_vibrato_depth(depth_unit);
    s->vibrato_step = (uint32_t)(hz * ((float)SJ6_VIBRATO_TICK * 0x1p32f / s->sample_rate) + .5f);
    if (reset) s->vibrato_phase = s->vibrato_left = 0;
}

void juno6_set_vibrato_words(juno6_t *s, const float values[2], unsigned present, int reset)
{
    for (unsigned i = 0; i < 2; ++i)
        set_lfo_word(&s->core, 105u + i, (present & (1u << i)) != 0, values[i]);
    /* Build-9 defaults for omitted depth and rate are binary32 positive zero. */
    juno6_set_vibrato(s, present & 1u ? values[0] : 0.f, present & 2u ? values[1] : 0.f, reset);
}

static float osc_level_gain(unsigned code)
{ return code ? ((float)(code-1u) / 127.f) * .808f : 1.f; }

#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
#include "sj6_velocity_scale.inc"
#endif
int juno6_set_filter_velocity(juno6_t *s,unsigned position)
{
    if (position>127u) return 0;
    s->filter_velocity_position=(uint8_t)position;
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
    s->filter_velocity_scale=sj6_velocity_scale[position];
#else
    const float raw=(float)position / 127.f;
    s->filter_velocity_scale=(float)((double)raw * 1.1);
#endif
    return 1;
}

static int render_typed_dry(juno6_t *s,float *left,float *right,unsigned frames)
{
    if (!(s->osc1_level_code|s->osc2_level_code|s->filter_velocity_position)) return dnbx_render(&s->core,left,right,frames,0);
    return dnbx_render_pitch_osc_levels_velocity(&s->core,left,right,frames,1.f,
                                       osc_level_gain(s->osc1_level_code),osc_level_gain(s->osc2_level_code),s->filter_velocity_scale);
}

static int render_typed_span(juno6_t *s, float *left, float *right, unsigned frames)
{
    if ((!s->vibrato_step && !s->vibrato_phase && !s->vibrato_left) ||
        !frames || frames > DNBX_MAX_FRAMES || !left || !right || left == right)
        return render_typed_dry(s,left,right,frames);
    if (!s->vibrato_depth) {
        int result = render_typed_dry(s,left,right,frames);
        if (result != DNBX_OK) return result;
        /* The original shared sine still advances while its depth is zero. */
        unsigned remaining = s->vibrato_left;
        if (frames > remaining) {
            unsigned ticks = (frames - remaining + SJ6_VIBRATO_TICK - 1) / SJ6_VIBRATO_TICK;
            s->vibrato_phase += s->vibrato_step * ticks;
            remaining += ticks * SJ6_VIBRATO_TICK;
        }
        s->vibrato_left = (uint8_t)(remaining - frames);
        s->vibrato_ratio = 1.f;
        return result;
    }
    while (frames) {
        if (!s->vibrato_left) {
            float wave = fm_clampf(fm_sin_turns((float)s->vibrato_phase * 0x1p-32f), -1.f, 1.f);
            s->vibrato_ratio = fm_exp2f(wave * (s->vibrato_depth / 12.f));
            s->vibrato_phase += s->vibrato_step;
            s->vibrato_left = SJ6_VIBRATO_TICK;
        }
        unsigned count = frames < s->vibrato_left ? frames : s->vibrato_left;
        int result = dnbx_render_pitch_osc_levels_velocity(&s->core,left,right,count,s->vibrato_ratio,osc_level_gain(s->osc1_level_code),osc_level_gain(s->osc2_level_code),s->filter_velocity_scale);
        if (result != DNBX_OK) return result;
        s->vibrato_left -= count;
        frames -= count;
        left += count;
        right += count;
    }
    return DNBX_OK;
}

/* Build-9 row columns 0 to 3. The ramps use one-sided amount laws.
 * Every wave uses the same 256 phase bins as the original constructors. */
static float lfo2_pwm_delta(unsigned shape, unsigned index, float depth)
{
    switch (shape) {
    case SJ6_LFO_UP: {
        float wave = index * (1.f / 128.f) - 1.f;
        return depth * ((wave + 1.f) * .5f);
    }
    case SJ6_LFO_DOWN: {
        float wave = 1.f - index * (1.f / 128.f);
        return depth * ((wave - 1.f) * .5f);
    }
    case SJ6_LFO_SQUARE:
        return ((index < 128u ? 1.f : -1.f) * depth) * .5f;
    default: {
        unsigned rotated = (index + 64u) & 255u;
        float wave = rotated < 128u ? rotated * (1.f / 64.f) - 1.f : 3.f - rotated * (1.f / 64.f);
        return (wave * depth) * .5f;
    }
    }
}

#include "juno6_lfo_hold.h"
static uint64_t sj6_held_next(void *ctx)
{
    juno6_t *s = ctx;
    uint64_t x = s->lfo2_entropy_state;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    s->lfo2_entropy_state = x;
    return x;
}
static void sj6_held_80hz(juno6_t *s)
{
    (void)sj6_held_tick(&s->lfo2_hold_elapsed, &s->lfo2_hold_sample, 1.f / 80.f,
                       s->lfo2_hold_period, sj6_held_next, s);
}
/* Advance the held clock when its mapped audio destination is bypassed; no
 * typed-render split, so muted/invalid routes do not change audio work. */
static void sj6_held_bypass(juno6_t *s, unsigned frames)
{
    unsigned until = s->lfo2_pwm_until_tick;
    while (frames) {
        if (until < 80u) {sj6_held_80hz(s); until += (unsigned)s->sample_rate;}
        unsigned count = until / 80u;
        if (count > frames) count = frames;
        frames -= count;
        until -= count * 80u;
    }
    s->lfo2_pwm_until_tick = until;
}

/* Sample the existing audio clock on the nominal 80 Hz control grid. */
/* At the cutoff controller's nominal 80 Hz tick, target submission
 * precedes the exponential smoother. Original Float update order retained. */
static void sj6_cutoff_tick(juno6_t *s,uint64_t phase)
{
    const float amount=s->wheel_lfo1_amount ? (float)(s->wheel_lfo1_amount-1u)/127.f:s->core.raw[89u];
    const float eps=0x1.0624dep-11f;
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
    if(amount>.01f) {
#else
    if((double)amount>.01) {
#endif
        unsigned index=((unsigned)(phase>>32)+64u)&255u;
        float wave=index<128u ? index*(1.f/64.f)-1.f:3.f-index*(1.f/64.f);
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
        float target=sj6_lfo_port(s->lfo1_pwm_base,amount,(int)(wave*64.f));
#else
        double candidate=(double)s->lfo1_pwm_base+(double)wave*(double)amount*.5;
        float target=(float)(candidate<0. ? 0.:candidate>1. ? 1.:candidate);
#endif
        s->lfo1_cut_target=target;
        if(s->lfo1_cut_force) {
            s->lfo1_cut_force=s->lfo1_cut_active=0;
            s->lfo1_cut_current=target;
            s->mod_wheel_cutoff_hz=sj6_normal_cutoff_hz(target);
        } else if(fm_fabsf(target-s->lfo1_cut_current)>eps) s->lfo1_cut_active=1;
    }
    if(s->lfo1_cut_active) {
        float current=s->lfo1_cut_current,target=s->lfo1_cut_target;
        float next=current+(target-current)*0x1.217c58p-1f;
        s->mod_wheel_cutoff_hz=sj6_normal_cutoff_hz(next);
        if(fm_fabsf(target-next)<=eps) {
            s->lfo1_cut_active=0;
            s->lfo1_cut_current=target;
        } else s->lfo1_cut_current=next;
    }
}
static int sj6_render_lfo1_cutoff(juno6_t *s,float *left,float *right,unsigned frames)
{
    uint64_t phase=s->lfo_phase_q40[0];
    while(frames) {
        if(s->lfo1_cut_until<=0) {
            sj6_cutoff_tick(s,phase);
            s->lfo1_cut_until+=(int32_t)s->sample_rate;
        }
        unsigned n=((unsigned)s->lfo1_cut_until+79u)/80u;
        if(n>frames)n=frames;
        if(s->mod_wheel_cutoff_hz)
            for(unsigned i=0;i<2;++i)s->core.layer[i].params.cutoff_hz=s->mod_wheel_cutoff_hz;
        int result=render_typed_span(s,left,right,n);
        if(result!=DNBX_OK)return result;
        s->lfo1_cut_until-=(int32_t)(80u*n);
        phase+=s->lfo_step_q40[0]*n;
        frames-=n; left+=n; right+=n;
    }
    return DNBX_OK;
}

static int render_lfo_pwm(juno6_t *s,unsigned loop,unsigned receiver,float *left,float *right,unsigned frames)
{
    const unsigned tick_hz = 80;
    const unsigned sample_rate = (unsigned)s->sample_rate;
    dnbx_layer_params *layer = &s->core.layer[receiver].params;
    const float base=loop ? s->core.raw[78u+receiver]:s->lfo1_pwm_base;
    const float depth=loop ? (s->lfo2_pwm & SJ6_LFO2_PWM_DEPTH_MASK)*(1.f/127.f):
        s->wheel_lfo1_amount ? (float)(s->wheel_lfo1_amount-1u)/127.f:s->core.raw[89u];
    const unsigned shape=loop ? s->lfo2_pwm>>SJ6_LFO2_PWM_SHAPE_SHIFT:SJ6_LFO_TRI;
    const float saved_width = layer->pulse_width, saved_depth = layer->lfo_pwm_depth;
    uint32_t *until = loop ? &s->lfo2_pwm_until_tick:&s->lfo1_pwm_until_tick;
    uint8_t *index = loop ? &s->lfo2_pwm_index:&s->lfo1_pwm_index;
    unsigned until_tick=*until;
    uint64_t phase=s->lfo_phase_q40[loop];
    int result = DNBX_OK;
    layer->lfo_pwm_depth = 0.f;
    while (frames) {
        if (until_tick < tick_hz) {
            *index = (uint8_t)(phase >> 32);
            if (shape == SJ6_LFO_HELD) sj6_held_80hz(s);
            until_tick += sample_rate;
        }
        float delta = shape == SJ6_LFO_HELD ? (s->lfo2_hold_sample * depth) * .5f :
                      lfo2_pwm_delta(shape,*index,depth);
        /* LFO1 route law combines the archived integer triangle sample and
         * original authored amount/base at original Double precision. */
        float raw;
        if(loop) raw=fm_clampf(base+delta,0.f,1.f);
        else {
            unsigned rotated=(*index+64u)&255u;
            float wave=rotated<128u ? rotated*(1.f/64.f)-1.f:3.f-rotated*(1.f/64.f);
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
            raw=sj6_lfo_port(base,depth,(int)(wave*64.f));
#else
            double v=(double)base+((double)wave*(double)depth)*.5;
            raw=(float)(v<0. ? 0.:v>1. ? 1.:v);
#endif
        }
        (void)dnbx_pwm_from_unit(raw, &layer->pulse_width);
        unsigned count = until_tick / tick_hz;
        if (count > frames) count = frames;
        result = render_typed_span(s, left, right, count);
        if (result != DNBX_OK) break;
        until_tick -= count * tick_hz;
        phase += s->lfo_step_q40[loop] * count;
        frames -= count;
        left += count;
        right += count;
    }
    *until=until_tick;
    layer->pulse_width = saved_width;
    layer->lfo_pwm_depth = saved_depth;
    return result;
}
#endif

int juno6_render_stereo(juno6_t *s,float *left,float *right,unsigned frames)
{
#if !X0X_SUPERJ6_BUILTIN_INIT
 if(!s->typed){
  stock_render(s,left,frames);
  memcpy(right,left,frames*sizeof *right);
  return 1;
 }
#endif
#if X0X_SUPERJ6_BUILTIN_INIT
 const float wheel_cutoff=s->mod_wheel_cutoff_hz;
 float base[2];
 const unsigned cut_route=s->lfo1_pwm_route==3;
 if (wheel_cutoff||cut_route) for(unsigned l=0;l<2;++l) {
  base[l]=s->core.layer[l].params.cutoff_hz;
  if(wheel_cutoff)s->core.layer[l].params.cutoff_hz=wheel_cutoff;
 }
 int rendered;
 unsigned receiver = ((s->lfo2_pwm & SJ6_LFO2_PWM_ROUTE_MASK) >> SJ6_LFO2_PWM_ROUTE_SHIFT) - 1u;
 if (receiver < DNBX_LAYERS && (s->lfo2_pwm & SJ6_LFO2_PWM_DEPTH_MASK) &&
     (s->lfo_running & 2u) && s->core.layer[receiver].params.waveform == 1.f &&
     s->core.raw[210u + receiver] > .5f && frames && frames <= DNBX_MAX_FRAMES &&
     left && right && left != right) {
  rendered=render_lfo_pwm(s,1u,receiver,left,right,frames);
 } else {
  unsigned first=(unsigned)s->lfo1_pwm_route-1u;
  if(first<DNBX_LAYERS && s->core.layer[first].params.waveform==1.f &&
     s->core.raw[210u+first]>.5f &&
     frames && frames<=DNBX_MAX_FRAMES && left && right && left!=right)
   rendered=render_lfo_pwm(s,0u,first,left,right,frames);
  else {
   rendered=cut_route && frames<=DNBX_MAX_FRAMES && left && right && left!=right ?
      sj6_render_lfo1_cutoff(s,left,right,frames):render_typed_span(s,left,right,frames);
   if(rendered==DNBX_OK) s->lfo1_pwm_until_tick=0;
  }
  if (rendered==DNBX_OK) {
   if ((s->lfo2_pwm >> SJ6_LFO2_PWM_SHAPE_SHIFT) == SJ6_LFO_HELD) sj6_held_bypass(s,frames);
   else s->lfo2_pwm_until_tick = 0;
  }
 }
 if (wheel_cutoff||cut_route) for(unsigned l=0;l<2;++l) s->core.layer[l].params.cutoff_hz=base[l];
 if (rendered!=DNBX_OK)return 0;
 /* Keep all three audio-clock phases independent of receiver and span. */
 for (unsigned i = 0, running = s->lfo_running; running; ++i, running >>= 1)
  if (running & 1u) s->lfo_phase_q40[i] =
      (s->lfo_phase_q40[i] + s->lfo_step_q40[i] * frames) & (SJ6_LFO_TURN - 1u);
#else
 if (dnbx_render(&s->core,left,right,frames,0)!=DNBX_OK)return 0;
#endif
 float hp_hz = s->hpf > 0.f ? 15+2000*s->hpf*s->hpf : 0.f;
#if X0X_SUPERJ6_BUILTIN_INIT
 if (s->hpf < -1.f) hp_hz=-s->hpf-1.f;
#endif
 if(hp_hz>0){
  const float hp=1/(1+TWO_PI*hp_hz/s->core.sample_rate);
  for(unsigned i=0;i<frames;i++){
   float y=hp*(s->typed_hp_state[0]+left[i]-s->typed_hp_input[0]);
   s->typed_hp_input[0]=left[i];s->typed_hp_state[0]=y;left[i]=y;
   y=hp*(s->typed_hp_state[1]+right[i]-s->typed_hp_input[1]);
   s->typed_hp_input[1]=right[i];s->typed_hp_state[1]=y;right[i]=y;
  }
 }
 return 1;
}

static unsigned legacy_stage(unsigned stage)
{
 if(stage==DNBX_ENV_IDLE)return 0;
 if(stage==DNBX_ENV_ATTACK)return 1;
 if(stage==DNBX_ENV_RELEASE_HOLD||stage==DNBX_ENV_RELEASE)return 4;
 return stage==DNBX_ENV_SUSTAIN?3u:2u;
}
static int juno6_read_typed_envelope(const juno6_t *s, unsigned filter, juno6_envelope_snapshot *out)
{
 if(!s||!out)return 0;
 juno6_envelope_snapshot value={0};
 const dnbx_layer_params *l=&s->core.layer[0].params;
 const dnbx_envelope_params *p=filter?&l->filter:&l->amp;
 value.adsr[0]=p->attack_s;value.adsr[1]=p->decay_s;
 value.adsr[2]=p->sustain;value.adsr[3]=p->release_s;
 value.selected_voice=-1;value.shared_amp_filter=0;
 for(unsigned i=0;i<DNBX_VOICES;i++)for(unsigned l=0;l<DNBX_LAYERS;l++){
  const dnbx_voice *v=&s->core.voice[i];
  const dnbx_envelope_state *e=filter?&v->layer[l].filter:&v->layer[l].amp;
  if(e->stage&&(value.selected_voice<0||e->level>value.level)){
   value.selected_voice=(int)i;value.level=e->level;
   value.stage=legacy_stage(e->stage);value.gate=v->gate;value.active=1;
  }
 }
 *out=value;return 1;
}

int juno6_read_envelope(const juno6_t *s, juno6_envelope_snapshot *out)
{
 if (!s || !out) return 0;
#if !X0X_SUPERJ6_BUILTIN_INIT
 if (!s->typed) return stock_read_envelope(s, out);
#endif
 return juno6_read_typed_envelope(s, 0, out);
}
#if X0X_SUPERJ6_BUILTIN_INIT
int juno6_read_panel_envelope(const juno6_t *s, unsigned filter, juno6_envelope_snapshot *out, unsigned *linked)
{
 *linked = s->core.filter_link;
 return juno6_read_typed_envelope(s, filter, out);
}
#endif

/* The mono interface folds the typed dry stereo channels at equal gain.
 * Bound temporary storage independently of the caller's block length. */
void juno6_render(juno6_t *s, float *out, unsigned frames)
{
#if !X0X_SUPERJ6_BUILTIN_INIT
 if (!s->typed) {
  stock_render(s, out, frames);
  return;
 }
#endif
 float right[32];
 while (frames) {
  unsigned n = frames < 32 ? frames : 32;
  (void)juno6_render_stereo(s, out, right, n);
  for (unsigned i = 0; i < n; ++i) out[i] = .5f * (out[i] + right[i]);
  frames -= n;
  out += n;
 }
}


#if defined(X0X_HOST) && defined(X0X_SJ6_DELAY_WITNESS)
/* Bounded non-ping-pong witness. Every owner supplies its own zeroed
 * lines; provision one guard cell for fractional taps. */
int juno6_delay24_arm(juno6_t *s,float *l,float *r,unsigned capacity,float fb,unsigned profile)
{
    if(!s||!s->typed||s->delay24.tap||!l||!r||l==r||capacity<2u||capacity>66151u||
       !(fb>=0.f&&fb<=.99f)||profile!=10000u)return 0;
    s->delay24.left=l;s->delay24.right=r;s->delay24.tap=1;s->delay24.capacity=capacity;s->delay24.feedback=fb;
    s->delay24.fb_raw=(unsigned)roundf(fb*(127.f/.99f));
    s->delay24.delay=s->delay24.delay_target=1.f;
    s->delay24.bpm=120.f;
    s->delay24.delay_elapsed=s->delay24.fb_elapsed=10000u;
    s->delay24.fb_target=fb;
    s->delay24.dry=s->delay24.target=1.f;s->delay24.elapsed=10000u;
    return 1;
}
static float delay25_frames(const juno6_t *s,unsigned raw,float bpm,unsigned sync)
{
    static const float grid[18]={0,.0234375f,.03125f,.046875f,.0625f,.09375f,
        .125f,.1875f,.25f,.375f,.5f,.75f,1,2,3,4,6,8};
    if(raw>127u || sync>1u || !(bpm>0.f) || !isfinite(bpm))return 0;
    float u=(float)raw/127.f;
    unsigned i=sync?(unsigned)roundf(17.f*u):0;
    float seconds=i ? grid[i]*(240.f/bpm) : 1.5f*u;
    float frames=seconds*s->sample_rate;
    if(!(frames<=s->delay24.capacity-1u) || frames<0.f)return 0;
    /* Causal fixture interpolation requires one frame at zero time. */
    if(frames<1.f)frames=1.f;
    return frames;
}
int juno6_delay25_can(const juno6_t *s,unsigned raw,float bpm,unsigned sync)
{
    return s&&s->delay24.tap&&delay25_frames(s,raw,bpm,sync)>0.f;
}
int juno6_delay25_profile(juno6_t *s,unsigned raw,float bpm,unsigned sync)
{
    if(!s||!s->delay24.tap)return 0;
    const float frames=delay25_frames(s,raw,bpm,sync);
    if(!(frames>0.f))return 0;
    if(frames!=s->delay24.delay_target){
        s->delay24.delay_from=s->delay24.delay;
        s->delay24.delay_target=frames;s->delay24.delay_elapsed=0;
    }
    s->delay24.time_raw=raw;s->delay24.bpm=bpm;s->delay24.sync=sync;
    s->delay24.clock_refused=0;
    return 1;
}
int juno6_delay25_clock(juno6_t *s,float bpm,unsigned sync)
{
    return s&&juno6_delay25_profile(s,s->delay24.time_raw,bpm,sync);
}
unsigned juno6_delay_position(const juno6_t *s,unsigned slot)
{
    if(slot==SJ6_DELAY24)return s->delay24.enabled;
    if(slot==SJ6_DELAY25)return s->delay24.time_raw;
    if(slot==SJ6_DELAY26)return s->delay24.fb_raw;
    if(slot==SJ6_DELAYSYNC)return s->delay24.sync;
    return s->delay24.mix;
}
int juno6_delay24_set(juno6_t *s,unsigned slot,unsigned pos)
{
    if(!s||!s->delay24.tap)return 0;
    if(slot==SJ6_DELAY25){
        if(s->delay24.clock_refused)return 0;
        return juno6_delay25_profile(s,pos,s->delay24.bpm,s->delay24.sync);
    }
    if(slot==SJ6_DELAY26 && pos<=127u){
        float fb=.99f*((float)pos/127.f);
        if(fb!=s->delay24.fb_target){
            s->delay24.fb_from=s->delay24.feedback;s->delay24.fb_target=fb;
            s->delay24.fb_elapsed=0;
        }
        s->delay24.fb_raw=pos;return 1;
    }
    if(slot==SJ6_DELAY24 && pos<=1u)s->delay24.enabled=pos;
    else if(slot==SJ6_DELAY27 && pos<=127u)s->delay24.mix=pos;
    else return 0;
    const float raw=(float)s->delay24.mix/127.f;
    const float target=s->delay24.enabled?(float)(1.0-.9*pow((double)raw,1.1)):1.f;
    if(target!=s->delay24.target){s->delay24.target=target;s->delay24.pending=1;}
    return 1;
}
void juno6_delay24_process(juno6_t *s,float *l,float *r,unsigned n)
{
    if(!s->delay24.tap)return;
    for(unsigned i=0;i<n;i++){
        if(!s->delay24.tick){
            if(s->delay24.delay_elapsed>=10000u)s->delay24.delay=s->delay24.delay_target;
            else{
                s->delay24.delay=fmaf(s->delay24.delay_target-s->delay24.delay_from,
                    (float)s->delay24.delay_elapsed/10000.f,s->delay24.delay_from);
                s->delay24.delay_elapsed+=8u;
            }
            if(s->delay24.fb_elapsed>=10000u)s->delay24.feedback=s->delay24.fb_target;
            else{
                s->delay24.feedback=fmaf(s->delay24.fb_target-s->delay24.fb_from,
                    (float)s->delay24.fb_elapsed/10000.f,s->delay24.fb_from);
                s->delay24.fb_elapsed+=8u;
            }
            if(s->delay24.pending){
                s->delay24.from=s->delay24.dry;s->delay24.elapsed=0;s->delay24.pending=0;
            }
            if(s->delay24.elapsed>=10000u)s->delay24.dry=s->delay24.target;
            else{
                s->delay24.dry=fmaf(s->delay24.target-s->delay24.from,
                    (float)s->delay24.elapsed/10000.f,s->delay24.from);
                s->delay24.elapsed+=8u;
            }
        }
        s->delay24.tick=(s->delay24.tick+1u)&7u;
        unsigned at=s->delay24.pos;
        unsigned whole=(unsigned)s->delay24.delay, cap=s->delay24.capacity;
        unsigned a=(at+cap-whole)%cap, b=a ? a-1u : cap-1u;
        float frac=s->delay24.delay-(float)whole;
        float left=fmaf(frac,s->delay24.left[b]-s->delay24.left[a],s->delay24.left[a]);
        float right=fmaf(frac,s->delay24.right[b]-s->delay24.right[a],s->delay24.right[a]);
        s->delay24.left[at]=fmaf(s->delay24.feedback,left,l[i]);
        s->delay24.right[at]=fmaf(s->delay24.feedback,right,r[i]);
        if(++at==s->delay24.capacity)at=0;
        s->delay24.pos=at;
        if(s->delay24.dry!=1.f && !s->delay24.clock_refused){
            float wet=1.f-s->delay24.dry;
            l[i]=fmaf(wet,left,s->delay24.dry*l[i]);
            r[i]=fmaf(wet,right,s->delay24.dry*r[i]);
        }
    }
}
#endif
