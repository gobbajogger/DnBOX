/* Visible waveforms must follow actual loop assignments and temporary audition,
 * not a fixed internal bank. Expected PCM comes from known storage assets. */
#define SMC_UI_MAIN inherited_outline_fixture
#include "smc_ui_test.c"
#include "dsp/ima_adpcm.h"
static uint8_t outline_assets[3][4096];
static void expected_outline(const uint8_t *data,uint32_t samples,uint8_t out[232])
{
 uint32_t per=(samples+231)/232,col=0;int32_t pred=0,index=0,peak=0;
 memset(out,0,232);
 for(uint32_t i=0;i<samples;i++){
  int32_t sample=ima_decode(data,i,&pred,&index);uint32_t next=i/per;
  if(next!=col){out[col]=(uint8_t)(peak>=32768?255:peak>>7);col=next;peak=0;}
  if(sample<0)sample=-sample;
  if(sample>peak)peak=sample;
 }
 out[col]=(uint8_t)(peak>=32768?255:peak>>7);
}
static void check_outline(int bank,int asset)
{
 uint8_t expected[232],actual[232];
 expected_outline(outline_assets[asset],8192,expected);memset(actual,0xa5,sizeof actual);
 CHECK(engine_brk_outline(bank,actual,232));CHECK(!memcmp(actual,expected,sizeof actual));
 if(bank==0){draw_header();draw_main();draw_knobs();CHECK(ui.outline_ok);CHECK(!memcmp(ui.outline,expected,sizeof expected));}
}
int main(void)
{
 setup(V_PART,PART_BRK);for(int n=0;n<16;n++)audio_block();
 int nb=engine_brk_nslots();const uint8_t bytes[3]={0x10,0x21,0x32};
 for(int k=0;k<3;k++){
  memset(outline_assets[k],bytes[k],sizeof outline_assets[k]);
  slot_is_img[k]=0;slot_nz[k]=1;slot_data[k][0]=outline_assets[k];
  slot_n[k][0]=8192;slot_rate[k][0]=22050;
 }
 uint8_t references[3][232];
 for(int k=0;k<3;k++)expected_outline(outline_assets[k],8192,references[k]);
 for(int a=0;a<3;a++)for(int b=a+1;b<3;b++)CHECK(memcmp(references[a],references[b],232)!=0);
 engine_brk_loops();CHECK(engine_brk_nslots()==nb+3);
 pref_set(PR(R_BRKSLOT,1,0,0),nb);audio_block();
 for(int asset=0;asset<3;asset++){
  pref_set(PR(R_BRKSLOT,0,0,0),nb+asset);audio_block();
  check_outline(0,asset);check_outline(1,0);
 }
 CHECK(brk_choice_turn(PR(R_BRKSLOT,1,0,0),1));audio_block();
 CHECK(engine_brk_preview_active());CHECK(ui.brk_choice_slot==nb+1);
 check_outline(0,1);check_outline(1,0);shot("break-actual-audition-waveform");
 brk_choice_cancel();audio_block();engine_brk_flush();now_ms+=2000;
 check_outline(0,2);check_outline(1,0);shot("break-restored-waveform");
 /* Audio-owner performance takeover restores publication without a UI cancel. */
 CHECK(brk_choice_turn(PR(R_BRKSLOT,1,0,0),1));audio_block();check_outline(0,1);
 engine_brk_live(0,1);audio_block();CHECK(!engine_brk_preview_active());
 check_outline(0,2);engine_brk_live(0,0);audio_block();brk_choice_cancel();
 check_outline(0,2);
 /* Same storage address and length can contain newly loaded material. */
 engine_brk_drop_range(outline_assets[2],sizeof outline_assets[2]);
 memset(outline_assets[2],0x03,sizeof outline_assets[2]);
 uint8_t replacement[232];expected_outline(outline_assets[2],8192,replacement);
 CHECK(memcmp(replacement,references[2],232)!=0);
 engine_brk_loops();audio_block();
 check_outline(0,2);
 /* Requested slot changes while publication waits for a held-slice release. */
 pref_set(PR(R_BRKSLICE,0,0,0),1);audio_block();CHECK(engine_brk_count(0)==8);
 engine_brk_live(32,1);audio_block();CHECK(engine_brk_held());
 pref_set(PR(R_BRKSLICE,0,0,0),0);pref_set(PR(R_BRKSLOT,0,0,0),nb+1);
 CHECK(engine_brk_count(0)==8);check_outline(0,2);
 brkpart_t pending=cur_pat()->brk;
 engine_brk_live(32,0);audio_block();engine_brk_flush();CHECK(engine_brk_count(0)==4);
 CHECK(!memcmp(&pending,&cur_pat()->brk,sizeof pending));now_ms+=2000;check_outline(0,1);
 shot("break-deferred-waveform-adopted");
 uint8_t guarded[232];memset(guarded,0xa5,sizeof guarded);
 CHECK(!engine_brk_outline(-1,guarded,232));CHECK(!engine_brk_outline(2,guarded,232));
 CHECK(!engine_brk_outline(0,0,232));CHECK(!engine_brk_outline(0,guarded,0));
 for(unsigned i=0;i<sizeof guarded;i++)CHECK(guarded[i]==0xa5);
 puts("BREAK actual assigned/audition/restored waveform snapshot PASS");return 0;
}
