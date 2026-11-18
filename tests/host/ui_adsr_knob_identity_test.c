/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t rendered[240*82];
static void audio_ms(unsigned ms){int32_t a[512];for(unsigned n=0;n<ms;n++)engine_render(a,44);}
static void caption(int slot,int stage,int pct)
{
 char s[24];snprintf(s,sizeof s,"K%d %c %d%%",UI_FIRST_KNOB+slot,"ADSR"[stage],pct);
 int width=tw(&FONT_T,s),left=31+slot*59-width/2;CHECK(left>=slot*59&&left+width<=(slot+1)*59+4);
 cv_begin(240,82,C_BLACK);cv_text(0,0,&FONT_T,s,SOUND_GRAY);int ink=0;
 for(int y=0;y<FONT_T.h;y++)for(int x=0;x<width;x++)if(cv_px[y*240+x]==swap16(SOUND_GRAY)){
  if(rendered[(12+y)*240+left+x]!=cv_px[y*240+x]){fprintf(stderr,"ADSR_PHYSICAL_KNOB_CAPTION_MISSING slot=%d stage=%d\n",slot,stage);CHECK(0);}ink++;
 }CHECK(ink>0);
}
int main(void)
{
 setup(V_PART,PART_303A);CHECK(!engine_bass_select_preset(0,ORIGINAL_SOUND_CHICAGO,0,&proj.sound));
 for(int n=0;n<1024&&engine_bass_preset_pending(0);n++)audio_ms(6);
 CHECK(!engine_bass_preset_pending(0));build_pages();int page=-1;
 for(int p=0;p<pg.n;p++)if(adsr_stage_slots(p,0)==15)page=p;
 CHECK(page>=0);*page_of(V_PART)=(uint8_t)page;build_pages();
 pref_t stages[4];memcpy(stages,pg.r[page],sizeof stages);
 ui.touched=-1;
 const int raw[]={0,64,127},pct[]={0,50,100};
 for(int q=0;q<3;q++){for(int k=0;k<4;k++)pref_set(stages[k],raw[q]);audio_ms(12);
 for(int shift=0;shift<4;shift++)for(unsigned mask=1;mask<16;mask++){
  for(int k=0;k<4;k++)pg.r[page][k]=NONE;
  for(int stage=0;stage<4;stage++)if(mask&(1u<<stage))pg.r[page][(stage+shift)%4]=stages[stage];
  cv_begin(240,82,C_BLACK);draw_adsr_inline(page,1,4,69,0);memcpy(rendered,cv_px,sizeof rendered);
  for(int stage=0;stage<4;stage++)if(mask&(1u<<stage))caption((stage+shift)%4,stage,pct[q]);
 }
 }
 puts("PASS ADSR captions retain actual physical knob identity for all partial placements");return 0;
}
