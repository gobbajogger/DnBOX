/* SPDX-License-Identifier: GPL-3.0-only */
#include "actual-clip-key-host.inc"
#include <assert.h>
int main(void){
 unsigned checked=0;uint16_t previous[240*138];
 palette_set(1);proj.set.ck_ch=2;proj.set.ck_gate=4;proj.set.ck_rate=8;
 for(unsigned pg=1;pg<=3;pg++)for(unsigned field=0;field<(pg==3?9u:4u);field++){
  ck.pg=pg|(CK_PAGE_FIELDS[pg-1][field]<<4);ck_t saved=ck;settings_t settings=proj.set;
  cv_begin(240,138,C_BLACK);cv_band(0,138);ck_page_draw();
  assert(!memcmp(&saved,&ck,sizeof ck));assert(!memcmp(&settings,&proj.set,sizeof settings));
  unsigned ink=0;for(unsigned i=0;i<240*138;i++)ink+=cv_px[i]!=swap16(C_BLACK);assert(ink>0);
  if(field)assert(memcmp(previous,cv_px,sizeof previous));memcpy(previous,cv_px,sizeof previous);
  char name[64];sprintf(name,"page-%u-field-%u.rgb565be",pg,field);FILE*f=fopen(name,"wb");assert(f);
  assert(fwrite(cv_px,2,240*138,f)==240*138);assert(!fclose(f));checked++;
 }
 uint32_t sig=0;ck_t saved=ck;assert(smk_ck_panel(&sig,1));assert(!memcmp(&saved,&ck,sizeof ck));
 assert(cv_w==240 && cv_h==82);FILE*f=fopen("keys-strip.rgb565be","wb");assert(f);assert(fwrite(cv_px,2,240*82,f)==240*82);fclose(f);
 assert(checked==17);printf("PASS actual clip-key graphics: %u selected-field states plus real bottom strip; no device/visual acceptance\n",checked);return 0;
}
