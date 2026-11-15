/* Existing actual host/runtime and baseline UI helpers, not a model renderer. */
#define ORIGINAL_UI_MAIN retained_suite_main
#include "original_release_ui_test.c"
#undef ORIGINAL_UI_MAIN
static void ordinary_header_phase_cases(void)
{
    static const int views[]={V_HOME,V_SEQ,V_MIX,V_PART};
    project_t *old=malloc(sizeof proj);CHECK(old);*old=proj;
    seq_t saved=seq;uint8_t steps[sizeof eng_step];memcpy(steps,eng_step,sizeof eng_step);
    seq.song_on=0;seq.chain_a=seq.chain_b=0;
    for(int part=0;part<NPARTS;part++){seq.ppat[part]=NPAT-1;seq.pcue[part]=NPAT-2;}
    for(int theme=0;theme<=5;theme+=5){palette_set(theme);
      for(unsigned v=0;v<sizeof views/sizeof*views;v++)for(int part=0;part<NPARTS;part++){
        screen(views[v],part,0);ui.msg_until=ui.touch_until=0;
        for(unsigned phase=0;phase<=16;phase+=16)for(int step=0;step<2;step++){
          seq.playing=0;eng_step[TRK_DRUM]=step;ui.frame=phase;draw_header();
          uint32_t stopped=picture_pixels(fb,132,0,53,HDR_H);
          ui.frame+=16;now_ms+=17;draw_header();
          CHECK(picture_pixels(fb,132,0,53,HDR_H)==stopped);
          eng_step[TRK_DRUM]=1-step;draw_header();
          CHECK(picture_pixels(fb,132,0,53,HDR_H)==stopped);
          seq.playing=1;eng_step[TRK_DRUM]=step;ui.frame=phase;draw_header();
          uint32_t live=picture_pixels(fb,132,0,53,HDR_H);
          ui.frame+=16;now_ms+=17;draw_header();
          CHECK(picture_pixels(fb,132,0,53,HDR_H)==live);
          eng_step[TRK_DRUM]=1-step;draw_header();
          CHECK(picture_pixels(fb,132,0,53,HDR_H)!=live);
        }
      }
    }
    CHECK(!memcmp(old,&proj,sizeof proj));seq=saved;memcpy(eng_step,steps,sizeof eng_step);free(old);palette_set(proj.set.palette);
}
int main(void){image_dir=0;load_force=0;boot();settle();ordinary_header_phase_cases();printf("PASS H6 actual_header checks=%u\n",checks);return 0;}
