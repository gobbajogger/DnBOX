/* SPDX-License-Identifier: GPL-3.0-only */
/* Incomplete reachable deep-list inventory through actual SEL entry. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
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
    memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
    char name[96];snprintf(name,sizeof name,"list-view-%02d-part-%02d-voice-%02d-row-%02d",view,part,voice,row);shot(name);
    printf("SCREEN {\"id\":\"%s\",\"view\":%d,\"part\":%d,\"voice\":%d,\"row\":%d,\"rows\":%d,\"width\":240,\"height\":240}\n",name,view,part,voice,row,ui.list_n);
    total++;list_move(1);if(ui.list_sel==row)break;
   }
   CHECK(captured==expected);
   printf("LIST_COVERAGE view=%d part=%d voice=%d first=%d selectable=%d captured=%d unique=%d\n",view,part,voice,first,expected,captured,captured);
   button_tap(B_HOME);CHECK(ui.overlay==O_NONE);
  }
 }
 printf("OVERLAY_INVENTORY %u states in %u lists; help/confirmation/transients/alternate engine coverage NOT_RUN\n",total,lists);
 return 0;
}
