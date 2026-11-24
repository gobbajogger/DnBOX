#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
static void select_detent(int dir) { now_ms+=200; enc_acc[EN_SELECT]=dir; input(); ui_frame(); }
static void visit_focus_page(const char *s) {
    for(int n=0;n<pg.n;n++){ if(!strcmp(pg.title[cur_page()],s)) return; select_detent(1); }
    CHECK(0 && "scope page reachable");
}
static void image(unsigned o,const char *token,int k,const char *state) {
    char path[96];
    snprintf(path,sizeof path,"%c-%s-%d-%s",'A'+o,token,k+1,state);
    ui_frame(); shot(path);
}
static void focus_panel(unsigned o,const char *token,const char *title) {
    visit_focus_page(title); now_ms+=1000; image(o,token,-1,"default");
    for(int k=0;k<4;k++) {
        pref_t r=pg.r[cur_page()][k];
        if(r.kind==R_NONE) continue;
        const x0x_param_t *d=pref_desc(r);
        CHECK(d);
        int max = d->max;
        if (r.kind == R_S1PATCH) {
            s1_patch_value v;
            CHECK(s1_read(r,&v));
            max = v.max_position;
        }
        for(int hi=0;hi<2;hi++) {
            for(int n=0;n<160 && matrix_value(r)!=(hi?max:0);n++){
                now_ms+=160;turn(k,hi?1:-1);
            }
            CHECK(matrix_value(r)==(hi?max:0));
            now_ms+=160; turn(k,hi?1:-1); /* touch but remain at the tested edge */
            CHECK(matrix_touch_matches(r,k) && matrix_value(r)==(hi?max:0));
            image(o,token,k,hi?"high":"low");
            printf("%c %s %d %s %d page=%d/%d\n",'A'+o,token,k+1,hi?"high":"low",matrix_value(r),cur_page()+1,pg.n);
            now_ms+=1000; image(o,token,k,hi?"high-expired":"low-expired");
            CHECK(ui.touched<0);
        }
    }
    now_ms+=160;turn(0,1);
    CHECK(ui.touched==0);
    select_detent(1);image(o,token,0,"category");
    now_ms+=1000;image(o,token,0,"category-expired");
}
int main(void){
    for(unsigned o=0;o<2;o++){
        for(unsigned sj=0;sj<2;sj++){
            int b=sj?ORIGINAL_SOUND_JUNO:ORIGINAL_SOUND_SYNTHONE;
            setup(V_PART,PART_303A+o); proj.set.bt|=PROJ_BT_CLK_IN;
            CHECK(!engine_bass_select_preset(o,b,0,&proj.sound));
            for(int n=0;n<1024&&engine_bass_preset_pending(o);n++)render(1);
            CHECK(engine_bass_bank(o)==b);build_pages();ui_frame();
            if(sj) focus_panel(o,"SJ6-LFO-RATE","LFO RATE");
            else {focus_panel(o,"S1-EFFECT","EFFECT"); focus_panel(o,"S1-OUTPUT","OUTPUT");}
        }
    }
    return 0;
}
