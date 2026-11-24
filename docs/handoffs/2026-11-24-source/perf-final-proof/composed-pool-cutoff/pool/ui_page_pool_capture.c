#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"

#ifndef SHARED_PAGE_POOL
#define SHARED_PAGE_POOL 0
#endif
static void physical_button(int btn) {
    now_ms+=200; held_btn=1u<<btn; input();
    now_ms+=40; held_btn=0; input(); ui_frame();
}
static void menu_detent(int dir) {
    now_ms+=200; enc_acc[EN_SELECT]=dir; input(); ui_frame();
}
static void audio(void){ int32_t a[512]; engine_render(a,256); }
static void ref_out(pref_t r) { printf(" %02x%02x%02x%02x",r.kind,r.a,r.b,r.c); }
static pref_t more_at(pages_t *p,unsigned i) {
#if SHARED_PAGE_POOL
    return *page_more(p,i);
#else
    return p->more[i];
#endif
}
static int materialize_max, more_max, contexts;
static void catalog(const char *tag){
    ui.overlay=O_NONE; build_pages();
    printf("STATE %s view=%d part=%d pg=%d more=%d\n",tag,ui.view,ui.part,pg.n,pg.nmore);
    if (pg.n>materialize_max) materialize_max=pg.n;
    if (pg.nmore>more_max) more_max=pg.nmore;
    contexts++;
    CHECK(4*pg.n+pg.nmore<=4*MAXPAGES);
    for(int k=0;k<pg.n;k++){
        page_query_t q={.page=k};
        describe_pages(NULL,&q,ui.view,ui.part,ui.mixsel,ui.sel);
        CHECK(q.n==pg.n && !strcmp(q.title,pg.title[k]));
        CHECK(!memcmp(q.r,pg.r[k],sizeof q.r));
        printf("PAGE %d %d [%s]",k,pg.group[k],q.title);
        for(int n=0;n<4;n++) ref_out(q.r[n]);
        printf("\n");
    }
    for(int k=0;k<pg.nmore;k++){
        printf("EXTRA %d",k);ref_out(more_at(&pg,k));printf("\n");
    }
    if(pg.nmore){
        physical_button(B_SEL); CHECK(ui.overlay==O_LIST && ui.list_n>0);
        int selectable=0;
        for(int k=0;k<ui.list_n;k++){
            if(list_rows[k].kind!=R_HDR && list_rows[k].kind!=R_NONE) selectable++;
            printf("MENU %d",k);ref_out(list_rows[k]);printf("\n");
        }
        /* Walk all actually selectable rows by the physical Select encoder.
         * Headers/filtering remain the production list's, not invented by the fixture. */
        int visited=0;
        for(int n=0;n<=ui.list_n;n++) {
            int before=ui.list_sel;
            pref_t r=list_rows[before];
            if(r.kind!=R_HDR && r.kind!=R_NONE){
                printf("PHYSICAL");ref_out(r);printf("\n");visited++;
            }
            menu_detent(1);
            if (ui.list_sel<=before) break;
        }
        CHECK(visited>0 && visited==selectable);
        char title[72];snprintf(title,sizeof title,"more-%s",tag);
        shot(title);
        physical_button(B_HOME); CHECK(ui.overlay==O_NONE);
    }
}
/* Capacity safety at *intermediate* boundaries, in both insertion orders. */
static void guard_probe(void){
#if SHARED_PAGE_POOL
    pages_t p={0};
    page_emitter_t e={&p,NULL,GR_AUDIO};
    pref_t tail=PR(R_ENG,T_MST,0,MST_RATIO), row=PR(R_BENGINE,0,0,0);
    more(&e,tail);
    for(int j=0;j<MAXPAGES;j++) add_page(&e,"test",row,row,row,row);
    CHECK(p.n==MAXPAGES-1 && p.nmore==1 && !memcmp(page_more(&p,0),&tail,sizeof tail));
    for(int i=0;i<MAXPAGES-1;i++) for(int j=0;j<4;j++) CHECK(!memcmp(&p.r[i][j],&row,sizeof row));
    for(int i=0;i<MAXMORE;i++) more(&e,tail);
    CHECK(p.nmore==4 && p.n==14);
    memset(&p,0,sizeof p);
    for(int j=0;j<MAXPAGES;j++) add_page(&e,"test",row,row,row,row);
    more(&e,tail); CHECK(p.n==MAXPAGES && p.nmore==0);
    for(int i=0;i<MAXPAGES;i++) for(int j=0;j<4;j++) CHECK(!memcmp(&p.r[i][j],&row,sizeof row));
#endif
}
int main(void){
    printf("SIZE page materialization=%zu ref=%zu\n",sizeof pg,sizeof(pref_t));
    guard_probe();
    /* Relevant layouts and every drum voice whose list can contain MORE. */
    const int views[]={V_HOME,V_PART,V_GEN,V_FX,V_MIX,V_SONG,V_GLO,V_PERF,V_SEQ};
    for(unsigned v=0;v<sizeof views/sizeof views[0];v++){
        for(unsigned p=0;p<NPARTS;p++){
            unsigned selections = views[v]==V_PART && p<NKIT ? NDRUM : 1;
            for(unsigned s=0;s<selections;s++){
                char tag[32];snprintf(tag,sizeof tag,"view%d-part%u-sel%u",views[v],p,s);
                setup(views[v],p);
                if(p<NKIT) ui.sel[p]=s;
                catalog(tag);
            }
        }
    }
    /* MIX can change target part and both drum tracks independently. */
    for(unsigned target=0;target<NPARTS;target++) for(unsigned s=0;s<NDRUM;s++){
        char tag[32];snprintf(tag,sizeof tag,"mix%u-sel%u",target,s);
        setup(V_MIX,PART_909);ui.mixsel=target;ui.sel[0]=s;ui.sel[1]=NDRUM-1-s;
        catalog(tag);
    }
#if X0X_SUPERJ6_BUILTIN_INIT || X0X_SYNTHONE_PATCH
    for(unsigned owner=0;owner<2;owner++){
#if X0X_SUPERJ6_BUILTIN_INIT
        setup(V_PART,PART_303A+owner);
        CHECK(!engine_bass_select_preset(owner,ORIGINAL_SOUND_JUNO,0,&proj.sound));
        for(int n=0;n<1024&&engine_bass_preset_pending(owner);n++)audio();
        CHECK(engine_bass_bank(owner)==ORIGINAL_SOUND_JUNO);
        catalog(owner?"sj6-b":"sj6-a");
#if X0X_ARP_HOOKS
        CHECK(pg.n==15 && !strcmp(pg.title[14],"SOUND"));
#endif
#endif
#if X0X_SYNTHONE_PATCH
        setup(V_PART,PART_303A+owner);
        CHECK(!engine_bass_select_preset(owner,ORIGINAL_SOUND_SYNTHONE,0,&proj.sound));
        for(int n=0;n<1024&&engine_bass_preset_pending(owner);n++)audio();
        CHECK(engine_bass_bank(owner)==ORIGINAL_SOUND_SYNTHONE);
        catalog(owner?"s1-b":"s1-a");
#endif
    }
#endif
    printf("PASS %d catalog contexts, max pages=%d more=%d\n",contexts,materialize_max,more_max);
    return 0;
}
