/* Independent read-only regression fixture. Production source is unchanged. */
#define SMC_UI_MAIN unused_independent_r024_main
#include "/tmp/dnbox-ui-r024-37c3f303/tests/host/smc_ui_test.c"
static void battery_region(uint16_t *out){for(int y=2;y<16;y++)for(int x=217;x<239;x++)out[(y-2)*22+x-217]=fb[y*240+x];}
int main(void){
 const char *messages[]={"NEWER DATA: NOT SAVED","MEMORY FULL: NOT SAVED","MOTION FULL: NOT ALL PASTED","PROJECT RESET: SAMPLES KEPT","BT SAVE FAILED: RETRYING","WAITING FOR CLOCK"};
 setup(V_PART,PART_303A);
 for(unsigned i=0;i<sizeof messages/sizeof messages[0];i++)printf("MESSAGE_WIDTH T=%d XS=%d B=%d text=%s\n",tw(&FONT_T,messages[i]),tw(&FONT_XS,messages[i]),tw(&FONT_B,messages[i]),messages[i]);
 uint16_t empty[22*14],full[22*14];
 CHECK(!setenv("X0X_HOST_BATTERY_STATE","0",1));
 smc_ui_begin(0,1);CHECK(smc_ui_active());smc_ui_draw();battery_region(empty);shot("independent-smc-empty");
 CHECK(!setenv("X0X_HOST_BATTERY_STATE","3",1));
 smc_ui_draw();battery_region(full);shot("independent-smc-full");
 int changed=memcmp(empty,full,sizeof full)!=0;printf("SMC_BATTERY_REGION_CHANGED=%d\n",changed);fflush(stdout);
 CHECK(changed); /* Every displayed page must retain actual battery state. */
 smc_peek->on=1;smc_peek->view=V_MIX;smc_peek->part=PART_303A;
 CHECK(!setenv("X0X_HOST_BATTERY_STATE","0",1));smc_ui_draw();battery_region(empty);
 CHECK(!setenv("X0X_HOST_BATTERY_STATE","3",1));smc_ui_draw();battery_region(full);CHECK(memcmp(empty,full,sizeof full)!=0);
 CHECK(!unsetenv("X0X_HOST_BATTERY_STATE"));puts("R024 SMC overlay/peek real battery response PASS");
 return 0;
}
