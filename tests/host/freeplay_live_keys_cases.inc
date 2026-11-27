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
/* Admission contexts must not impersonate a true physical release. */
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
        case 15: engine_request_notes_off();break; /* explicit Panic exception */
        case 16: smk_clip_enter_cleanup();test_clip_mode=1;break;
        case 17: project_busy=1;break;
        case 18: perf.test=0;perf.phase=1;break;
        case 19: set_view(V_SEQ);ui.freeplay[0]=0;break;
        }
        scan();
        if(path==0) { T(ui.part==PART_303B);T(live_owner==1); }
        int retained=path!=15;
        T(!!engine_live_key_probe(0,2)==retained);
        T(!!(engine_bass_keys_held()&1)==retained);
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
static void selected_and_transport(int bank)
{
    fresh_keys(bank);
    edge(17,100); T(engine_live_key_probe(0,2));
    uint32_t before,after,context;int arp;
    engine_live_key_state(0,&before,&arp,&context);
    enc_acc[EN_ALGO]=-1;scan();T(live_owner==1);
    edge(19,100);T(engine_live_key_probe(0,2)&&engine_live_key_probe(1,2));
    btn(B_PLAY,1);btn(B_PLAY,0);scan();T(seq.playing);
    btn(B_PLAY,1);btn(B_PLAY,0);scan();T(!seq.playing);
    T(engine_live_key_probe(0,2)&&engine_live_key_probe(1,2));
    T(engine_live_key_probe(0,1)==53 && engine_live_key_probe(1,1)==55);
    T(!engine_bass_select_preset(0,bank==0?1:0,0,&proj.sound));
    tick(250);T(engine_bass_bank(0)==bank && engine_bass_preset_pending(0)==2);
    edge(17,0);T(!engine_live_key_probe(0,2)&&engine_live_key_probe(1,2));
    engine_live_key_state(0,&after,&arp,&context);T(before==after);
    for(unsigned i=0;i<2200 && engine_bass_preset_pending(0);i++)audio_block();
    if(engine_bass_bank(0)!=(bank==0?1:0))fprintf(stderr,"preset wait bank-old=%d actual=%d pending=%d gate=%u held=%u\n",bank,engine_bass_bank(0),engine_bass_preset_pending(0),engine_live_key_probe(0,2),engine_bass_keys_held());
    T(engine_bass_bank(0)==(bank==0?1:0));
    edge(19,0);T(!engine_live_key_probe(1,2));
    /* HAL overflow and resync must remember real key-up even if the same
       physical index is already back down before this input scan. */
    fresh_keys(bank);edge(17,100);
    for(unsigned i=0;i<40;i++)physical(0,(i&1)?0:96);
    physical(17,0);physical(17,96);scan();
    T(!engine_live_key_probe(0,2));
    edge(17,0);edge(17,96);T(engine_live_key_probe(0,2));
    /* The audio-side FIFO can be full even after the HAL reported release.
       Retain the press-time off request and retry instead of cancelling A. */
    engine_live_key_state(0,&before,&arp,&context);
    unsigned pushed=0;
    while(engine_live_key(0,before,1,122,0,0,0))++pushed;
    T(pushed==31);
    physical(17,0);ui_input_only();
    T((live_keys[0].down[17]&4) && engine_live_key_probe(0,2));
    audio_block();ui_input_only();audio_block();
    T(!engine_live_key_probe(0,2) && !live_keys[0].down[17]);
    engine_live_key_state(0,&after,&arp,&context);T(before==after);
    edge(17,100);T(engine_live_key_probe(0,2));
    /* User project reset stays pending; genuine release drains first. */
    const unsigned rev=project_revision;
    T(ui_project_reset()==UI_PROJECT_BUSY && !project_reset_wait);
    run_action(ACT_RESET,0);T(project_reset_wait);
    scan();T(project_revision==rev&&engine_live_key_probe(0,2));
    edge(17,0);ui_frame();T(!project_reset_wait&&project_revision!=rev);
}
static void page_hold_release(int bank)
{
    static const unsigned keys[]={0,17,36};
    for(unsigned i=0;i<3;++i){
        fresh_keys(bank);
        ui.freeplay[0]=0;scan();edge(keys[i],100);
        const unsigned pitch=36+keys[i];
        uint32_t gen,now,ctx;int arp;
        engine_live_key_state(0,&gen,&arp,&ctx);
        T(engine_live_key_probe(0,1)==pitch);
        /* Real UI view action refuses new musical onsets in this context. */
        btn(B_SEQ,1);btn(B_SEQ,0);scan();
        T(ui.view==V_SEQ && live_release_only && engine_live_key_probe(0,2));
        T(engine_live_key_probe(0,1)==pitch);
        /* An outer press must not replace our earlier voice on the step page. */
        const unsigned other=keys[i]==0?36:0;
        edge(other,100); T(engine_live_key_probe(0,1)==pitch);
        edge(other,0); T(engine_live_key_probe(0,1)==pitch);
        ui.oct=1;scan();
        T(engine_live_key_probe(0,1)==pitch);
        /* A release followed by a fresh press within the same scan must
         * release the old voice, but cannot admit the new press on SEQ. */
        physical(keys[i],0);physical(keys[i],100);scan();
        T(!engine_live_key_probe(0,2));
        edge(keys[i],0);T(!engine_live_key_probe(0,2));
        engine_live_key_state(0,&now,&arp,&ctx);
        T(now==gen); /* normal one release, not a cancel/re-generation */
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
        owner_switches(bank); page_hold_release(bank);
        selected_and_transport(bank);
    }
#if defined(X0X_S1C) && X0X_S1C && defined(X0X_SUPERJ6_FACTORY) && X0X_SUPERJ6_FACTORY
    selected_and_transport(ORIGINAL_SOUND_SYNTHONE);
    selected_and_transport(ORIGINAL_SOUND_JUNO);
#endif
    T(!irq_depth);
    printf("PASS %u checks: 37 keys x 6 octaves x {Chicago, Miami} x {EDIT, SEQ, list, HELP}, edge keys x50; first-render edges, modal/part/transport retention, preset deferral, overflow and project reset\n",checks);
    return 0;
}
