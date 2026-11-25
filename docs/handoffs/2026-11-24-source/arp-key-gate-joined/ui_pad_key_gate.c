/* SPDX-License-Identifier: GPL-3.0-only */
#define X0X_ARP_KEYBOARD_TEST 1
#include "gate_helpers.inc"

static uint32_t physical_pads;
static uint64_t physical_keyboard;
uint32_t plat_arp_input(uint64_t *keyboard)
{
 *keyboard=physical_keyboard;
 return physical_pads;
}
static void gesture(uint32_t pads,uint64_t keyboard)
{
 physical_pads=pads;physical_keyboard=keyboard;
 held_keys=pads|(uint32_t)(keyboard>>5);
 ++now_ms;input();render(4);
}
static unsigned gates_s1(const s1_dry_part *p,unsigned note)
{
 unsigned n=0;
 for(unsigned i=0;i<S1C_VOICES;++i)n+=p->e.v[i].root==(int)note&&p->e.v[i].gate>0.f;
 return n;
}
static void select_bank(unsigned v,unsigned bank)
{
 current=NULL;juno=NULL;
 CHECK(!engine_bass_select_preset(v,bank,0,&proj.sound));
 for(unsigned n=0;n<1024&&engine_bass_preset_pending(v);++n)render(1);
 CHECK(engine_bass_bank(v)==(int)bank);
 if(bank==ORIGINAL_SOUND_JUNO){CHECK(juno);parts_juno[v]=juno;}
 else {CHECK(current);parts[v]=current;}
}
static void physical_program(unsigned v)
{
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_SEQ);
 *page_of(V_SEQ)=3;build_pages();smc_peek_cancel();ui.oct=0;
 CHECK(arp_seq_page());
 const uint32_t pad=1u<<WHITE_KEY[0];
 const uint64_t e=UINT64_C(1)<<(52-36), alias=UINT64_C(1)<<(WHITE_KEY[0]+5);
 engine_arp_steps before,after;
 CHECK(engine_arp_steps_read(v,&before)&&before.offset[0]==0 && (before.on_mask&1u));
 uint8_t peer[ARP_BLOCK_BYTES];
 memcpy(peer,proj.sound.v[T_303][ARP_BLOCK_VOICE0+1-v],sizeof peer);
 gesture(0,0);gesture(pad,0); CHECK(arp_held==0);gesture(pad,e);
 CHECK(engine_arp_steps_read(v,&after)&&after.offset[0]==4);
 gesture(pad,0);gesture(pad,alias);
 CHECK(engine_arp_steps_read(v,&after)&&after.offset[0]==-7);
 gesture(0,alias);gesture(0,0);
 CHECK(engine_arp_steps_read(v,&after)&&after.offset[0]==-7&&after.on_mask==before.on_mask&&after.oct_mask==before.oct_mask);
 CHECK(!memcmp(peer,proj.sound.v[T_303][ARP_BLOCK_VOICE0+1-v],sizeof peer));
 for(unsigned i=1;i<ARP_STEPS;++i)CHECK(after.offset[i]==before.offset[i]);
}
static void configure_clock(unsigned s)
{
 ui.overlay=O_NONE;ui.part=PART_303A+s;set_view(V_PART);build_pages();visit("ARP PARAM");
 CHECK(engine_arp_source_get(s,ARP_F_SOURCE_RATE)==120&&engine_arp_source_get(s,ARP_F_SOURCE_FACTOR)==9);
 detent(0,1);render(1);detent(1,-1);render(1);
 CHECK(engine_arp_source_get(s,ARP_F_SOURCE_RATE)==121&&engine_arp_source_get(s,ARP_F_SOURCE_FACTOR)==10);
}
static void program_play(unsigned j)
{
 const unsigned s=1-j;
 stage_juno(1);select_bank(s,ORIGINAL_SOUND_SYNTHONE);
 /* Only config is set here; no programmed note or toggle is seeded. */
 set_arp(j,ARP_F_ON,1);set_arp(j,ARP_F_MODE,ARP_SEQ);set_arp(j,ARP_F_DIVISION,1);set_arp(j,ARP_F_GATE,1);
 set_arp(s,ARP_F_ON,1);set_arp(s,ARP_F_MODE,ARP_SEQ);
 physical_program(j);physical_program(s);
 configure_clock(s);
 select_page_gate(s);gate_number(100);detent(2,-1);render(1);gate_number(99);
 select_page_gate(j);gate_number(1);detent(2,-1);render(1);gate_number(0);
 CHECK(arp_of(j).gate==0 && arp_of(s).gate==99);
 CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 CHECK(arp_of(j).gate==0 && arp_of(s).gate==99);
 CHECK(engine_arp_source_get(s,ARP_F_SOURCE_RATE)==121&&engine_arp_source_get(s,ARP_F_SOURCE_FACTOR)==10);
 sound_t prior=proj.sound;
 const unsigned base=60,programmed=53;
 key_midi(j,base,100);sample(1);
 CHECK(gates(parts_juno[j],programmed)==1 && !gates(parts_juno[j],base) && !gates_s1(parts[s],programmed));
 sample(1);CHECK(!all_gates(parts_juno[j]));
 key_midi(j,base,0);sample(1);CHECK(!all_gates(parts_juno[j]));
 key_midi(s,base,100);sample(1);
 CHECK(gates_s1(parts[s],programmed)==1 && !gates_s1(parts[s],base) && !all_gates(parts_juno[j]));
 sample(1);CHECK(gates_s1(parts[s],programmed)==1); /* Original independent clock, positive gate. */
 key_midi(s,base,0);sample(1);CHECK(!gates_s1(parts[s],programmed));
 CHECK(!memcmp(&prior,&proj.sound,sizeof prior));
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 program_play(0);program_play(1);
 printf("PASS actual SMK pad+key and same-white alias, both engines/owners, programmed pitch after recall, physical Gate/zero next-sample release and active S1 clock (%u checks)\n",checks);
 return 0;
}
