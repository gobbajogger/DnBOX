/* SPDX-License-Identifier: GPL-3.0-only */
void juno6_init(juno6_t *s,float sr)
{
 stock_init(s,sr);
 (void)dnbx_init_superj6(&s->core,s->sample_rate);
 /* Register is neutral until a factory load owns the raw control. Leave its
  * presence bit clear so an ordinary preset still reports it as unbound. */
 s->core.raw[86]=.5f;
 s->master_gain=1.f;
}
int juno6_note_on(juno6_t *s,uint8_t note,uint8_t velocity)
{
 if (!s->typed) return stock_note_on(s,note,velocity);
 return dnbx_note_on(&s->core,note,velocity)>=0;
}
void juno6_note_off(juno6_t *s,uint8_t note)
{
 if (!s->typed) stock_note_off(s,note);
 else (void)dnbx_note_off(&s->core,note);
}
void juno6_all_off(juno6_t *s)
{
 stock_all_off(s);
 (void)dnbx_all_off(&s->core);
 memset(s->typed_hp_state,0,sizeof s->typed_hp_state);
 memset(s->typed_hp_input,0,sizeof s->typed_hp_input);
}
int juno6_active(const juno6_t *s)
{
 return s->typed ? dnbx_active_voices(&s->core)>0 : stock_active(s);
}

int juno6_can_edit_amp2(const juno6_t *s)
{
 return s->typed || !stock_active(s);
}

/* Direct AMP2 events target only layer 2, including while AMP link is on.
 * The control converters and the envelope sample renderer are owned by DnBOX. */
static int set_amp2(dnbx_part *core, unsigned id, float raw)
{
    dnbx_layer_params layer;
    if (dnbx_get_layer(core, 1, &layer) != DNBX_OK) return 0;
    switch (id) {
    case 119:
        if (dnbx_attack_from_unit(raw, &layer.amp.attack_s) != DNBX_OK) return 0;
        break;
    case 120:
        layer.amp.decay_s = .002f + 23.998f * fm_powf(raw, 3.35f);
        break;
    case 121:
        layer.amp.sustain = fm_powf(raw, 2.5f);
        break;
    case 122:
        layer.amp.release_s = .002f + 23.998f * fm_powf(raw, 3.35f);
        break;
    default:
        return 0;
    }
    if (dnbx_set_layer(core, 1, &layer) != DNBX_OK) return 0;
    return dnbx_store_raw(core, id, raw) == DNBX_RAW_STORED_ONLY;
}

/* Called for an independent layer-one edit on the idle audio owner. */
void juno6_apply_pre_note_attack(juno6_t *s, float raw)
{
    dnbx_layer_params layer;
    if (dnbx_get_layer(&s->core, 0, &layer) != DNBX_OK ||
        dnbx_attack_from_unit(raw, &layer.amp.attack_s) != DNBX_OK ||
        dnbx_set_layer(&s->core, 0, &layer) != DNBX_OK) return;
    (void)dnbx_store_raw(&s->core, 36, raw);
    s->typed = 1;
}

/* Independent compact PWM LFO. Hz mode does not establish a factory route. */
void juno6_apply_lfo1_hz(juno6_t *s, float raw)
{
    float hz;
    if (!s->typed || dnbx_lfo_from_unit(raw, &hz) != DNBX_OK ||
        dnbx_store_raw(&s->core, 88, raw) != DNBX_RAW_STORED_ONLY) return;
    /* The phase increment reads this parameter directly. Keep running
     * phases, envelopes and voice increments untouched. */
    for (unsigned layer = 0; layer < DNBX_LAYERS; ++layer)
        s->core.layer[layer].params.lfo_hz = hz;
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
static float factory_tune(const juno6_t *s)
{
    return s->bend + 24.f * (s->core.raw[86] - .5f);
}

/* The measured sub and master scalar paths use 101/125 and 101/250.
 * Round the integer significand once, without soft-double calls on target. */
static float volume_from_raw(float raw, unsigned half)
{
    if (raw == 0.f) return 0.f;
    fm_bits_t value = {.f = raw};
    unsigned exponent = value.u >> 23;
    /* Preserve the previous master behavior for non-factory tiny values. */
    if (half && exponent < 3) return raw * .404f;
    uint32_t significand = (value.u & 0x7fffffu) | (exponent ? 0x800000u : 0u);
    uint32_t numerator = significand * 101u;
    if (exponent <= 1) {
        value.u = (numerator + 62u) / 125u;
        return value.f;
    }
    unsigned result_exponent = exponent - 1 - half;
    if (numerator < (125u << 23)) {
        numerator *= 2;
        --result_exponent;
    }
    /* The denominator is odd, so there is no tie. */
    value.u = (result_exponent << 23) + (numerator + 62u) / 125u;
    return value.f;
}

void juno6_apply_factory_waves(juno6_t *s, const juno6_oscillator_edit waves[2], float sub_volume)
{
    enum { SUB_VOLUME_ID = 202 };
    float sub_gain = s->sub_level;
    if (sub_volume >= 0.f) {
        sub_gain = volume_from_raw(sub_volume, 0);
        (void)dnbx_store_raw(&s->core, SUB_VOLUME_ID, sub_volume);
    } else {
        s->core.raw_present[SUB_VOLUME_ID / 32] &= ~(1u << (SUB_VOLUME_ID % 32));
    }
    for (unsigned layer = 0; layer < DNBX_LAYERS; ++layer) {
        s->core.layer[layer].params.lfo_pwm_depth = 0.f;
        s->core.layer[layer].params.sub_gain = sub_gain;
        s->core.layer[layer].params.tune_semitones = factory_tune(s);
        juno6_apply_pre_note_wave(s, layer, &waves[layer]);
    }
}

int juno6_set(juno6_t *s,unsigned id,float x)
{
 if(!s)return 0;
 if(id==JUNO6_APP_PARAM_BASE){
  if(!(x>=0 && x<=JUNO6_FACTORY_MASTER_MAX))return 0;
  if(dnbx_store_raw(&s->core,0,x)!=DNBX_RAW_STORED_ONLY)return 0;
  s->master_gain=volume_from_raw(x,1);
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
 if(!stock_set(s,id,x))return 0;
 if(id==17)return dnbx_set_sustain(&s->core,x>=.5f)==DNBX_OK;
 if(id==9)return 1;
 if(id==13||id==14)return 1; /* Existing runtime owns stock chorus parameters. */
 for(unsigned l=0;l<DNBX_LAYERS;l++){
  dnbx_layer_params p;
  if(dnbx_get_layer(&s->core,l,&p)!=DNBX_OK)return 0;
  switch(id){
  case 0:
   if (p.oscillator_mode == DNBX_OSC_SUPERJ6) p.oscillator_mode = DNBX_OSC_STOCK_BLEP;
   p.waveform=(2+x)/3;break;
  case 1:p.sub_gain=s->sub_level;break;
  case 2:p.cutoff_hz=35+x*x*12000;break;
  case 3:p.resonance=s->resonance;break;
  case 4:p.filter_env_hz=7000*x;break;
  case 5:p.amp.attack_s=p.filter.attack_s=s->attack;break;
  case 6:p.amp.decay_s=p.filter.decay_s=s->decay;break;
  case 7:p.amp.sustain=p.filter.sustain=s->sustain;break;
  case 8:p.amp.release_s=p.filter.release_s=s->release;break;
  case 10:p.lfo_hz=.1f+8*x*x;break;
  case 11:p.lfo_pwm_depth=x*.2f;break;
  case 12:p.pulse_width=clampf(s->pulse_width,.03f,.5f);break;
  case 15:p.gain=x;break;
  case 16:p.tune_semitones=factory_tune(s);break;
  default:break;
  }
  if(dnbx_set_layer(&s->core,l,&p)!=DNBX_OK)return 0;
 }
 return 1;
}

int juno6_render_stereo(juno6_t *s,float *left,float *right,unsigned frames)
{
 if(!s->typed){
  stock_render(s,left,frames);
  memcpy(right,left,frames*sizeof *right);
  return 1;
 }
 if(dnbx_render(&s->core,left,right,frames,0)!=DNBX_OK)return 0;
 if(s->hpf>0){
  const float hp=1/(1+TWO_PI*(15+2000*s->hpf*s->hpf)/s->core.sample_rate);
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
int juno6_read_envelope(const juno6_t *s,juno6_envelope_snapshot *out)
{
 if(!s||!out)return 0;
 if(!s->typed)return stock_read_envelope(s,out);
 juno6_envelope_snapshot value={0};
 const dnbx_envelope_params *p=&s->core.layer[0].params.amp;
 value.adsr[0]=p->attack_s;value.adsr[1]=p->decay_s;
 value.adsr[2]=p->sustain;value.adsr[3]=p->release_s;
 value.selected_voice=-1;value.shared_amp_filter=0;
 for(unsigned i=0;i<DNBX_VOICES;i++)for(unsigned l=0;l<DNBX_LAYERS;l++){
  const dnbx_voice *v=&s->core.voice[i];
  const dnbx_envelope_state *e=&v->layer[l].amp;
  if(e->stage&&(value.selected_voice<0||e->level>value.level)){
   value.selected_voice=(int)i;value.level=e->level;
   value.stage=legacy_stage(e->stage);value.gate=v->gate;value.active=1;
  }
 }
 *out=value;return 1;
}

/* The mono interface folds the typed dry stereo channels at equal gain.
 * Bound temporary storage independently of the caller's block length. */
void juno6_render(juno6_t *s, float *out, unsigned frames)
{
 if (!s->typed) {
  stock_render(s, out, frames);
  return;
 }
 float right[32];
 while (frames) {
  unsigned n = frames < 32 ? frames : 32;
  (void)juno6_render_stereo(s, out, right, n);
  for (unsigned i = 0; i < n; ++i) out[i] = .5f * (out[i] + right[i]);
  frames -= n;
  out += n;
 }
}
