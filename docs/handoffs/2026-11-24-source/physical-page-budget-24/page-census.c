/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_main
#include "smc_ui_test.c"
#if COMPACT_PAGE_BUDGET
#define TITLE(n) page_title(&pg,n)
#define LREF(n) list_ref(n)
#else
#define TITLE(n) pg.title[n]
#define LREF(n) list_rows[n]
#endif
static void render_blocks(unsigned n){for(;n;n--){int32_t a[512];engine_render(a,256);}}
static void drain(unsigned owner){
    for(unsigned n=0;n<1024&&engine_bass_preset_pending(owner);n++)render_blocks(1);
    CHECK(!engine_bass_preset_pending(owner));
}
static unsigned long frame_hash(void){
    unsigned long h=2166136261u;
    for(unsigned i=0;i<240u*240u;i++)h=((h^fb[i])*16777619u)&0xffffffffu;
    return h;
}
static uint8_t census_group(unsigned p)
{
#ifdef DNBX_PAGE24
    return page_group(&pg,p);
#else
    return pg.group[p];
#endif
}
static void refs(pref_t r){printf("%02x%02x%02x%02x",r.kind,r.a,r.b,r.c);}
static void emit_rows(void){
    unsigned n=pg.n;
    for(unsigned p=0;p<n;p++){
        page_query_t q={.page=(int)p};
        describe_pages(NULL,&q,ui.view,ui.part,ui.mixsel,ui.sel);
        CHECK(q.n==pg.n);
        CHECK(!strcmp(q.title,TITLE(p)));
        CHECK(!memcmp(q.r,pg.r[p],sizeof q.r));
        if(ui.view==V_PART && is_303()) {
            *page_of(ui.view)=p;ui.touched=-1;ui.msg_until=0;ui_frame();
            printf("frame %d %u %08lx\n",ui.part,p,frame_hash());
        }
        printf("page %d %u %s %u ",ui.view,p,TITLE(p),census_group(p));
        for(unsigned k=0;k<4;k++)refs(pg.r[p][k]);
        smc_page_set(p);
        printf(" smc ");for(unsigned k=0;k<8;k++)refs(smc_knob_target(k));
        puts("");
    }
    open_list_of_pages();
    printf("list %u ",ui.list_n);for(unsigned n=0;n<ui.list_n;n++)refs(LREF(n));puts("");
    ui.overlay=O_NONE;
}
int main(void){
    fprintf(stderr,"page-bytes=%zu list-bytes=%zu\n",sizeof pg,sizeof list_rows);
#ifndef PAGE_OVERFLOW_FIXTURE
    const int views[]={V_HOME,V_PART,V_SEQ,V_GEN,V_FX,V_MIX,V_SONG};
    for(unsigned v=0;v<sizeof views/sizeof *views;v++)
        for(int p=0;p<NPARTS;p++){
            /* V_GEN is only entered by an actual bass part. The production
               part picker leaves V_GEN on selecting any other part. */
            if(views[v]==V_GEN && p!=PART_303A && p!=PART_303B) continue;
            setup(views[v],p);
            if(part_is_drum(p))for(int sel=0;sel<NDRUM;sel++){
                ui.sel[p]=sel;build_pages();emit_rows();
            }else emit_rows();
        }
    for(unsigned b=0;b<NBASS;b++)for(unsigned bank=0;bank<4;bank++){
        setup(V_PART,PART_303A+b);
        CHECK(!engine_bass_select_preset(b,bank,0,&proj.sound));drain(b);
        build_pages();emit_rows();
    }
    setup(V_GLO,PART_303A);
    for(unsigned g=0;g<6;g++){
        global_section(g);
        printf("global %u ",g);for(unsigned n=0;n<ui.list_n;n++)refs(LREF(n));puts("");
    }
#else
    setup(V_PART,PART_303A);
    CHECK(!engine_bass_select_preset(0,ORIGINAL_SOUND_SYNTHONE,0,&proj.sound));drain(0);
    build_pages();
    CHECK(pg.n==22);
    CHECK(!strcmp(TITLE(pg.n-1),"SOUND"));
    *page_of(ui.view)=0;
    for(int q=1;q<=16;q++){
        now_ms+=160;enc_acc[EN_SELECT]=1;input();
        CHECK(cur_page()==q);
    }
    CHECK(cur_page()==16);
    CHECK(!strcmp(TITLE(cur_page()),"AMP ENVELOP"));
    page_query_t q={.page=16};
    describe_pages(NULL,&q,ui.view,ui.part,ui.mixsel,ui.sel);
    CHECK(q.n==22 && !strcmp(q.title,TITLE(16)) && !memcmp(q.r,pg.r[16],sizeof q.r));
    CHECK(smc_page_count()==22);
    smc_page_set(16);
    CHECK(smc_page_now()==16);
    char smct[12]; smc_page_name(16,smct);
    CHECK(!strcmp(smct,"AMP ENVELOP"));
    ui_frame();CHECK(frame_hash()!=0);
    open_list_of_pages();CHECK(ui.list_n>64 && ui.list_n<LIST_MAX);
    CHECK(LREF(ui.list_n-1).kind==R_BSTATUS);
    int reached=0;
    for(unsigned i=0;i<ui.list_n;i++)if(i>64 && LREF(i).kind==R_ENG)reached=1;
    CHECK(reached);
    for(unsigned i=0;i<70;i++){now_ms+=160;enc_acc[EN_SELECT]=1;input();}
    CHECK(ui.list_sel>=70 && LREF(ui.list_sel).kind==R_ENG);
    /* Materialized and SMC query modes saturate at the same shared
       reference ceiling, retain ordered MORE, and never expose ghosts. */
    pages_t tmp={0};
    page_query_t query={.page=21};
    page_emitter_t material={&tmp,0,GR_SOUND}, single={0,&query,GR_SOUND};
    for(unsigned i=0;i<3;i++){more(&material,PR(R_ENG,T_909,0,i));more(&single,PR(R_ENG,T_909,0,i));}
    for(unsigned i=0;i<30;i++){
        add_page(&material,PT_SOUND,NONE,NONE,NONE,NONE);
        add_page(&single,PT_SOUND,NONE,NONE,NONE,NONE);
    }
    CHECK(tmp.n==22 && query.n==22 && tmp.nmore==query.nmore && query.nmore==3);
    for(unsigned i=0;i<3;i++)CHECK(page_more(&tmp,i)->c==i);
    printf("PASS synthetic page index 16 and %u list entries, SOUND stays last\n",ui.list_n);
#endif
#ifdef DNBX_PAGE24
    pages_t tmp={0};
    page_query_t query={.page=23};
    page_emitter_t material={&tmp,0,GR_NONE}, single={0,&query,GR_NONE};
    for(unsigned i=0;i<25;i++) {
        material.group=single.group=i%(GR_BT+1u);
        pref_t r=PR(R_ENG,T_909,0,i);
        add_page(&material,i==23?PT_SOUND:PT_SEQUENCE,r,NONE,NONE,NONE);
        add_page(&single,i==23?PT_SOUND:PT_SEQUENCE,r,NONE,NONE,NONE);
    }
    CHECK(sizeof tmp <= 424 && tmp.n==24 && query.n==24);
    CHECK(!strcmp(page_title(&tmp,23),"SOUND") && !strcmp(query.title,"SOUND"));
    CHECK(!memcmp(tmp.r[23],query.r,sizeof query.r) && query.r[0].c==23);
    for(unsigned i=0;i<24;i++)CHECK(page_group(&tmp,i)==i%(GR_BT+1u));
    puts("PASS real 24th emitter slot at unchanged page budget");
#endif
    fprintf(stderr,"checks=%u\n",checks);
    return 0;
}
