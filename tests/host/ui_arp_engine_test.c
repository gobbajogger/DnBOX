/* SPDX-License-Identifier: GPL-3.0-only */
/* ARP SEQ / ARPEGGIATOR pages against Synths' real engine hooks (engine_arp.h,
 * X0X_ARP_HOOKS=1): pad and knob input through the UI, edits applied at the
 * audio boundary, values read back from the engine, a held key really arps. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static unsigned failures, expectations;
#define EXPECT(x) do{expectations++;if(!(x)){failures++;fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);}}while(0)
static void render(int n){for(int i=0;i<n;i++){int32_t a[512];engine_render(a,256);}}
static void pad_down(int w){held_keys=1u<<WHITE_KEY[w];++now_ms;input();}
static void pad_up(void){held_keys=0;now_ms+=5;input();render(4);ui_frame();}
static void tap_pad(int w){pad_down(w);pad_up();}
static void turn(int k,int e){enc_acc[EN_K1+k]=e;++now_ms;input();render(4);ui_frame();}
static void tap_panel(int b){held_btn=1u<<b;++now_ms;input();held_btn=0;++now_ms;input();render(4);}
static int page_named(const char *t){for(int i=0;i<pg.n;i++)if(!strcmp(pg.title[i],t))return i;return -1;}
static void engine_case(int kind,const char *tag)
{
    engine_arp_settings s; engine_arp_steps st; char name[64];
    setup(V_SEQ,PART_303A);
    EXPECT(!engine_bass_select_preset(0,kind,0,&proj.sound));
    for(int n=0;n<1024&&engine_bass_preset_pending(0);n++)render(1);
    EXPECT(engine_bass_bank(0)==kind);render(8);
    ui.held_step=-1;build_pages();
    EXPECT(page_named("ARP SEQ")==3);
    *page_of(V_SEQ)=3;build_pages();ui_frame();EXPECT(arp_seq_page());
    EXPECT(engine_arp_steps_read(0,&st));unsigned on0=st.on_mask;
    tap_pad(4);EXPECT(engine_arp_steps_read(0,&st)&&((st.on_mask^on0)&1u<<4));
    pad_down(5);turn(0,3);
    EXPECT(engine_arp_steps_read(0,&st)&&st.offset[5]==3);
    tap_panel(B_OCTUP);
    EXPECT(engine_arp_steps_read(0,&st)&&st.offset[5]==(kind==ORIGINAL_SOUND_JUNO?15:12));   /* engine clamps S1 at +12 */
    turn(2,1);EXPECT(engine_arp_steps_read(0,&st)&&(st.oct_mask&1u<<5));
    pad_up();
    for(int n=0;n<4;n++){turn(0,-1);}
    EXPECT(engine_arp_settings_read(0,&s)&&s.length==12);
    set_view(V_PART);build_pages();int a=page_named("ARPEGGIATOR");EXPECT(a>=0);
    *page_of(V_PART)=(uint8_t)(a<0?0:a);build_pages();ui_frame();
    turn(0,1);for(int n=0;n<3;n++){turn(2,1);}
    EXPECT(engine_arp_settings_read(0,&s)&&s.on==1&&s.mode==ARP_SEQ);
    snprintf(name,sizeof name,"real-arpeggiator-%s",tag);shot(name);
    tap_panel(B_SEQ);*page_of(V_SEQ)=3;build_pages();ui_frame();
    EXPECT(ui.touched<0);                                    /* no K7 readout carried onto ARP SEQ's empty K7 */
    seq.playing=1;engine_bass_on(0,48,0,0);
    unsigned step=99,first=99,moved=0;uint32_t age=0;
    for(int n=0;n<400;n++){render(4);if(engine_arp_view(0,&step,&age)){if(first==99)first=step;else if(step!=first)moved=1;}}
    EXPECT(first!=99);EXPECT(moved);
    ui_frame();snprintf(name,sizeof name,"real-arp-seq-%s-running",tag);shot(name);
    engine_bass_off(0);seq.playing=0;render(64);
}
int main(void)
{
    engine_case(ORIGINAL_SOUND_SYNTHONE,"s1");
    engine_case(ORIGINAL_SOUND_JUNO,"sj6");
    if(failures){fprintf(stderr,"FAIL %u/%u\n",failures,expectations);return 1;}
    printf("PASS ui-arp-engine %u checks\n",expectations);return 0;
}
