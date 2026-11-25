#define X0X_ARP_KEYBOARD_TEST 1
#include "gate_helpers.inc"
static uint32_t pads;
static uint64_t keys;
uint32_t plat_arp_input(uint64_t *out) { *out=keys;return pads; }
static void gesture(uint32_t p,uint64_t k)
{
 pads=p;keys=k;held_keys=p|(uint32_t)(k>>5);
 ++now_ms;input();render(4);
}
static void panel_mode(unsigned v,int max)
{
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_PART);build_pages();visit("ARPEGGIATOR");
 CHECK(pg.r[cur_page()][2].a==ARP_F_MODE&&pref_desc(pg.r[cur_page()][2])->max==max);
}
static void mode_label(unsigned v)
{
 panel_mode(v,ARP_SAGA_RANDOM2);
 char num[24],unit[8];pref_value(pg.r[cur_page()][2],num,unit);
 CHECK(!strcmp(num,"RANDOM2") && arp_of(v).mode==ARP_SAGA_RANDOM2);
}
static int current_note(const juno6_t *s)
{
 int note=-1;
 for(unsigned i=0;i<DNBX_VOICES;++i)if(s->core.voice[i].gate) {
   CHECK(note<0);note=s->core.voice[i].note;
 }
 return note;
}
static void real_pool(unsigned v)
{
 /* Deliberately unsorted holders; 65 occurs twice at interval 12 and
    pitch 132 clips only after selection. These are firmware-path tests,
    not a claim about the original Swift entropy bits. */
 static const int draw[2][12]={{120,65,53,127,127,120,65,127,53,53,65,120},
                              {120,120,65,77,120,65,53,120,127,65,53,65}};
 key_midi(v,120,100);key_midi(v,65,100);key_midi(v,53,100);
 int n=-1;
 for(unsigned event=0;event<12;++event){
   for(unsigned frames=0;frames<1400;++frames){
     sample(1); n=current_note(parts_juno[v]);
     if(n>=0)break;
   }
   CHECK(n==draw[v][event] && !all_gates(parts_juno[1-v]));
   sample(1);CHECK(!all_gates(parts_juno[v]));
 }
 key_midi(v,53,0);key_midi(v,65,0);key_midi(v,120,0);sample(3);
 CHECK(!all_gates(parts_juno[v]));
}
static void program_grid(unsigned v)
{
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_SEQ);
 *page_of(V_SEQ)=3;build_pages();smc_peek_cancel();ui.oct=0;
 CHECK(arp_seq_page() && pg.r[cur_page()][0].a==ARP_F_LENGTH &&
       pg.r[cur_page()][1].a==ARP_F_INTERVAL && pg.r[cur_page()][2].a==ARP_F_GATE);
 uint32_t pad=1u<<WHITE_KEY[0];
 uint64_t alias=UINT64_C(1)<<(WHITE_KEY[0]+5);
 gesture(0,0);gesture(pad,0);gesture(pad,alias);gesture(0,alias);gesture(0,0);
 engine_arp_steps st;CHECK(engine_arp_steps_read(v,&st)&&st.offset[0]==-7&&(st.on_mask&1u));
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 stage_juno(0);
 panel_mode(0,ARP_UPDN_REPEAT);
 CHECK(engine_arp_set(0,arp_of(0).revision,ARP_F_MODE,ARP_SAGA_RANDOM2)==S1_EDIT_REJECTED);
 stage_juno(1);
 for(unsigned v=0;v<NBASS;++v){
  set_arp(v,ARP_F_ON,1);set_arp(v,ARP_F_DIVISION,1);set_arp(v,ARP_F_RANGE,2);
  set_arp(v,ARP_F_INTERVAL,12);set_arp(v,ARP_F_GATE,1);
  set_arp(v,ARP_F_MODE,ARP_UPDN_REPEAT);
  sound_t prior=proj.sound;
  panel_mode(v,ARP_SAGA_RANDOM2);undo_start();detent(2,1);render(1);mode_label(v);
  slot137(&prior,v)[SJ6_ARP_RANDOM_BYTE]=2;
  CHECK(!memcmp(&prior,&proj.sound,sizeof prior));
  CHECK(undo_commit_stable());
  physical_history(0);CHECK(arp_of(v).mode==ARP_UPDN_REPEAT);
  physical_history(1);mode_label(v);
  program_grid(v);
  select_page_gate(v);gate_number(1);detent(2,-1);render(1);gate_number(0);
  CHECK(arp_of(v).gate==0 && arp_of(v).mode==ARP_SAGA_RANDOM2);
 }
 project_capture_stage();slot137(&project_stage.sound,0)[SJ6_ARP_RANDOM_BYTE]=3;
 CHECK(project_validate_stage()<0);
 CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 seq.bpm=120.f;render(2);
 sound_t cold=proj.sound;unsigned writes=store_writes;
 for(unsigned v=0;v<NBASS;++v){
  mode_label(v); CHECK(arp_of(v).gate==0 && arp_of(v).range==2 && arp_of(v).interval==12);
  engine_arp_steps st;CHECK(engine_arp_steps_read(v,&st)&&st.offset[0]==-7 && (st.on_mask&1u));
  real_pool(v);
 }
 CHECK(!memcmp(&cold,&proj.sound,sizeof cold)&&writes==store_writes);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1&&!fclose(f));
 select_patch(0);panel_mode(0,ARP_SEQ);
 CHECK(read137(&proj.sound,0)[SJ6_ARP_RANDOM_BYTE]==2 &&
       engine_arp_set(0,arp_of(0).revision,ARP_F_MODE,ARP_SAGA_RANDOM2)==S1_EDIT_REJECTED);
 puts("PASS physical Build9 RANDOM2 mode4 on both owners, checked seeded pool incl repeats/overlap/clipping, zero-Gate release, pad+key grid unaffected, Undo/Redo and exact cold saved reopen");
 printf("PASS %u UBSan sample and control checks\n",checks);
}
