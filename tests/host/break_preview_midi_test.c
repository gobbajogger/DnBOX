/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN inherited_smc_fixture
#include "smc_ui_test.c"
static uint8_t preview_data[2][4096];
static void audio_tick(void){int32_t out[512];engine_render(out,256);}
int main(void){
 setup(V_PART,PART_BRK);int nb=engine_brk_nslots();
 for(int k=0;k<2;k++){memset(preview_data[k],k?0x45:0x10,sizeof preview_data[k]);slot_is_img[k]=0;slot_nz[k]=1;slot_data[k][0]=preview_data[k];slot_n[k][0]=8192;slot_rate[k][0]=22050;}
 engine_brk_loops();CHECK(engine_brk_nslots()==nb+2);
 brkpart_t *bp=&seq.pat[seq.ppat[PART_BRK]].brk;bp->slot_a=nb;bp->slot_b=nb;
 engine_brk_loops();for(int i=0;i<16;i++)audio_tick();brkpart_t before=*bp;
 CHECK(brk_choice_turn(PR(R_BRKSLOT,0,0,0),1));audio_tick();
 CHECK(!memcmp(bp,&before,sizeof before));CHECK(engine_brk_count(0)==1);CHECK(engine_brk_held()!=0);
 brk_choice_cancel();audio_tick();engine_brk_flush();CHECK(!memcmp(bp,&before,sizeof before));CHECK(!ui.brk_choice_on);CHECK(!engine_brk_held());
 CHECK(brk_choice_turn(PR(R_BRKSLOT,0,0,0),1));CHECK(brk_choice_confirm());audio_tick();engine_brk_flush();CHECK(bp->slot_a==nb+1);CHECK(!engine_brk_held());CHECK(!ui.brk_choice_on);
 engine_brk_live(33,1);CHECK(!engine_brk_preview(nb));audio_tick();engine_brk_live(33,0);audio_tick();
 bp->slice_mode=0;engine_brk_loops();audio_tick();CHECK(engine_brk_count(0)>1);
 seq.ch[PART_BRK]=4;CHECK(brk_choice_turn(PR(R_BRKSLOT,0,0,0),-1));audio_tick();
 host_midi_put(0x29u | (0x94u<<8) | (13u<<16) | (100u<<24));audio_tick();
 CHECK(!brk_choice_confirm());CHECK(bp->slot_a==nb+1);
 CHECK(engine_brk_count(0)>1); /* performance MIDI must use original slicing, not audition count1 */
 engine_brk_preview_end();host_midi_put(0x28u | (0x84u<<8) | (13u<<16));audio_tick();engine_brk_flush();CHECK(!engine_brk_held());
 CHECK(engine_brk_preview(nb));audio_tick();engine_brk_live(32,1);audio_tick();
 CHECK(engine_brk_count(0)>1);engine_brk_preview_end();audio_tick();CHECK(engine_brk_held()&1u);
 engine_brk_live(32,0);audio_tick();CHECK(!engine_brk_held());
 CHECK(brk_choice_turn(PR(R_BRKSLOT,0,0,0),-1));audio_tick();
 unsigned next=(seq.ppat[PART_BRK]+1)%NPAT;seq.pat[next].brk=*bp;seq.pat[next].brk.slot_a=nb;seq.ppat[PART_BRK]=next;
 engine_brk_loops();audio_tick();CHECK(!engine_brk_preview_active());CHECK(engine_brk_count(0)>1);CHECK(!brk_choice_confirm());
 puts("BREAK full host queued preview/cancel/confirm PASS");return 0;
}
