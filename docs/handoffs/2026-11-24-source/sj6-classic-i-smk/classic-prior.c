#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static project_t imported,live;
static uint8_t hdr[32],stored[OBJ_NOBJ][PLAT_STORE_MAX];
static int lengths[OBJ_NOBJ];
int main(void)
{
 setup(V_PART,PART_303A);ui.overlay=O_NONE;proj.set.bt|=PROJ_BT_CLK_IN;
 FILE *f=fopen("out/media.bin","rb"); CHECK(f);
 CHECK(fread(hdr,sizeof hdr,1,f)==1 && fread(&imported,sizeof imported,1,f)==1 &&
   fread(store,sizeof store,1,f)==1&&fread(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 for(unsigned v=0;v<NBASS;v++)for(unsigned i=27;i<30;i++)CHECK(read137(&imported.sound,v)[i]>0);
 project_stage=imported;CHECK(project_validate_stage()<0);
 for(unsigned v=0;v<NBASS;v++)memset(slot137(&project_stage.sound,v)+27,0,3);
 CHECK(project_validate_stage()==0);
 live=proj;memcpy(stored,store,sizeof stored);memcpy(lengths,store_len,sizeof lengths);
 unsigned n=store_writes;
 CHECK(project_load()==-9&&project_storage_unsupported());
 CHECK(project_save()==-9&&project_load_stage()==-9);
 CHECK(!memcmp(&live.sound,&proj.sound,sizeof proj.sound));
 CHECK(n==store_writes&&!memcmp(stored,store,sizeof stored)&&!memcmp(lengths,store_len,sizeof lengths));
 printf("PASS exact 603/510 prior reader rejects actual b27–29 written media, clears only those bytes to admit, zero writes: %u checks\n",checks);
}
