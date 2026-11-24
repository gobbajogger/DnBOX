/* SPDX-License-Identifier: GPL-3.0-only */
/* Native selected ADSR values: actual bound engine readouts, no device I/O. */
#define SMC_UI_MAIN retained_smc_main
#include "smc_ui_test.c"

/* RGB565 signatures from reviewed native captures, re-pinned for the
 * prefix-free "A/D/S/R value" captions (calm pass: no K-number prefix). The active value
 * band and inactive frames are pinned. Synth One and SuperJ6 rows are the
 * same draw_adsr_inline graph, so both bands sit at the same offset.
 * Stage values and owner selection below are production bindings. */
static const struct { const char *name; uint32_t hash; } expected[] = {
    {"s1-filter-dark-0", 0xb6858f71u},
    {"s1-amp-dark-0", 0x2f0693e3u},
    {"sj6-amp-dark-0", 0x4976872cu},
    {"s1-filter-dark-1", 0xdc671ad9u},
    {"s1-amp-dark-1", 0xbef38583u},
    {"sj6-amp-dark-1", 0x63296615u},
    {"s1-filter-light-0", 0x7979c14bu},
    {"s1-amp-light-0", 0xbcf0d0d2u},
    {"sj6-amp-light-0", 0x42452f32u},
    {"s1-filter-light-1", 0xcc0ce4b1u},
    {"s1-amp-light-1", 0x11bd3d60u},
    {"sj6-amp-light-1", 0xca26bd1bu},
    {"s1-dark-inactive", 0x673b8165u},
    {"s1-light-inactive", 0xed24a140u},
    {"sj6-dark-inactive", 0x79b4944au},
    {"sj6-light-inactive", 0x9833f8acu},
};
static unsigned failures, witnessed;

static void check_pixels(const char *name,int x0,int x1,int y0,int y1)
{
    uint32_t observed=2166136261u;
    for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++)
        observed=(observed^fb[y*240+x])*16777619u;
    for(unsigned i=0;i<sizeof expected/sizeof expected[0];i++) {
        if(strcmp(name,expected[i].name)) continue;
        witnessed++;
        if (observed!=expected[i].hash) {
            fprintf(stderr,"FAIL %s RGB565: %08lx != %08lx\n",name,
                    (unsigned long)observed,(unsigned long)expected[i].hash);
            failures++;
        }
        return;
    }
    CHECK(0);
}

static void audio(unsigned count)
{
    int32_t a[512];
    for(unsigned i=0;i<count;i++) engine_render(a,256);
}
static int owner_page(int kind,int first)
{
    for(int p=0;p<pg.n;p++){
        int ok=1;
        for(int i=0;i<4;i++)
            ok &= pg.r[p][i].kind==kind && pg.r[p][i].c==first+i;
        if(ok) return p;
    }
    return -1;
}
static void prime(unsigned engine, int light)
{
    setup(V_PART,PART_303A);
    CHECK(engine_bass_select_preset(0,engine,0,&proj.sound)==0);
    for(int i=0;i<1024&&engine_bass_preset_pending(0);i++) audio(1);
    CHECK(!engine_bass_preset_pending(0));
    CHECK(engine_bass_sound_bank(&proj.sound,bidx())==(int)engine);
    proj.set.palette=light?5:0;
    palette_set(proj.set.palette);
    build_pages();
}
static void captured(const char *owner,int light,int p,int endpoint,int synth)
{
    char name[96]; int first, rows, step;
    *page_of(V_PART)=p; build_pages(); CHECK(cur_page()==p);
    const int values[4]={40,40,80,100};
    for(int k=0;k<4;k++){
        int wanted=endpoint ? ((k&1)?127:0) : values[k];
        pref_t pstage=pg.r[p][k];
        pref_set(pstage,wanted);
        audio(12); /* consume each queued S1 stage edit before sending the next */
        CHECK(pref_get(pstage)==wanted);
    }
    if(synth){
        s1_patch_snapshot s;
        CHECK(engine_s1_patch_snapshot(0,&s));
    } else {
        ui_adsr_t v; CHECK(adsr_row(p)); adsr_values(&v,p); CHECK(v.enabled==3&&v.present==15);
    }
    ui.msg_until=0; ui.touch_until=0; ui.touched=-1; memset(blit_hash,0,sizeof blit_hash); memset(cv_tile_valid,0,sizeof cv_tile_valid); ui_frame();
    snprintf(name,sizeof name,"%s-%s-%d",owner,light?"light":"dark",endpoint);
    rows=matrix_three_rows()?3:4; step=matrix_three_rows()?67:50;
    first=p/4*4; if(pg.n>=rows && first+rows>pg.n) first=pg.n-rows;
    int y=HDR_H+4+(p-first)*step+9; /* one shared envelope graph; selected run 1 px below the row header */
    check_pixels(name,3,237,y-2,y+13);
}
static void inactive(int engine,int light)
{
    prime(engine,light);
    *page_of(V_PART)=0; build_pages(); CHECK(cur_page()==0);
    memset(blit_hash,0,sizeof blit_hash); memset(cv_tile_valid,0,sizeof cv_tile_valid); ui_frame();
    char name[72]; snprintf(name,sizeof name,"%s-%s-inactive",engine==ORIGINAL_SOUND_SYNTHONE?"s1":"sj6",light?"light":"dark");
    check_pixels(name,0,240,0,240);
}
int main(void)
{
    for(int light=0;light<2;light++){
        for(int t=0;t<2;t++){
            prime(ORIGINAL_SOUND_SYNTHONE,light);
            int filter=owner_page(R_S1PATCH,5); CHECK(filter>=0);
            captured("s1-filter",light,filter,t,1);
            prime(ORIGINAL_SOUND_SYNTHONE,light);
            int amp=owner_page(R_S1PATCH,9); CHECK(amp>=0);
            captured("s1-amp",light,amp,t,1);
            prime(ORIGINAL_SOUND_JUNO,light);
            int juno=owner_page(R_ENG,5); CHECK(juno>=0);
            captured("sj6-amp",light,juno,t,0);
        }
        inactive(ORIGINAL_SOUND_SYNTHONE,light);
        inactive(ORIGINAL_SOUND_JUNO,light);
    }
    CHECK(witnessed==16);
    printf("Selected ADSR RGB565: 12 active value bands, 4 inactive frames, %u mismatches\n",failures);
    return failures?1:0;
}
