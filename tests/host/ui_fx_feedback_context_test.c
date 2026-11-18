/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t context[240*240];
static uint32_t project_hash(void) {const uint8_t *p=(const uint8_t*)&proj;uint32_t h=2166136261u;for(unsigned i=0;i<sizeof proj;i++)h=(h^p[i])*16777619u;return h;}
int main(void)
{
    static const int raw[]={0,1,52,126,127};
    static const char *const value[]={"0","1","35","84","85"};
    for(int mode=0;mode<2;mode++)for(unsigned k=0;k<sizeof raw/sizeof raw[0];k++) {
        setup(V_FX,PART_909);*page_of(V_FX)=1;build_pages();
        pref_t r=PR(R_ENG,T_FX,0,FX_DL_FDBK);
        pref_set(PR(R_ENG,T_FX,0,FX_DL_TYPE),mode);pref_set(r,raw[k]);
        char num[16],unit[8],want[20];pref_value(r,num,unit);
        CHECK(!strcmp(num,value[k]));CHECK(!strcmp(unit,"%"));
        put_s(put_s(want,value[k]),"%");
        uint32_t before=project_hash();
        draw_main();memcpy(context,fb,sizeof context);
        CHECK(before==project_hash());
        int width=tw(&FONT_T,want),x0=111-width,y0=141;
        cv_begin(240,138,0);cv_text(0,0,&FONT_T,want,C_WHITE);
        int ink=0;
        for(int y=0;y<FONT_T.h;y++)for(int x=0;x<width;x++)
            if(cv_px[y*240+x]==swap16(C_WHITE)) {
                if(context[(y0+y)*240+x0+x]!=cv_px[y*240+x]) {
                    fprintf(stderr,"feedback-context-mismatch mode=%d raw=%d expected=%s\n",mode,raw[k],want);CHECK(0);
                }
                ink++;
            }
        CHECK(ink>0);
    }
    printf("PASS FX configured feedback context/knob boundaries: %u checks\n",checks);
    return 0;
}
