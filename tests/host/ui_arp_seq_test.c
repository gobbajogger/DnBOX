/* SPDX-License-Identifier: GPL-3.0-only */
/* SJ6 / Synth One ARPEGGIATOR group and pad-edited ARP SEQ page (UX-PROPOSAL.md
 * Decisions, 23 Nov 2026) against the host fake of Synths' hooks. Real input
 * path (held_keys / enc_acc / panel buttons) and the real framebuffer. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static unsigned failures, expectations;
#define EXPECT(x) do{expectations++;if(!(x)){failures++;fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);}}while(0)
static void tap_panel(int b){held_btn=1u<<b;++now_ms;input();held_btn=0;++now_ms;input();}
static void pad_down(int w){held_keys=1u<<WHITE_KEY[w];++now_ms;input();}
static void pad_up(void){held_keys=0;now_ms+=5;input();ui_frame();}
static void tap_pad(int w){pad_down(w);pad_up();}
static void turn(int k,int e){enc_acc[EN_K1+k]=e;++now_ms;input();ui_frame();}
static int page_named(const char *t){for(int i=0;i<pg.n;i++)if(!strcmp(pg.title[i],t))return i;return -1;}
static int lit(int w){leds();return (lit_keys>>WHITE_KEY[w])&1u;}
static int on_screen(const char *s);
static void roll_frame(void){seq.playing=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui.msg_until=0;ui_frame();seq.playing=0;}
static void prime(int kind)
{
    setup(V_SEQ,PART_303A);
    EXPECT(!engine_bass_select_preset(0,kind,0,&proj.sound));
    for(int n=0;n<1024&&engine_bass_preset_pending(0);n++){int32_t a[512];engine_render(a,256);}
    EXPECT(engine_bass_bank(0)==kind);
    arp_fake_reset();arp_fake.offmax[0]=kind==ORIGINAL_SOUND_JUNO?24:12;
    ui.held_step=-1;build_pages();ui_frame();
}
static void engine_case(int kind,const char *tag)
{
    char name[64];
    prime(kind);
    /* SEQ: SEQUENCE -> PRESET -> KEYS -> ARP SEQ; K5 STEPS, K6 INTERVAL, K7 GATE, K8 empty. */
    EXPECT(pg.n==4&&page_named("ARP SEQ")==3);
    EXPECT(pg.r[3][0].kind==R_ARP&&pg.r[3][0].a==ARP_F_LENGTH&&pg.r[3][1].a==ARP_F_INTERVAL);
    EXPECT(pg.r[3][2].kind==R_ARP && pg.r[3][2].a==ARP_F_GATE && pg.r[3][3].kind==R_NONE);
    *page_of(V_SEQ)=3;build_pages();ui_frame();EXPECT(arp_seq_page());
    uint8_t pattern_before[sizeof cur_pat()->bass[0]];memcpy(pattern_before,&cur_pat()->bass[0],sizeof pattern_before);
    /* tap: step off, tap again: on (never the pattern's step) */
    tap_pad(4);EXPECT(!(arp_fake.st[0].on_mask&1u<<4));EXPECT(!lit(4));
    tap_pad(4);EXPECT(arp_fake.st[0].on_mask&1u<<4);EXPECT(lit(4));
    EXPECT(!memcmp(pattern_before,&cur_pat()->bass[0],sizeof pattern_before));
    /* hold + K5: semitone offset; the hold is not a tap afterwards */
    pad_down(5);turn(0,3);EXPECT(arp_fake.st[0].offset[5]==3);
    snprintf(name,sizeof name,"arp-seq-%s-held",tag);shot(name);
    EXPECT(on_screen("OFFSET"));EXPECT(on_screen("+3st"));EXPECT(on_screen("OCT+"));
    tap_panel(B_OCTUP);EXPECT(arp_fake.st[0].offset[5]==(kind==ORIGINAL_SOUND_JUNO?15:12));   /* OCT+: +12, clamped per engine */
    tap_panel(B_OCTDN);EXPECT(arp_fake.st[0].offset[5]==(kind==ORIGINAL_SOUND_JUNO?3:0));
    turn(2,1);EXPECT(arp_fake.st[0].oct_mask&1u<<5);          /* K7: OCT+ */
    turn(1,1);EXPECT(!(arp_fake.st[0].on_mask&1u<<5));        /* K6: the step off */
    unsigned before=arp_fake.step_sets;turn(3,1);EXPECT(arp_fake.step_sets==before);   /* K8: nothing */
    pad_up();EXPECT(!(arp_fake.st[0].on_mask&1u<<5));EXPECT(arp_held<0);
    EXPECT(!memcmp(pattern_before,&cur_pat()->bass[0],sizeof pattern_before));
    /* page knobs: K5 STEPS, K6 INTERVAL */
    for(int n=0;n<4;n++){turn(0,-1);}
    EXPECT(arp_fake.s[0].length==12);EXPECT(!lit(13));
    turn(1,-1);turn(1,-1);EXPECT(arp_fake.s[0].interval==10);
    char gate_num[16], gate_unit[8];
    pref_value(pg.r[3][2],gate_num,gate_unit);
    EXPECT(!strcmp(gate_num,"50") && !strcmp(gate_unit,"%"));
    turn(2,1);EXPECT(arp_fake.s[0].gate>50 && arp_fake.s[0].gate<=100);
    snprintf(name,sizeof name,"arp-seq-%s",tag);ui.msg_until=0;ui.touched=-1;now_ms+=3000;ui_frame();shot(name);
    EXPECT(!on_screen("VELOCITY"));                         /* the pattern's lane is not the arp's */
    /* playhead: only the step that really sounded, only while the arp is on */
    arp_fake.view_ok[0]=1;arp_fake.view_step[0]=2;arp_fake.view_age[0]=10;arp_fake.s[0].on=0;EXPECT(lit(2));
    arp_fake.s[0].on=1;EXPECT(!lit(2));arp_fake.view_age[0]=5000;EXPECT(lit(2));
    {   /* the roll's playhead column: only a fresh real step with the arp on */
        static uint16_t idle[240*120];
        arp_fake.view_ok[0]=0;roll_frame();memcpy(idle,fb+240*20,sizeof idle);
        arp_fake.view_ok[0]=1;arp_fake.view_age[0]=5000;roll_frame();EXPECT(!memcmp(idle,fb+240*20,sizeof idle));
        arp_fake.view_age[0]=10;arp_fake.s[0].on=0;roll_frame();EXPECT(!memcmp(idle,fb+240*20,sizeof idle));
        arp_fake.s[0].on=1;roll_frame();EXPECT(memcmp(idle,fb+240*20,sizeof idle)!=0);
        arp_fake.view_ok[0]=0;
    }
    /* EDIT: ARPEGGIATOR = ON, RANGE, MODE, SPEED (Chicago order), before EFFECT, then SOUND */
    set_view(V_PART);build_pages();
    int a=page_named("ARPEGGIATOR");EXPECT(a>=0);
    EXPECT(a>=0&&pg.r[a][0].a==ARP_F_ON&&pg.r[a][1].a==ARP_F_RANGE&&pg.r[a][2].a==ARP_F_MODE&&pg.r[a][3].a==ARP_F_SPEED);
    if(kind==ORIGINAL_SOUND_SYNTHONE)EXPECT(page_named("EFFECT")==a+1);
    EXPECT(!strcmp(pg.title[pg.n-1],"SOUND")&&a==pg.n-2-(kind==ORIGINAL_SOUND_SYNTHONE));
    for(int i=0;i<pg.n;i++){int n=0;for(int k=0;k<4;k++)n+=pg.r[i][k].kind!=R_NONE;EXPECT(n<=4);}
    *page_of(V_PART)=(uint8_t)a;build_pages();ui_frame();
    for(int n=0;n<3;n++){turn(2,1);}
    EXPECT(arp_fake.s[0].mode==ARP_SEQ);
    turn(1,1);EXPECT(arp_fake.s[0].range==2);
    snprintf(name,sizeof name,"arpeggiator-%s",tag);shot(name);
    EXPECT(on_screen("SEQ"));EXPECT(on_screen("2 OCT"));
    /* a stale revision is the engine's to reject; the UI says so and keeps going */
    arp_fake.ok[0]=0;build_pages();EXPECT(page_named("ARPEGGIATOR")<0);
}
static int on_screen(const char *s)
{
    const felucca_font_t *fonts[]={&FONT_T,&FONT_XS,&FONT_S,&FONT_B,&FONT_M};
    for(unsigned f=0;f<5;f++){
        int w=tw(fonts[f],s),h=fonts[f]->h;
        cv_begin(240,40,C_BLACK);cv_band(0,40);cv_text(0,0,fonts[f],s,C_WHITE);
        for(int y=0;y+h<240;y++)for(int x=0;x+w<240;x++){
            uint16_t ink=0;int ok=1,ninks=0;
            for(int j=0;j<h&&ok;j++)for(int i=0;i<w;i++){
                int mask=cv_px[j*240+i]==swap16(C_WHITE);uint16_t p=fb[(y+j)*240+x+i];
                if(mask){if(!ninks++)ink=p;else if(p!=ink){ok=0;break;}}
                else if(ninks&&p==ink){ok=0;break;}
            }
            if(ok&&ninks)return 1;
        }
    }
    return 0;
}
int main(void)
{
    engine_case(ORIGINAL_SOUND_SYNTHONE,"s1");
    engine_case(ORIGINAL_SOUND_JUNO,"sj6");
    /* Chicago keeps its own arp; no ARP SEQ page, pads stay on the pattern */
    prime(ORIGINAL_SOUND_CHICAGO);EXPECT(page_named("ARP SEQ")<0);
    *page_of(V_SEQ)=3;EXPECT(!arp_seq_page());
    /* UNDO over an arp block: the engine re-reads it (undo_changed skips T_303 voices >= 2) */
    {   const uint32_t base=__builtin_offsetof(project_t,sound)-UNDO_OFF;
        arp_fake_reset();
        undo_changed(base+(T_303*NVOICES_MAX+2)*NPARAMS_MAX+3,8);
        EXPECT(arp_fake.reloads[0]==1&&arp_fake.reloads[1]==0);
        undo_changed(base+(T_303*NVOICES_MAX+3)*NPARAMS_MAX,NPARAMS_MAX);
        EXPECT(arp_fake.reloads[1]==1);
        undo_changed(base+(T_303*NVOICES_MAX+0)*NPARAMS_MAX,4);
        EXPECT(arp_fake.reloads[0]==1&&arp_fake.reloads[1]==1);   /* the bass cell is not the arp */
    }
    if(failures){fprintf(stderr,"FAIL %u/%u\n",failures,expectations);return 1;}
    printf("PASS ui-arp-seq %u checks\n",expectations);return 0;
}
