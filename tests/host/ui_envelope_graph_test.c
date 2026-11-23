/* SPDX-License-Identifier: GPL-3.0-only */
/* Every envelope row in every engine draws the full-row envelope graph
 * (draw_adsr_inline), and the graph follows its values
 * and the sounding voice. Production pages and renderer, no device I/O.
 * An envelope row is a sound page whose controls are all envelope stages
 * (Attack/Decay/Sustain/Release by name). FAILs:
 *   G1 the row is drawn as cell glyphs instead of the graph;
 *   G2 turning one stage from minimum to maximum leaves the screen unchanged;
 *   G3 (synths) a note leaves the graph unchanged, or attack, hold and
 *      release all draw the same playhead/level. */
#define SMC_UI_MAIN retained_smc_main
#include "smc_ui_test.c"

static unsigned failures, rows, graphs;
static void audio(unsigned n){int32_t a[512];for(unsigned i=0;i<n;i++)engine_render(a,256);}
/* Synth rows: only that row's panel (draw_part_sound geometry), so the header,
 * LEDs or other rows cannot satisfy a check. Drum pages: the whole screen,
 * because the drum layout draws the amplitude graph in its own panel. */
static int row_y0=0,row_y1=240;
static void row_rect(int p,int synth)
{
    row_y0=0;row_y1=240;
    if(!synth)return;
    int three=matrix_three_rows(),rows=three?3:4,height=three?66:49,step=three?67:50;
    int first=p/4*4;if(pg.n>=rows&&first+rows>pg.n)first=pg.n-rows;
    row_y0=HDR_H+4+(p-first)*step;row_y1=row_y0+height;
}
static uint32_t screen_hash(void)
{
    memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);
    ui.msg_until=0;ui.touch_until=0;ui.touched=-1;ui_frame();
    uint32_t h=2166136261u;
    for(int y=row_y0;y<row_y1;y++)for(int x=0;x<240;x++)h=(h^fb[y*240+x])*16777619u;
    return h;
}
static int stage_name(const char *n)
{
    static const char *const words[]={"ATTACK","DECAY","SUSTAIN","RELEASE"};
    size_t len=strlen(n);
    for(unsigned w=0;w<4;w++){
        size_t l=strlen(words[w]);
        if(len<l)continue;
        int same=1;
        for(size_t i=0;i<l;i++){char c=n[len-l+i];if(c>='a'&&c<='z')c=(char)(c-32);same&=c==words[w][i];}
        if(same && (len==l || n[len-l-1]==' '))return 1;
    }
    return 0;
}
static int envelope_row(int p)
{
    int n=0;
    for(int k=0;k<4;k++){
        pref_t r=pg.r[p][k];
        if(r.kind==R_NONE)continue;
        if(!stage_name(pref_name(r)))return 0;
        n++;
    }
    return n>0;
}
static int graph_row(int p) { return adsr_row(p); }
static void settle(void){for(int i=0;i<1024&&engine_bass_preset_pending(0);i++)audio(1);audio(12);}
static void check_page(const char *owner,int p,int synth)
{
    *page_of(V_PART)=p;build_pages();
    if(!envelope_row(p))return;
    rows++;
    if(!graph_row(p)){fprintf(stderr,"FAIL G1 %s page %d (%s): envelope row without the graph\n",owner,p,pg.title[p]);failures++;return;}
    graphs++;
    row_rect(p,synth);
    for(int k=0;k<4;k++){
        pref_t r=pg.r[p][k];
        if(r.kind==R_NONE)continue;
        const x0x_param_t *d=pref_desc(r);
        int keep=pref_get(r),max=d&&d->max?d->max:127;
        pref_set(r,0);audio(12);uint32_t lo=screen_hash();
        pref_set(r,max);audio(12);uint32_t hi=screen_hash();
        pref_set(r,keep);audio(12);
        if(lo==hi){fprintf(stderr,"FAIL G2 %s page %d %s: graph static across its range\n",owner,p,pref_name(r));failures++;}
    }
    if(synth){
        uint32_t idle=screen_hash();
        engine_bass_on_velocity(0,48,0,0,100);audio(2);uint32_t a=screen_hash();
        audio(40);uint32_t b=screen_hash();
        engine_bass_off(0);audio(8);uint32_t c=screen_hash();
        audio(400);
        if(a==idle||(a==b&&b==c)){fprintf(stderr,"FAIL G3 %s page %d: graph does not follow the voice (%08lx %08lx %08lx %08lx)\n",
            owner,p,(unsigned long)idle,(unsigned long)a,(unsigned long)b,(unsigned long)c);failures++;}
    }
}
int main(void)
{
    static const char *const engines[4]={"chicago","miami","synthone","superj6"};
    for(unsigned e=0;e<4;e++){
        setup(V_PART,PART_303A);
        CHECK(engine_bass_select_preset(0,(int)e,0,&proj.sound)==0);
        settle();
        CHECK(engine_bass_sound_bank(&proj.sound,bidx())==(int)e);
        build_pages();
        int n=pg.n;
        for(int p=0;p<n;p++)check_page(engines[e],p,1);
    }
    for(int kit=PART_909;kit<=PART_808;kit++)for(int v=0;v<NDRUM;v++){
        setup(V_PART,kit);ui.sel[kit]=(uint8_t)v;build_pages();
        char owner[24];snprintf(owner,sizeof owner,"%s voice %d",kit==PART_909?"909":"808",v);
        int n=pg.n;
        for(int p=0;p<n;p++)check_page(owner,p,0);
    }
    printf("Envelope graph: %u envelope rows, %u drawn as the graph, %u failures\n",rows,graphs,failures);
    CHECK(rows>=6);
    return failures?1:0;
}
