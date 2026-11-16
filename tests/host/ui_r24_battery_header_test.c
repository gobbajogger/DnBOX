/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN unused_r24_battery_smc_main
#include "smc_ui_test.c"
static void battery_pixels(uint16_t *out){draw_header();for(int y=2;y<16;y++)for(int x=217;x<239;x++)out[(y-2)*22+x-217]=fb[y*240+x];}
int main(void){
 uint16_t full[22*14],actual[22*14],unknown[22*14];
 CHECK(!setenv("X0X_HOST_BATTERY_STATE","3",1));setup(V_PART,PART_909);battery_pixels(full);shot("r024-full");
 CHECK(!setenv("X0X_HOST_BATTERY_STATE","4",1));battery_pixels(actual);shot("r024-usb-power");
 CHECK(memcmp(full,actual,sizeof full)!=0); /* USB power is not known full charge. */
 CHECK(!setenv("X0X_HOST_BATTERY_STATE","-1",1));battery_pixels(unknown);shot("r024-unread");
 CHECK(!setenv("X0X_HOST_BATTERY_STATE","0",1));battery_pixels(actual);shot("r024-empty");
 CHECK(memcmp(unknown,actual,sizeof actual)!=0); /* Unread is not measured empty. */
 for(int level=1;level<=3;level++){char state[4],name[32];snprintf(state,sizeof state,"%d",level);CHECK(!setenv("X0X_HOST_BATTERY_STATE",state,1));memcpy(unknown,actual,sizeof actual);battery_pixels(actual);CHECK(memcmp(unknown,actual,sizeof actual)!=0);snprintf(name,sizeof name,"r024-level%d",level);shot(name);}
 CHECK(!setenv("X0X_HOST_BATTERY_STATE","3",1));
 const int views[]={V_HOME,V_PART,V_FX,V_MIX,V_SEQ,V_SONG,V_GLO,V_GEN,V_PERF};
 for(unsigned i=0;i<sizeof views/sizeof views[0];i++){
  setup(views[i],PART_909);battery_pixels(actual);CHECK(!memcmp(full,actual,sizeof full));
  ui.help=1;battery_pixels(actual);CHECK(!memcmp(full,actual,sizeof full));ui.help=0;
  ui.overlay=O_ASK;battery_pixels(actual);CHECK(!memcmp(full,actual,sizeof full));ui.overlay=O_LIST;battery_pixels(actual);CHECK(!memcmp(full,actual,sizeof full));ui.overlay=O_NONE;
  put_s(ui.msg,"CHECK STATUS");ui.msg_until=now_ms+1000;battery_pixels(actual);CHECK(!memcmp(full,actual,sizeof full));
  put_s(ui.msg,"PICK 127");ui.msg_until=now_ms+1000;battery_pixels(actual);CHECK(!memcmp(full,actual,sizeof full));
 }
 CHECK(!unsetenv("X0X_HOST_BATTERY_STATE"));printf("R024 actualbattery states/persistentheader PASS (%uchecks); no unmeasuredpercent claim\n",checks);return 0;
}
