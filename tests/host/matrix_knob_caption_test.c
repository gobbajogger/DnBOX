/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual renderer pixels; no replacement caption formatter. */
#define CHICAGO_PRESENTATION_NO_MAIN
#include "chicago_presentation_test.c"
#include "ui_sound_footer_oracle.h"
static const char *out;
/* Owner removed all physical-button hint captions; bindings stay tested below. */
static const char *const physical[16]={"","","","","","","","","","","","","","","",""};
static const char *const logical[16]={"","","","","","","","","","","","","","","",""};
static void check_caption(const char *caption,const char *name)
{
    int page=cur_page();
    /* The native Chicago envelope graph replaces the active row widgets and
     * caption; its saved bindings are still checked independently below. */
    if(engine_bass_bank(bidx())==ORIGINAL_SOUND_CHICAGO && page==3)caption="";
    /* Independent displayed titles; never derive expected text from renderer. */
    static const char *const chi_titles[]={"OSCILLATOR","FILTER 1","FILTER 2","ENVELOPE","AMPLIFIER","ARPEGGIATOR","EFFECT","SOUND"};
    static const char *const mia_titles[]={"CARRIER","XMOD","FILTER","WOBBLE","OUTPUT","SOUND"};
    int bank=engine_bass_bank(bidx());
    CHECK(bank==ORIGINAL_SOUND_CHICAGO || bank==ORIGINAL_SOUND_MIAMI);
    CHECK(page>=0 && page<(bank==ORIGINAL_SOUND_CHICAGO?8:6));
    /* Miami pages 5 and 6 occupy rows 3 and 4 of the final four-row window. */
    int tail = bank == ORIGINAL_SOUND_MIAMI && page >= 4;
    int r = tail ? page - 2 : page % 4, top = 24 + r * 50;
    const char *expected_title=bank==ORIGINAL_SOUND_CHICAGO?chi_titles[page]:mia_titles[page];
    CHECK(!strcmp_s(matrix_group(page),expected_title));
    char title[80];put_s(put_s(put_i(title,(tail ? page : r)+1),"  "),expected_title);
    
    CHECK(8+tw(&FONT_T,title)+8<=232-tw(&FONT_T,caption));
    sound_t before=proj.sound;int dirty=ui.dirty,sw=save_wait;
    pref_t refs[4];memcpy(refs,pg.r[page],sizeof refs);
    shot(out,name);
    CHECK(!memcmp(&before,&proj.sound,sizeof before));CHECK(!memcmp(refs,pg.r[page],sizeof refs));
    CHECK(cur_page()==page && ui.dirty==dirty && save_wait==sw);
    uint16_t label=SOUND_INK;
    /* Independent exact flat surface geometry, not the production wrapper.
     * The obsolete raised bevel/glare is no longer an accepted oracle. */
    cv_begin(240,49,SOUND_BASE);
    rbox(3,0,234,49,8,SOUND_SELECT_EDGE);
    rbox(4,1,232,47,7,SOUND_SELECT);
    cv_text(8,0,&FONT_T,title,label);text_r(232,0,&FONT_T,caption,label);
    /* Chicago ENVELOPE row: the ADSR graph draws its K5-K8 A/D/S/R values at
     * rest (#746); ui-glyph-polish checks those pixels. */
    if(!(engine_bass_bank(bidx())==ORIGINAL_SOUND_CHICAGO && page==3))
    for(int y=0;y<11;y++)for(int x=8;x<232;x++)if(fb[(top+y)*240+x]!=cv_px[y*240+x]) {
        fprintf(stderr,"caption %s mismatch page%d top%d x%d y%d got%04x want%04x\n",name,page,top,x,y,fb[(top+y)*240+x],cv_px[y*240+x]);CHECK(0);
    }
    expect_sound_step_footer();
}
/* Owner-declared native page order; changing a binding must fail before a
 * caption derived from that binding can accidentally agree with itself. */
static void check_bindings(int bank,int voice)
{
    static const int chi[][4]={{0,1,-1,-1},{2,3,4,-1},{5,6,-1,-1},{7,8,9,10},
        {11,12,-1,-1},{13,15,14,16},{17,18,19,-1}};
    static const int mia[][4]={{0,1,2,3},{6,5,7,8},{9,10,12,-1},{11,13,-1,-1},{4,15,14,-1}};
    const int (*want)[4]=bank?mia:chi;int count=bank?5:7;
    CHECK(pg.n==count+1);
    for(int p=0;p<count;p++)for(int k=0;k<4;k++) {
        pref_t r=pg.r[p][k];
        if(want[p][k]<0)CHECK(r.kind==R_NONE);
        else CHECK(r.kind==R_ENG && r.a==T_303 && r.b==voice && r.c==want[p][k]);
    }
    CHECK(pg.r[count][0].kind==R_BENGINE && pg.r[count][1].kind==R_BPRESET);
    CHECK(pg.r[count][2].kind==R_BSTATUS && pg.r[count][3].kind==R_NONE);
}
static void check_wrap(void)
{
    for(int v=0;v<2;v++)for(int bank=0;bank<2;bank++) {
        fixture(v,bank);int n=pg.n;check_bindings(bank,v);
        *page_of(V_PART)=0;
        for(int d=-1;d<=1;d+=2)for(int i=0;i<n*2;i++) {
            int previous=cur_page(),want=(previous+d+n)%n;
            sound_t before=proj.sound;int dirty=ui.dirty,sw=save_wait;
            now_ms++;enc_acc[EN_SELECT]=d;ui_input_only();
            CHECK(cur_page()==want);CHECK(cur_page()/4==want/4);
            CHECK(!memcmp(&before,&proj.sound,sizeof before));CHECK(ui.dirty==dirty && save_wait==sw);
            if(previous/4!=want/4) {
                unsigned mask=0;for(int k=0;k<4;k++)if(pg.r[want][k].kind!=R_NONE)mask|=1u<<k;
                char name[64];snprintf(name,sizeof name,"%s-%c-wrap-%d-%d",bank?"miami":"chicago",'a'+v,d,want+1);
                check_caption((X0X_SMK_CAPTIONS?physical:logical)[mask],name);
            }
        }
        /* Other views keep their previous clamp behavior. */
        set_view(V_SEQ);*page_of(V_SEQ)=0;now_ms++;enc_acc[EN_SELECT]=-1;ui_input_only();CHECK(cur_page()==0);
        *page_of(V_SEQ)=pg.n-1;now_ms++;enc_acc[EN_SELECT]=1;ui_input_only();CHECK(cur_page()==pg.n-1);
    }
}
int main(int argc,char **argv)
{
    CHECK(argc==2);out=argv[1];boot();run_ms(50);engine_guard_enable(0);
    check_wrap();if(getenv("X0X_CAPTION_WRAP_ONLY")){printf("PASS %u row wrap checks\n",checks);return 0;}
    const char *const *names=X0X_SMK_CAPTIONS?physical:logical;
    for(int v=0;v<2;v++)for(int bank=0;bank<2;bank++) {
        fixture(v,bank);
        CHECK(8+tw(&FONT_T,"3  SOUND")+8<=232-tw(&FONT_T,names[3]));
        for(int p=0;p<pg.n;p++) {
            *page_of(V_PART)=p;unsigned mask=0;char name[64];
            for(int k=0;k<4;k++)if(pg.r[p][k].kind!=R_NONE)mask|=1u<<k;
            snprintf(name,sizeof name,"%s-%c-row-%d",bank?"miami":"chicago",'a'+v,p+1);
            check_caption(names[mask],name);
        }
        /* Requested-engine name and pending actions are rendered, not inferred. */
        *page_of(V_PART)=row(R_BENGINE,0);request(v,bank^1);engine_bass_wheel_owners(1u<<v);
        check_caption(names[7],"pending-sound");engine_bass_cancel_preset(v);engine_bass_wheel_owners(0);
    }
    /* Exhaust every possible hole pattern without compacting the four slots. */
    fixture(0,1);*page_of(V_PART)=0;pref_t refs[4];memcpy(refs,pg.r[0],sizeof refs);
    for(int mask=0;mask<16;mask++) {
        for(int k=0;k<4;k++)pg.r[0][k]=(mask&(1<<k))?refs[k]:NONE;
        char name[32];snprintf(name,sizeof name,"mask-%02d",mask);check_caption(names[mask],name);
    }
    puts("Caption and footer tests use exact production framebuffer pixels.");
    printf("PASS %u mapped-knob caption checks; SMK captions=%d\n",checks,X0X_SMK_CAPTIONS);return 0;
}
