#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static project_t imported, live;
static uint8_t hdr[32], stored[OBJ_NOBJ][PLAT_STORE_MAX];
static int lengths[OBJ_NOBJ];
int main(void)
{
 setup(V_PART,PART_303A);ui.overlay=O_NONE;proj.set.bt|=PROJ_BT_CLK_IN;
 FILE*f=fopen("out/mixed-media.bin","rb"); CHECK(f);
 CHECK(fread(hdr,sizeof hdr,1,f)==1 && fread(&imported,sizeof imported,1,f)==1 &&
  fread(store,sizeof store,1,f)==1&&fread(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 CHECK(read137(&imported.sound,0)[20]==3 || read137(&imported.sound,1)[20]==3);
 CHECK(read137(&imported.sound,0)[16]==3 || read137(&imported.sound,1)[16]==3);
 CHECK(read137(&imported.sound,0)[21]!=0 || read137(&imported.sound,1)[21]!=0);
 CHECK(read137(&imported.sound,0)[8]==0x80 || read137(&imported.sound,1)[8]==0x80);
 CHECK(read137(&imported.sound,0)[30]==16 || read137(&imported.sound,1)[30]==16);
 CHECK(read137(&imported.sound,0)[27] || read137(&imported.sound,1)[27]);
 /* Mathematically isolate the new byte from otherwise supported PB/media. */
 project_stage=imported; CHECK(project_validate_stage()<0);
 for(unsigned owner=0;owner<NBASS;owner++)memset(slot137(&project_stage.sound,owner)+27,0,3);
 CHECK(project_validate_stage()==0);
 memcpy(stored,store,sizeof stored);memcpy(lengths,store_len,sizeof lengths);
 live=proj; unsigned writes=store_writes;
 CHECK(project_load()==-9 && project_storage_unsupported());
 CHECK(project_save()==-9 && project_load_stage()==-9);
 CHECK(!memcmp(&live.sound,&proj.sound,sizeof proj.sound));
 CHECK(writes==store_writes && !memcmp(stored,store,sizeof stored)&&!memcmp(lengths,store_len,sizeof lengths));
 puts("PASS exact prior 763 reader admitted Classic-I-cleared mixed control, refused authored Classic I b27-29 media before writes");
 return 0;
}
