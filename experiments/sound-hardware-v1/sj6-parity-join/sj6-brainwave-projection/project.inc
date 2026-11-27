static unsigned unit_detent(float x) {return (unsigned)(127.f*x+.5f);}
static unsigned typed_unit(float x) {return 1u+unit_detent(x);}
static int apply_brainwave_projection(original_runtime_part *p,unsigned wet,unsigned unison_probe)
{
    juno6_t *s=&p->dsp.juno.synth;
    if(!s->typed)return 0;
    /* Set only built-in source-backed receivers. The diagnostic doesn't
       claim the missing IDs ran, and does not substitute receipt identity. */
    const juno6_oscillator_edit waves[2]={{P82,P78,P210},{P83,P79,P211}};
    juno6_apply_factory_waves(s,waves);
    juno6_apply_pre_note_range(s,P86);
    const uint8_t envelope[SJ6_EXTRA_COUNT]={
        unit_detent(P36),unit_detent(P37),unit_detent(P38),unit_detent(P39),
        unit_detent(P132),unit_detent(P133),unit_detent(P134),unit_detent(P135),
        P116>=.5f
    };
    juno6_apply_panel_envelopes(s,0x1ff,envelope,0);
    if(P116==0 && !juno6_apply_independent_amp1_attack(s,P36))return 0;
    const float amp2[4]={P119,P120,P121,P122};
    for(unsigned i=0;i<4;i++)if(original_runtime_param(p,JUNO6_APP_PARAM_BASE+119u+i,amp2[i])!=ORIGINAL_RUNTIME_OK)return 0;
    if(original_runtime_param(p,JUNO6_APP_PARAM_BASE,P0)!=ORIGINAL_RUNTIME_OK ||
       original_runtime_param(p,2,P7)!=ORIGINAL_RUNTIME_OK ||
       original_runtime_param(p,3,P8)!=ORIGINAL_RUNTIME_OK)return 0;
    juno6_set_sj6_hpf(s,P28);
    const float vib[2]={P105,P106};
    juno6_set_vibrato_words(s,vib,3u,1);
    if(!juno6_set_filter_velocity(s,unit_detent(P137)))return 0;
    uint8_t room[SJ6_ROOM_COUNT]={0};
    room[SJ6_ROOM_ENABLE]=wet; room[SJ6_ROOM_TYPE]=P12>=.5f;
    room[SJ6_ROOM_SIZE]=unit_detent(P13);room[SJ6_ROOM_LOCUT]=unit_detent(P14);room[SJ6_ROOM_MIX]=unit_detent(P15);
    room[SJ6_ROOM_WIDEN]=P16>=.5f;room[SJ6_ROOM_RATE]=unit_detent(P22);room[SJ6_ROOM_DEPTH]=unit_detent(P23);
    original_runtime_sj6_room(p,room);
    if(s->macro[2]!=P7||s->macro[3]!=P8||s->macro[9]!=P28||p->dsp.juno.room.enable!=wet)return 0;
    const struct {unsigned id;float original;} exact[]={
        {0,P0},{7,P7},{36,P36},{78,P78},{79,P79},{82,P82},{83,P83},
        {86,P86},{105,P105},{106,P106},{119,P119},{120,P120},{121,P121},{122,P122},
        {210,P210},{211,P211}
    };
    for(unsigned i=0;i<sizeof exact/sizeof *exact;i++){
        float raw=0.f;
        if(dnbx_read_raw(&s->core,exact[i].id,&raw)!=DNBX_OK||raw!=exact[i].original){
            fprintf(stderr,"raw read failure %u %a vs %a\n",exact[i].id,raw,exact[i].original);
            return 0;
        }
        fprintf(stderr,"raw %u %a\n",exact[i].id,raw);
    }
    if(unison_probe){
        static int16_t history[2][SJ6_UNISON_HISTORY];
        /* On, authored detune, stereo, type. Type 0 is the actual first
           physical stop; this check deliberately admits only that stop. */
        if(P61!=0.0f)return 0;
        const uint8_t u[4]={P59>=.5f,typed_unit(P62),unit_detent(P60),0};
        juno6_unison_config(s,u,history);
        if(s->unison.taps!=1u)return 0;
        fprintf(stderr,"authored Unison receiver active\n");
    }
    fprintf(stderr,"projection verified; not full preset\n");
    return 1;
}
