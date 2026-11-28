int main(void)
{
    unsigned cases=0;
    static uint16_t expected[240*220], joined[240*220];
    boot();
    for(unsigned theme=0;theme<NPALETTES;theme++) for(unsigned v=0;v<NBASS;v++){
        palette_set(theme);ui.view=V_PART;ui.part=v+PART_303A;ui.overlay=O_NONE;ui.help=0;
        describe_pages(&pg,0,ui.view,ui.part,ui.mixsel,ui.sel);
        for(unsigned page=0;page<(unsigned)pg.n;page++) for(int touch=-1;touch<4;touch++){
            ui.page[ui.view][ui.part]=page;
            ui.touched=touch;ui.touch_until=plat_ms()+3000;
            const uint16_t bg=C_BLACK; cv_legacy_render l={0}; if(!cv_capture_legacy_render(&l,v))return 2;
            for(int band=0;band<2;++band){
                cv_window(240,MAIN_ROWS,band*BAND_H,BAND_H,C_BLACK);
                cv_band(band*BAND_H,BAND_H);draw_part_sound(band);
                memcpy(expected+band*BAND_H*240,cv_px,BAND_H*240*2);
            }
            cv_tile_valid[3]=0; /* full old legacy footer, including its y69 cutoff */
            legacy_knob_commit(1);
            memcpy(expected+2*BAND_H*240,fb+KNOB_Y*240,KNOB_H*240*2);
            ui_t before=ui; if(getenv("MUT")) {palette_set((theme+2)%NPALETTES);ui.part=PART_909;ui.view=V_MIX;}
            if(getenv("MUT")) memset(proj.sound.v[T_303][v],0xED,sizeof l.sound);
            for(int band=0;band<3;++band)for(unsigned y=0;y<(band==2?KNOB_H:BAND_H);y+=48){
                const unsigned full=band==2?KNOB_H:BAND_H; unsigned h=full-y<48?full-y:48;
                cv_window(240,(band==2?KNOB_H:MAIN_ROWS),(band==2?0:band*BAND_H)+y,h,bg);
                cv_band(band==2?0:band*BAND_H,band==2?KNOB_H:BAND_H);cv_paint_legacy_matrix(band,&l);
                memcpy(joined+(band*BAND_H+y)*240,cv_px,h*240*2);
            }
            ui=before;palette_set(theme);memcpy(proj.sound.v[T_303][v],l.sound,sizeof l.sound);
            if(memcmp(joined,expected,sizeof joined)){
                for(unsigned n=0;n<240*220;++n)if(joined[n]!=expected[n]){
                    printf("MISMATCH theme=%u part=%u page=%u touch=%d xy=%u,%u got=%04x exp=%04x\n",theme,v,page,touch,n%240,n/240,joined[n],expected[n]);break;}
                return 8;
            }
            ++cases;
        }
    }
    printf("PASS %u full native-renderer MAIN 48+21 and footer 48+34 pixel comparisons with mutation, %zu B record\n",cases,sizeof(cv_legacy_render));return 0;
}
