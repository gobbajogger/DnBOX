/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual typed native glyph and AA raster, with isolated path recording. */
#define SMC_UI_MAIN retained_smc_fixture_main
#include "smc_ui_test.c"
static int rising_span(const float *xy,int n,float *span) {
    int found=0;float low=0,high=0;
    for(int i=1;i<n;i++)if(xy[2*i+1]<xy[2*i-1]) {
        float a=xy[2*i-2],b=xy[2*i];
        if(a>b){float t=a;a=b;b=t;}
        if(!found||a<low)low=a;if(!found||b>high)high=b;found=1;
    }
    *span=found?high-low:0;return found;
}
int main(void) {
    const int ids[][3]={{0,0,1},{0,2,3},{0,3,3},{0,4,3},{1,0,4},{1,6,3}};
    const float scales[][2]={{1.375f,1.5f},{1.8f,1.8f},{.62f,.76f},{1.3f,1.5f}};
    unsigned states=0,failures=0;
    float flat[]={0,0,16,0,32,0},vertical[]={1,12,1,0,31,0},span=0;
    CHECK(!rising_span(flat,3,&span));CHECK(rising_span(vertical,3,&span)&&span==0);
    for(unsigned c=0;c<6;c++) {
        setup(V_PART,ids[c][0]?PART_808:PART_909);ui.sel[ui.part]=(uint8_t)ids[c][1];build_pages();
        pref_t r=PR(R_ENG,ids[c][0],ids[c][1],ids[c][2]);const x0x_param_t*d=pref_desc(r);
        CHECK(d && !strcmp(d->name,"Attack") && d->max==127 && drum_strip_role(r)==DG_ATTACK);
        unsigned visible=0;for(int p=0;p<pg.n;p++)for(int k=0;k<4;k++)if(pg.r[p][k].kind==r.kind&&pg.r[p][k].a==r.a&&pg.r[p][k].b==r.b&&pg.r[p][k].c==r.c)visible++;
        CHECK(visible==1);int original=pref_get(r);uint32_t age=0;float env=0;
        CHECK(!drum_live(r,&age,&env) && age==0xffffffffu && env==0);
        for(unsigned scale=0;scale<4;scale++)for(unsigned endpoint=0;endpoint<2;endpoint++) {
            int raw=endpoint?d->max:0;float sx=scales[scale][0],sy=scales[scale][1];
            cv_begin(64,48,C_BLACK);cv_band(0,48);fwui_attack_calls=fwui_attack_count=0;fwui_attack_recording=1;
            dv_glyph(r,(float)raw,4,4,sx,sy,C_WHITE);fwui_attack_recording=0;
            CHECK(fwui_attack_calls==1 && fwui_attack_count>=2 && fwui_attack_count<=40);
            CHECK(pref_get(r)==original);
            int edge=rising_span(fwui_attack_xy,fwui_attack_count,&span);float ratio=span/(32.f*sx);
            int pass=edge && (endpoint ? ratio>=.4f : ratio<=.1f);failures+=!pass;states++;
            unsigned ink=0;for(unsigned i=0;i<64*48;i++)ink+=cv_px[i]!=swap16(C_BLACK);CHECK(ink>0);
            char name[80];snprintf(name,sizeof name,"attack-%u-scale-%u-end-%u.rgb565be",c,scale,endpoint);
            char path[512];const char*dir=getenv("SMC_UI_SHOTS");CHECK(dir);snprintf(path,sizeof path,"%s/%s",dir,name);
            FILE*f=fopen(path,"wb");CHECK(f);CHECK(fwrite(cv_px,2,64*48,f)==64*48);CHECK(!fclose(f));
            printf("ATTACK_RESULT {\"target\":%d,\"voice\":%d,\"slot\":%d,\"raw\":%d,\"maximum\":%u,\"sx\":%.9g,\"sy\":%.9g,\"viewboxWidth\":32,\"risingSpanRatio\":%.9g,\"risingEdge\":%s,\"pass\":%s,\"ink\":%u,\"raster\":\"%s\"}\n",r.a,r.b,r.c,raw,(unsigned)d->max,(double)sx,(double)sy,(double)ratio,edge?"true":"false",pass?"true":"false",ink,name);
        }
    }
    CHECK(states==48);printf("ATTACK_SUMMARY states=%u failures=%u checks=%u\n",states,failures,checks);
    return failures?1:0;
}
