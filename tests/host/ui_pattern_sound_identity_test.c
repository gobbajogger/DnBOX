/* Pattern recall identities remain readable without changing stored codes. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
int main(void) {
 static const unsigned codes[]={0,1,129,0xC1,0xC2,0xC3,0xC4,0xE1,255};
 static const char *const expected[]={"OFF","C1","M1","S1","S1","S1","S1","J1","INVALID"};
 for(int slot=0;slot<2;slot++) {
  setup(V_SEQ,PART_303A+slot);
  for(unsigned i=0;i<sizeof(codes)/sizeof(*codes);i++) {
   pat_of(PART_303A+slot)->rsv[slot]=codes[i];
   char value[64]={0},unit[32]={0};
   pref_value(PR(R_BPATSND,0,slot,0),value,unit);
   if(strcmp(value,expected[i]))fprintf(stderr,"PATTERN_SOUND_IDENTITY_MISMATCH code=%u got=%s want=%s\n",codes[i],value,expected[i]);
   CHECK(!strcmp(value,expected[i]));CHECK(!*unit);
   CHECK(pat_of(PART_303A+slot)->rsv[slot]==codes[i]);
  }
 }
 puts("PASS pattern sound engine identity and unchanged historical recall bytes");
 return 0;
}
