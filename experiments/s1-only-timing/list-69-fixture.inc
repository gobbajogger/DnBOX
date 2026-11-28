static unsigned injected;
static int inject;
static void test_display_interrupt(unsigned band,unsigned first)
{
    if(!inject || band || first)return;
    ++injected;
    ui.view=V_HOME;ui.part=PART_BRK;ui.overlay=O_NONE;
    ui.list_top=137;ui.list_n=0;
    memset(list_rows,0,sizeof list_rows);
    palette_set(5);
    blit_hash[1]=blit_hash[3]=0;
}
int main(void)
{
    unsigned n=0;
    static uint16_t target[240*(MAIN_ROWS+KNOB_ROWS)];
    static pref_t saved[LIST_MAX];
    boot();
    now_ms=1000;
    unsigned bad=0;
    for(unsigned t=0;t<NPALETTES;t++)for(unsigned p=0;p<NPARTS;p++)for(unsigned scroll=0;scroll<9;scroll++){
        palette_set(t);
        ui.view=(p==PART_BRK)?V_GLO:V_PART;
        ui.part=p;ui.overlay=O_LIST;ui.help=0;
        page_query_t q={0};
        describe_pages(&pg,&q,ui.view,p,ui.mixsel,ui.sel);
        open_list_of_pages();
        for(unsigned i=0;i<scroll;i++)list_move(1);
        ui_t before=ui;
        memcpy(saved,list_rows,sizeof saved);
        unsigned rows=(visible_list_rows()==9)?MAIN_ROWS+KNOB_ROWS:MAIN_ROWS;
        inject=0;blit_hash[1]=blit_hash[3]=0;
        draw_list_once();
        memcpy(target,fb+MAIN_Y*240,rows*240u*2u);
        ui=before; memcpy(list_rows,saved,sizeof saved);
        inject=1;blit_hash[1]=blit_hash[3]=0;
        draw_list_once();
        if(memcmp(target,fb+MAIN_Y*240,rows*240u*2u) || blit_hash[1] || blit_hash[3]){
            fprintf(stderr,"LIST mismatch t=%u p=%u scroll=%u\n",t,p,scroll);
            return 1;
        }
        ++n;
    }
    if(injected!=n)return 2;
    printf("PASS %u LIST layouts and injected state changes at 69 rows, %u ordinary backing bytes\n", n, (unsigned)sizeof cv_px);
    return bad!=0;
}
