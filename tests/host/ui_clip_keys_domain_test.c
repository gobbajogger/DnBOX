/* SPDX-License-Identifier: GPL-3.0-only */
#include "actual-clip-key-host.inc"
#include <assert.h>
int main(void) {
 unsigned states=0, expected=0; palette_set(1);
 proj.set.ck_ch=2; proj.set.ck_gate=4; proj.set.ck_rate=8;
 ck_t base_ck=ck; settings_t base_settings=proj.set;
 /* Positive controls: instrumentation must detect clipping, overlap, and a cell overrun. */
 cv_begin(240,138,C_BLACK); cv_band(0,138);
 audit_text(-40,0,&FONT_T,"ROOT",C_WHITE);
 audit_text(-40,0,&FONT_T,"ROOT",C_WHITE);
 audit_call=0; audit_text(115,0,&FONT_T,"ROOT",C_WHITE);
 assert(audit_clipped && audit_overlap && audit_overrun);
 audit_call=audit_draws=audit_clipped=audit_overlap=audit_overrun=0;
 for (unsigned pg=1; pg<=3; pg++) for (unsigned slot=0; slot<(pg==3?9u:4u); slot++) {
  unsigned field=CK_PAGE_FIELDS[pg-1][slot]; expected+=CK_FMAX[field]+1;
  for (unsigned raw=0; raw<=CK_FMAX[field]; raw++) {
   ck=base_ck; proj.set=base_settings;
   ck.pg=pg|(field<<4); ck_set(field,raw);
   if(field==CK_E_LATCH) assert(!!ck_get(field)==!!raw);
   if(field!=CK_E_LATCH) assert(ck_get(field)==raw);
   ck_t saved=ck; settings_t settings=proj.set;
   audit_pg=pg; audit_field=field; audit_raw=raw; audit_call=0;
   cv_begin(240,138,C_BLACK); cv_band(0,138); ck_page_draw();
   assert(!memcmp(&saved,&ck,sizeof ck)); assert(!memcmp(&settings,&proj.set,sizeof settings));
   assert(audit_call==2*(pg==3?9u:4u));
   states++;
  }
 }
 assert(states==expected && states==153);
 printf("SUMMARY states=%u draws=%u clipped_ink=%u label_value_overlap=%u cell_overrun=%u\n",states,audit_draws,audit_clipped,audit_overlap,audit_overrun);
 return (audit_clipped||audit_overlap||audit_overrun)?1:0;
}
