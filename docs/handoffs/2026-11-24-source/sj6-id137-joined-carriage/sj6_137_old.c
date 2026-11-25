#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static project_t check,fresh;
static uint8_t media_hdr[32];
static uint8_t saved[OBJ_NOBJ][PLAT_STORE_MAX], prev[OBJ_NOBJ][PLAT_STORE_MAX];
static int lens[OBJ_NOBJ],prevlens[OBJ_NOBJ];
static void wire_header(uint8_t *dst,project_t *p){
 CHECK(!memcmp(&fresh,p,sizeof fresh));memcpy(dst,media_hdr,32);
}
#include "ui_vnext_old_usb_witness.inc"
static void refuse(void){
 memcpy(saved,store,sizeof saved);memcpy(prev,store_previous,sizeof prev);
 memcpy(lens,store_len,sizeof lens);memcpy(prevlens,store_previous_len,sizeof prevlens);
 uint32_t n=store_writes;
 CHECK(project_load()==-9 && project_storage_unsupported());
 CHECK(project_save()==-9);
 project_device_bt_set(!(proj.set.bt&1));CHECK(project_device_bt_save()==-9);
 CHECK(project_load_stage()==-9);
 CHECK(n==store_writes && !memcmp(saved,store,sizeof saved) && !memcmp(prev,store_previous,sizeof prev));
 CHECK(!memcmp(lens,store_len,sizeof lens)&&!memcmp(prevlens,store_previous_len,sizeof prevlens));
}
int main(void){
 setup(V_PART,PART_303A);ui.overlay=O_NONE;proj.set.bt|=PROJ_BT_CLK_IN;
 FILE *f=fopen("out/media.bin","rb");CHECK(f);
 CHECK(fread(media_hdr,sizeof media_hdr,1,f)==1&&fread(&fresh,sizeof fresh,1,f)==1&&
       fread(store,sizeof store,1,f)==1&&fread(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 old_usb_probe();
 refuse(); /* exact tester-produced media */
 store[OBJ_SET][4+offsetof(settings_t,snd_rev)]=PROJ_SND_REV;refuse();
 puts("PASS unchanged old project+BT and ED_PROJECT refuse exact new writer media; no object modified");
 return 0;
}
