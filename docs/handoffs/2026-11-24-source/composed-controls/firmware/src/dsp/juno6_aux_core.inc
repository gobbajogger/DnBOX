/* Read/write only established native receivers selected by the original
 * auxiliary LFO destination table. All settings below are span-local. */
static int juno6_aux_core_supported(unsigned id)
{
    return id==7u || id==8u || (id>=36u&&id<=39u) || (id>=72u&&id<=75u) ||
           (id>=124u&&id<=126u) || (id>=131u&&id<=136u)
#if X0X_SJ6_NOISE_SUB_NATIVE
           || id==201u || id==202u
#endif
           ;
}
__attribute__((noinline)) static float sj6_aux_infer_cutoff(float hz)
{
    float a=0.f,b=1.f;
    for(unsigned n=0;n<12u;n++){
        const float x=(a+b)*.5f;
        if(sj6_normal_cutoff_hz(x)<hz)a=x;else b=x;
    }
    return (a+b)*.5f;
}
static float juno6_aux_core_base(const juno6_t *s,unsigned id)
{
    const dnbx_layer_params *p=&s->core.layer[0].params;
    float raw;
    if(!juno6_aux_core_supported(id))return -1.f;
#if X0X_SJ6_NOISE_SUB_NATIVE
    if(id==201u)return s->core.sj6_noise_code*(1.f/127.f);
    if(id==202u)return s->core.sj6_sub_code*(1.f/127.f);
#endif
    if(dnbx_read_raw(&s->core,id,&raw)==DNBX_OK)return fm_clampf(raw,0.f,1.f);
    if(id==7u)return sj6_aux_infer_cutoff(p->cutoff_hz);
    if(id==8u)return p->resonance;
    if((id>=72u && id<=75u)||(id>=124u && id<=126u))return .5f;
    if(id==131u)return 0.f;
    if(id==136u)return fm_clampf(p->filter_env_hz*(1.f/7000.f),0.f,1.f);
    const unsigned filter=id>=132u, stage=id-(filter?132u:36u);
    const dnbx_envelope_params *e=filter?&p->filter:&p->amp;
    if(stage==2u)return filter?e->sustain:fm_powf(e->sustain,.4f);
    const float t=stage==0u?e->attack_s:stage==1u?e->decay_s:e->release_s;
    if(filter)return fm_clampf(fm_powf(t*(1.f/16.f),1.f/3.f),0.f,1.f);
    return fm_clampf(fm_powf(fm_maxf(t-.002f,0.f)/
                     (stage?23.998f:15.998f),1.f/(stage?3.35f:3.33f)),0.f,1.f);
}
typedef struct {
    dnbx_layer_params layer[2];
    float tune[2][DNBX_VOICES];
    float track,strength,offset[4],bend[3];
#if X0X_SJ6_NOISE_SUB_NATIVE
    float noise,sub;
#endif
    uint32_t present;
    unsigned active;
} sj6_aux_core_saved;
__attribute__((noinline,minsize)) static void juno6_aux_core_apply(juno6_t *s,sj6_aux_core_saved *save,unsigned id,float raw)
{
    dnbx_part *p=&s->core;
    if(!save->active){
        for(unsigned l=0;l<2u;l++){
            save->layer[l]=p->layer[l].params;
            for(unsigned i=0;i<DNBX_VOICES;i++)save->tune[l][i]=p->voice[i].layer[l].increment;
        }
        save->track=p->raw[131];save->strength=p->raw[136];
        save->present=p->raw_present[4];
#if X0X_SJ6_NOISE_SUB_NATIVE
        save->noise=p->sj6_noise_gain;save->sub=p->sj6_sub_gain;
#endif
        save->active=1u;
    }
    if(id>=124u && id<=126u){
        /* AU's independent global/layer trim, not the keyboard wheel. */
        const unsigned k=id-124u;
        save->bend[k]=(raw-.5f)*24.f;
        save->active|=1u<<(6u+k);
    } else if(id==7u){
        const float hz=sj6_normal_cutoff_hz(raw);
        for(unsigned l=0;l<2u;l++)p->layer[l].params.cutoff_hz=hz;
    } else if(id==8u){
        for(unsigned l=0;l<2u;l++)p->layer[l].params.resonance=raw;
    } else if(id==131u || id==136u){
        p->raw[id]=raw;p->raw_present[id/32u]|=1u<<(id%32u);
#if X0X_SJ6_NOISE_SUB_NATIVE
    } else if(id==201u){
        /* Original independent Noise gain: .707u^2.5. */
        p->sj6_noise_gain=.707f*raw*raw*fm_sqrtf(raw);
    } else if(id==202u){
        /* Original separate octave-down Sub fader: .808u. */
        p->sj6_sub_gain=.808f*raw;
#endif
    } else if(id>=72u && id<=75u){
        const unsigned l=id&1u,semi=id<74u;
        const float authored=semi?(float)juno6_layer_semi(s,l):(float)juno6_layer_fine(s,l);
        const float scale=semi?24.f:100.f;
        const float d=(float)(unsigned)(raw*scale+.5f)-authored;
        dnbx_layer_params q=p->layer[l].params;
        /* Repeated same-class writes replace that class in destination
         * order; an independent semitone and fine choice can coexist. */
        unsigned bit=1u<<(2u+l+(semi?0u:2u));
        if(save->active&bit){
            /* Reset that class only; use the current span's raw offset. */
            const unsigned idx=(semi?0u:2u)+l;
            const float prior=save->offset[idx];
            q.tune_semitones+= (d-prior)*(semi?1.f:.01f);
        }else q.tune_semitones+=d*(semi?1.f:.01f);
        save->offset[(semi?0u:2u)+l]=d;save->active|=bit;
        (void)dnbx_set_layer(p,l,&q);
    } else {
        const unsigned filter=id>=132u,stage=id-(filter?132u:36u);
        const float physical=filter?(stage==2u?raw:16.f*raw*raw*raw):amp_stage_from_unit(stage,raw);
        for(unsigned l=0;l<2u;l++){
            dnbx_envelope_params amp=p->layer[l].params.amp,eg=p->layer[l].params.filter;
            dnbx_envelope_params *e=filter?&eg:&amp;
            const float values[4]={physical,physical,physical,physical};
            apply_adsr(e,1u<<stage,values);
            dnbx_apply_panel_envelopes(p,l,&amp,&eg,p->filter_link);
        }
    }
}
__attribute__((noinline,minsize)) static void juno6_aux_core_bends(juno6_t *s,const sj6_aux_core_saved *save)
{
    if(!(save->active&0x1c0u))return;
    const float common=(save->active&64u)?save->bend[0]:0.f;
    for(unsigned l=0;l<DNBX_LAYERS;l++){
        const float local=(save->active&(128u<<l))?save->bend[l+1u]:0.f;
        dnbx_sj6_transient_bend(&s->core,l,common+local);
    }
}
__attribute__((noinline,minsize)) static void juno6_aux_core_restore(juno6_t *s,const sj6_aux_core_saved *save)
{
    dnbx_part *p=&s->core;
    if(!save->active)return;
    p->raw[131]=save->track;p->raw[136]=save->strength;p->raw_present[4]=save->present;
#if X0X_SJ6_NOISE_SUB_NATIVE
    p->sj6_noise_gain=save->noise;p->sj6_sub_gain=save->sub;
#endif
    for(unsigned l=0;l<2u;l++){
        p->layer[l].params=save->layer[l];
        dnbx_apply_panel_envelopes(p,l,&save->layer[l].amp,&save->layer[l].filter,p->filter_link);
        for(unsigned i=0;i<DNBX_VOICES;i++)p->voice[i].layer[l].increment=save->tune[l][i];
    }
}
