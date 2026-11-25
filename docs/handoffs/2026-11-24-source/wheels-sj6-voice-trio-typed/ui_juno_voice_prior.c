#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static project_t original,live;
static uint8_t hdr[32], saved[OBJ_NOBJ][PLAT_STORE_MAX];
static int lengths[OBJ_NOBJ];
int main(void)
{
 setup(V_PART,PART_303A);ui.overlay=O_NONE;proj.set.bt|=PROJ_BT_CLK_IN;
 FILE *f=fopen("out/voice-media.bin","rb");CHECK(f);
 CHECK(fread(hdr,sizeof hdr,1,f)==1&&fread(&original,sizeof original,1,f)==1&&
   fread(store,sizeof store,1,f)==1&&fread(store_len,sizeof store_len,1,f)==1&&!fclose(f));
 for(unsigned o=0;o<2;o++){
  const uint8_t *b=original.sound.v[T_303][9u+2u*o]+4;
  CHECK(b[0]==(0x83u+4u*o)&&b[1]==17u+o);
 }
 project_stage=original;CHECK(project_validate_stage()<0);
 for(unsigned o=0;o<2;o++)memset(project_stage.sound.v[T_303][9u+2u*o]+4,0,2);
 CHECK(!project_validate_stage());
 live=proj;unsigned writes=store_writes;
 memcpy(saved,store,sizeof saved);memcpy(lengths,store_len,sizeof lengths);
 CHECK(project_load()==-9&&project_storage_unsupported()&&project_save()==-9&&project_load_stage()==-9);
 CHECK(!memcmp(&live.sound,&proj.sound,sizeof proj.sound)&&store_writes==writes&&!memcmp(saved,store,sizeof saved)&&!memcmp(lengths,store_len,sizeof lengths));
 puts("PASS exact earlier receiver refuses typed b36–37 writer data before writes; clearing only two bytes per owner admits stage");
}
