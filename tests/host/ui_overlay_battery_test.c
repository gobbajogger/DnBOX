/* SPDX-License-Identifier: GPL-3.0-only */
/* Incomplete reachable deep-list inventory through actual SEL entry. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
typedef char row_witness_fits[(LIST_MAX<=64)?1:-1];
int main(void)
{
 unsigned total=0,lists=0;
 for(int view=0;view<NVIEWS;view++)for(int part=0;part<NPARTS;part++) {
  if(view==V_GLO)continue;
  if(view==V_GEN && (part<PART_303A || part>PART_303B))continue;
  int voices=(view==V_PART && part<NKIT)?NDRUM:1;
  for(int voice=0;voice<voices;voice++) {
   setup(view,part);if(part<NKIT)ui.sel[part]=(uint8_t)voice;
   build_pages();button_tap(B_SEL);if(ui.overlay!=O_LIST)continue;
   CHECK(ui.list_n>0 && ui.list_n<=LIST_MAX);lists++;
   for(int back=0;back<LIST_MAX;back++){int before=ui.list_sel;list_move(-1);if(ui.list_sel==before)break;}
   int expected=0,first=-1;uint64_t seen=0;
   for(int r=0;r<ui.list_n;r++)if(!list_skip(r)){expected++;if(first<0)first=r;}
   CHECK(ui.list_sel==first);int captured=0;
   for(int count=0;count<LIST_MAX;count++) {
    int row=ui.list_sel;CHECK(row<ui.list_n && !list_skip(row));
    CHECK(!(seen&(UINT64_C(1)<<row)));seen|=UINT64_C(1)<<row;captured++;
    /* Do not reset blit/tile caches between levels: battery level changes and the frame clock advances 50ms. */
    const int levels[]={3,0,1,2,3,2,1,0};
    for(unsigned k=0;k<sizeof levels/sizeof levels[0];k++) {
     host_battery_level=levels[k];now_ms+=50;ui_frame();
     CHECK(ui.overlay==O_LIST && ui.list_sel==row && ui.view==view && ui.part==part);
     for(int x=219;x<233;x++) {
      CHECK(fb[5*240+x]==swap16(SOUND_GRAY));
      CHECK(fb[12*240+x]==swap16(SOUND_GRAY));
     }
     for(int cell=0;cell<3;cell++)for(int y=7;y<11;y++)for(int x=221+cell*4;x<224+cell*4;x++)
      CHECK(fb[y*240+x]==swap16(cell<levels[k]?SOUND_GRAY:SOUND_LINE));
    }
    total++;list_move(1);if(ui.list_sel==row)break;
   }
   CHECK(captured==expected);
   printf("LIST_COVERAGE view=%d part=%d voice=%d first=%d selectable=%d captured=%d unique=%d\n",view,part,voice,first,expected,captured,captured);
   button_tap(B_HOME);CHECK(ui.overlay==O_NONE);
  }
 }
 printf("OVERLAY_BATTERY %u rows in %u lists, %u injected battery-level updates, %u checks; other domains NOT_RUN\n",total,lists,total*8,checks);
 return 0;
}
