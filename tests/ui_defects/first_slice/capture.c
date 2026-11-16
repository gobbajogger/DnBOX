/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
static void final_label_audit(void) {
    for(unsigned i=0;i<first_nlabels;i++) {
        const first_label_t *e=&first_labels[i];
        cv_begin(240,138,0);first_original_cv_text(0,0,e->font,e->text,0xffff);
        unsigned expected=0,matched=0;int width=tw(e->font,e->text);
        for(int y=0;y<e->font->h;y++)for(int x=0;x<width;x++)if(cv_px[y*240+x]==0xffff) {
            int sx=e->x+x,sy=e->y+y;if(sx<0||sx>=240||sy<0||sy>=240)continue;
            expected++;if(fb[sy*240+sx]==swap16(e->color))matched++;
        }
        printf("FIRST:{\"kind\":\"finaltext\",\"text\":");first_json_string(e->text);
        printf(",\"row\":%d,\"control\":%d,\"origin\":[%d,%d],\"pixels\":%u,\"expectedOpaquePixels\":%u,\"finalFrameMatchRatio\":%.6f,\"bbox\":[%d,%d,%d,%d]}\n",e->row,e->control,e->x,e->y,matched,expected,expected?(double)matched/expected:0,e->x,e->y,e->x+width,e->y+e->font->h);
    }
}
int main(void) {
    unsigned screens=0;
    for(int kit=0;kit<2;kit++)for(int voice=0;voice<NDRUM;voice++) {
        setup(V_PART,kit);ui.sel[kit]=(uint8_t)voice;build_pages();
        for(int step=0;step<16;step+=4)sm_set(cur_pat()->drum[kit].hit[voice],step);
        int pages=pg.n;
        for(int page=0;page<pages;page++) {
            *page_of(V_PART)=(uint8_t)page;build_pages();ui.touched=-1;ui.msg_until=0;
            for(int frame=0;frame<12;frame++){now_ms+=16;ui_frame();}
            char name[96];snprintf(name,sizeof name,"kit%d-voice%02d-page%02d",kit,voice,page);
            printf("FIRST:{\"kind\":\"screen\",\"id\":\"%s\",\"kit\":%d,\"voice\":%d,\"page\":%d}\n",name,kit,voice,page);
            memset(first_mask,0,sizeof first_mask);first_nlabels=0;first_enabled=1;
            draw_main();draw_knobs();first_enabled=0;draw_header();
            printf("FIRST:{\"kind\":\"mask\",\"bits\":\"");
            for(unsigned i=0;i<sizeof first_mask;i++)putchar(first_mask[i]?'1':'0');puts("\"}");
            shot(name);final_label_audit();screens++;
        }
    }
    printf("FIRST:{\"kind\":\"complete\",\"screens\":%u,\"rendererExit\":0}\n",screens);
    return 0;
}
