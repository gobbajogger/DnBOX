/* SPDX-License-Identifier: GPL-3.0-only */
/* Free Play on the SMK-37: all 37 physical keys (27 central + 10 outer edge keys)
 * give a note-on on every synth engine, at every octave, also while a settings
 * list or HELP is open on the synth page. Driven through the production
 * smk__key_event -> smk_extq/plat_outer_keys -> ui live route -> engine. */
static unsigned checks;
#define T(c) do{checks++;if(!(c)){fprintf(stderr,"FAIL %d %s\n",__LINE__,#c);exit(1);}}while(0)
#define TK(c,p,o,b) do{checks++;if(!(c)){fprintf(stderr,"FAIL %d %s (key %u oct %d bank %d)\n",__LINE__,#c,(unsigned)(p),(int)(o),(int)(b));exit(1);}}while(0)
static void audio_block(void){int32_t out[512];engine_render(out,256);}
static void physical(unsigned p,unsigned velocity)
{
    smk__key_event(p,velocity);held_keys=fm1_in.notes;
    for(unsigned k=0;k<NKEYS;k++)key_velocity[k]=fm1_in.key_vel[k+5];
}
static void scan(void){ui_input_only();audio_block();ui_input_only();audio_block();}
static void edge(unsigned p,unsigned velocity){physical(p,velocity);scan();}
static void fresh_keys(int bank)
{
    for(unsigned p=0;p<37;p++)physical(p,0);
    ui.freeplay[0]=ui.freeplay[1]=0;ui.rec=0;ui.overlay=O_NONE;ui.help=0;
    test_clip_mode=test_padcal=test_smcui=0;held_btn=ui.btn=0;ui.held_step=-1;
    seq_stop(&seq);scan();engine_bass_off(0);engine_bass_off(1);tick(200);
    T(!engine_bass_select_preset(0,bank,0,&proj.sound));tick(200);
    T(engine_bass_bank(0)==bank);
    ui.part=PART_303A;set_view(V_PART);build_pages();ui.oct=0;
    audio_block();ui.freeplay[0]=1;scan();scan();T(live_owner==0);
    /* Only the admitted voice acknowledges its cancellation here. An inactive
     * voice can retain its wait bit after audio has already released its keys. */
    T(!(live_wait&1u));
}
static void set_oct(int o){ui.oct=(int8_t)o;scan();}
static void every_key(int bank,const char *ctx)
{
    for(int o=-2;o<=3;o++) {
        set_oct(o);
        for(unsigned p=0;p<37;p++) {
            unsigned pitch=(unsigned)(36+(int)p+12*o);
            edge(p,64+(p&31));
            TK(engine_live_key_probe(0,2)==1,p,o,bank);          /* gate on: the note sounds */
            TK(engine_live_key_probe(0,1)==pitch,p,o,bank);
            TK(engine_live_key_probe(0,0)==live_token(p),p,o,bank);
            edge(p,0);
            TK(!engine_live_key_probe(0,0),p,o,bank);
        }
    }
    set_oct(0);(void)ctx;
}
/* Owner report (0.0.13): the lowest/highest keys sometimes do not respond.
 * Fast taps, taps inside one scan, and taps beside held central keys. */
static void edge_keys_intermittent(int bank)
{
    static const unsigned outer[10]={0,1,2,3,4,32,33,34,35,36};
    for(int rep=0;rep<50;rep++) for(unsigned i=0;i<10;i++) {
        unsigned p=outer[i];
        edge(p,100);TK(engine_live_key_probe(0,2)==1 && engine_live_key_probe(0,1)==36+p,p,0,bank);
        edge(p,0);TK(!engine_live_key_probe(0,0),p,0,bank);
        /* held central key, then an edge key on top: the edge key owns the gate */
        edge(17,90);edge(p,100);TK(engine_live_key_probe(0,1)==36+p,p,0,bank);
        edge(p,0);TK(engine_live_key_probe(0,2)==1,p,0,bank);edge(17,0);TK(!engine_live_key_probe(0,2),p,0,bank);
        /* press right after a release of another edge key in the same scan */
        physical(outer[(i+1)%10],100);physical(outer[(i+1)%10],0);physical(p,100);scan();
        TK(engine_live_key_probe(0,1)==36+p,p,0,bank);edge(p,0);
    }
}
static void settings_do_not_swallow(int bank)
{
    open_list_of_pages();T(ui.overlay==O_LIST);scan();
    T(ui_live_key_owner()==1);every_key(bank,"list");
    ui.overlay=O_NONE;ui.help=1;scan();every_key(bank,"help");ui.help=0;
    set_view(V_SEQ);build_pages();scan();every_key(bank,"seq");
    set_view(V_PART);build_pages();scan();
    /* EDIT of a synth plays the keys with FREE PLAY off too (kbd_on): all 37, not only the central 27 */
    ui.freeplay[0]=0;scan();T(ui_live_key_owner()==1);every_key(bank,"edit-nofreeplay");
    open_list_of_pages();scan();every_key(bank,"edit-nofreeplay-list");ui.overlay=O_NONE;
    ui.freeplay[0]=1;scan();
}
/* B3: scan() above deliberately settles two input/render cycles. Check the
 * first cycle separately so a queued key cannot acquire an extra audio block
 * of delay while the all-keys coverage still passes. This observes the real
 * engine gate/identity, not a modeled DAC timestamp or envelope attack. */
static void first_render_edges(int bank)
{
    static const int views[2]={V_PART,V_SEQ};
    for(unsigned v=0;v<2;v++) {
        set_view(views[v]);build_pages();scan();
        T(ui_live_key_owner()==1);
        for(unsigned p=0;p<37;p++) {
            TK(!engine_live_key_probe(0,2),p,0,bank);
            physical(p,100);ui_input_only();audio_block();
            TK(engine_live_key_probe(0,2)==1,p,0,bank);
            TK(engine_live_key_probe(0,1)==36+p,p,0,bank);
            TK(engine_live_key_probe(0,0)==live_token(p),p,0,bank);
            physical(p,0);ui_input_only();audio_block();
            TK(!engine_live_key_probe(0,2),p,0,bank);
            TK(!engine_live_key_probe(0,0),p,0,bank);
        }
    }
}
/* Every owner-loss path must release the old engine key before its physical
 * key-up. A generation change without release used to abandon this key. */
static void owner_switches(int bank)
{
    for (int path=0;path<20;path++) {
        fresh_keys(bank);edge(17,100);T(engine_live_key_probe(0,2));
        uint32_t generation,context;int arp;
        engine_live_key_state(0,&generation,&arp,&context);
        switch(path) {
        case 0: enc_acc[EN_ALGO]=-1; break;
        case 1: ui.part=PART_909;build_pages();break; /* controller selection */
        case 2: set_view(V_HOME);break;
        case 3: set_view(V_GLO);break;
        case 4: set_view(V_MIX);break;
        case 5: set_view(V_FX);break;
        case 6: set_view(V_GEN);break;
        case 7: set_view(V_SONG);break;
        case 8: ui.overlay=O_ASK;break;
        case 9: ui.rec=1;break;
        case 10: test_clip_mode=1;break;
        case 11: test_clip_mode=1;break;   /* (was the pad calibration, removed) */
        case 12: test_smcui=1;break;
        case 13: held_btn=1u<<B_HOME;break;
        case 14: held_btn=1u<<B_SAVE;break;
        case 15: engine_live_key_force_serial(0,generation+1,context);break;
        case 16: engine_live_key_force_serial(0,generation+1,context+1);break;
        case 17: project_busy=1;break;
        case 18: perf.test=0;perf.phase=1;break;
        case 19: set_view(V_SEQ);ui.freeplay[0]=0;break;
        }
        scan();
        if(path==0) { T(ui.part==PART_303B);T(live_owner==1); }
        T(!engine_live_key_probe(0,2));
        T(!(engine_bass_keys_held()&1));
        edge(17,0);T(!engine_live_key_probe(0,2));
        test_clip_mode=test_padcal=test_smcui=0;held_btn=0;
        project_busy=0;perf.test=-1;perf.phase=0;
    }
    fresh_keys(bank);edge(17,100);enc_acc[EN_ALGO]=-1;scan();
    edge(18,100);T(engine_live_key_probe(1,2));
    edge(17,0);T(engine_live_key_probe(1,2)); /* stale A release cannot end B */
    edge(18,0);T(!engine_live_key_probe(1,2));
    fresh_keys(bank);edge(17,100);set_view(V_HOME);scan();
    int ch=seq.ch[TRK_BASS0];midi3(0x90|ch,65,100);audio_block();
    T(engine_bass_keys_held()&1);
    edge(17,0);T(engine_bass_keys_held()&1); /* foreign MIDI source stays held */
    midi3(0x80|ch,65,0);audio_block();T(!(engine_bass_keys_held()&1));
}
/* Cancellation acknowledgement belongs to each voice. Exercise the inactive
 * wait bit that a global !live_wait assertion incorrectly rejected, and prove
 * both the active voice and the cancelled voice accept fresh physical edges. */
static void cancelled_voice_reentry(int bank)
{
    fresh_keys(bank);enc_acc[EN_ALGO]=-1;scan();T(live_owner==1);
    edge(20,100);T(engine_live_key_probe(1,2));
    uint32_t before,after,context;int arp;
    engine_live_key_state(1,&before,&arp,&context);
    enc_acc[EN_ALGO]=1;scan();T(live_owner==0);
    engine_live_key_state(1,&after,&arp,&context);
    T(after!=before);T(!engine_live_key_probe(1,2));
    T(!(engine_bass_keys_held()&2));T(live_wait&2u);
    /* Selecting A must not need a visit to B to acknowledge B's cancellation. */
    fresh_keys(bank);T(live_wait&2u);
    edge(17,100);T(engine_live_key_probe(0,2));edge(17,0);
    enc_acc[EN_ALGO]=-1;scan();T(live_owner==1);T(!(live_wait&2u));
    edge(20,100);T(engine_live_key_probe(1,2));
    edge(20,0);T(!engine_live_key_probe(1,2));
}

static unsigned event_count,last_accent,last_velocity,last_pitch,last_token;
static uint64_t lifecycle_digest=UINT64_C(1469598103934665603);
int __real_engine_live_key(int,uint32_t,int,int,int,int,int);
int __wrap_engine_live_key(int voice,uint32_t serial,int token,int pitch,int velocity,int accent,int slide)
{
    ++event_count;last_accent=accent;last_velocity=velocity;last_pitch=pitch;last_token=token;
    unsigned words[]={voice,serial,token,pitch,velocity,accent,slide};
    for(unsigned i=0;i<sizeof(words)/sizeof(*words);i++){
        lifecycle_digest^=words[i];lifecycle_digest*=UINT64_C(1099511628211);
    }
    return __real_engine_live_key(voice,serial,token,pitch,velocity,accent,slide);
}
static void extended(int bank)
{
    fresh_keys(bank);
    unsigned before=event_count;
    edge(17,65);T(event_count==before+1 && last_accent==0 && last_velocity==65&&last_pitch==53 && last_token==live_token(17));
    edge(17,96);T(event_count==before+1); /* repeated down, one immutable owner */
    edge(17,0);T(event_count==before+2 && last_velocity==0 && !engine_live_key_probe(0,2));
    ui.btn=held_btn=1u<<B_ENV;
    edge(17,96);T(last_accent==1&&last_velocity==96);
    edge(17,0);T(last_accent==1&&!last_velocity);
    ui.btn=held_btn=0;
    edge(17,66);T(last_accent==0&&last_velocity==66); edge(17,0);
    /* The outer transport has a fixed 32-event ring. Repeated raw edges must
     * cause its actual overflow/cancel rather than leaking a ghost held note. */
    edge(20,100);T(engine_live_key_probe(0,2));
    for(unsigned i=0;i<40;i++)physical(0,i&1u?0:80);
    scan();T(!engine_live_key_probe(0,2));
    edge(20,0);edge(1,90);T(engine_live_key_probe(0,2));edge(1,0);
    if(bank==ORIGINAL_SOUND_SYNTHONE || bank==ORIGINAL_SOUND_JUNO){
        engine_arp_settings a;
        T(engine_arp_settings_read(0,&a));
        T(engine_arp_set(0,a.revision,ARP_F_ON,1)!=S1_EDIT_REJECTED);
        scan();scan();uint32_t gen,context;int arp;
        engine_live_key_state(0,&gen,&arp,&context);T(arp==0 && engine_arp_settings_read(0,&a) && a.on);
        before=event_count;edge(17,90);T((engine_bass_keys_held()&1) && event_count==before+1);
        edge(22,84);T((engine_bass_keys_held()&1) && event_count==before+2);
        edge(17,0);T((engine_bass_keys_held()&1) && event_count==before+3);
        edge(22,0);T(!(engine_bass_keys_held()&1) && event_count==before+4);
        T(engine_arp_settings_read(0,&a));
        T(engine_arp_set(0,a.revision,ARP_F_ON,0)!=S1_EDIT_REJECTED);
        scan();
    }
}
int main(int argc,char **argv)
{
    (void)argc;(void)argv;boot();tick(300);
    for(int bank=FACTORY_BANK_CHICAGO;bank<=FACTORY_BANK_MIAMI;bank++) {
        fresh_keys(bank);every_key(bank,"edit");
        edge_keys_intermittent(bank);
        settings_do_not_swallow(bank);
        first_render_edges(bank);
        owner_switches(bank);
        cancelled_voice_reentry(bank);
    }
    for(int bank=ORIGINAL_SOUND_CHICAGO;bank<=ORIGINAL_SOUND_JUNO;bank++)extended(bank);
    T(!irq_depth);printf("exact lifecycle event_count=%u hash=%016llx\n",event_count,(unsigned long long)lifecycle_digest);
    printf("PASS %u checks: 37 keys x 6 octaves x {Chicago, Miami} x {EDIT, SEQ, list, HELP}, edge keys x50; on/off in first render\n",checks);
    return 0;
}
