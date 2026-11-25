/* SPDX-License-Identifier: GPL-3.0-only */
#include "gate_helpers.inc"
static void k3(int e)
{
 now_ms+=180;
 enc_acc[EN_K1+2]=e;
 input(); /* real detent input, no rendered samples until the witness asks */
}
static void expect_on(unsigned v,unsigned note)
{
 CHECK(gates(parts_juno[v],note)==1 && !all_gates(parts_juno[1-v]));
}
#define expect_off(v) do { if (all_gates(parts_juno[(v)])||all_gates(parts_juno[1-(v)])) \
    fprintf(stderr, "off failure call line %d owner%u\n",__LINE__,(v)); \
    CHECK(!all_gates(parts_juno[(v)])&&!all_gates(parts_juno[1-(v)])); } while(0)
enum { PERIOD_Q8=352800, STEP=(PERIOD_Q8+255)/256 };
static original_runtime_part *observed[NBASS];
int __real_original_runtime_render(original_runtime_part *,original_runtime_workspace *,float *,float *,unsigned);
int __wrap_original_runtime_render(original_runtime_part *p, original_runtime_workspace *w,float *l,float *r,unsigned n)
{
 int ret=__real_original_runtime_render(p,w,l,r,n);
 if(p->kind==ORIGINAL_SOUND_JUNO)for(unsigned v=0;v<NBASS;++v)
    if(parts_juno[v]==&p->dsp.juno.synth) observed[v]=p;
 return ret;
}
static unsigned expected_deadline(unsigned v,unsigned percent)
{
 CHECK(observed[v] && observed[v]->arp.prm.arpSeqTempoMultiplier==PERIOD_Q8);
 const unsigned target=percent*PERIOD_Q8/100u;
 const unsigned phase=observed[v]->arp.seq.mBeatTime;
 /* Source clock phase is measured, deadline law is independently computed. */
 return target<=phase ? 1u : (target-phase+255u)/256u;
}
static unsigned sync_on(unsigned v,unsigned note)
{
 for(unsigned n=0;n<STEP+10;++n) {
    sample(1);
    if(gates(parts_juno[v],note))return n+1;
 }
 return 0;
}
static void verify_pair(unsigned v,int clocked)
{
 unsigned note=57+v;
 fprintf(stderr,"start owner=%u clocked=%d tempo=%.1f\n",v,clocked,seq_tempo(&seq));
 set_arp(v,ARP_F_ON,1);
 set_arp(v,ARP_F_DIVISION,1);
 set_arp(v,ARP_F_MODE,clocked?ARP_SEQ:ARP_UP);
 set_arp(v,ARP_F_GATE,1);
 select_page_gate(v);gate_number(1);
 if(clocked){seq_start(&seq);sample(50);}
 key_midi(v,note,100);sample(1);
 if(clocked){expect_off(v);CHECK(sync_on(v,note));}
 expect_on(v,note); /* t0 at actual admitted note */
 unsigned due=expected_deadline(v,1);
 if(!clocked) CHECK(due==14);
 k3(-1); /* 1%->0% at first sample after note on */
 for(unsigned t=1;t<due;++t) {
   sample(1);expect_on(v,note);
   if(t==1)gate_number(0);
 }
 sample(1);expect_off(v); /* original Q8 deadline, measured from this step onset */
 for(unsigned t=due+1;t<STEP-1;++t){sample(1);expect_off(v);}
 CHECK(sync_on(v,note));expect_on(v,note); /* Zero is applied only to subsequent step. */
 k3(1);
 sample(1);expect_off(v);gate_number(1); /* 0% onset still expires on next sample */
 CHECK(sync_on(v,note));expect_on(v,note);
 due=expected_deadline(v,1);
 for(unsigned t=1;t<due;++t){sample(1);expect_on(v,note);}
 sample(1);expect_off(v);
 if(clocked){
   CHECK(sync_on(v,note));expect_on(v,note);
   k3(-1);sample(1);expect_on(v,note);
   seq_stop(&seq); /* STOP while a latched early-release voice is active. */
 }
 key_midi(v,note,0);sample(1);expect_off(v);sample(16);expect_off(v);
}
static void multiple_pending(unsigned v)
{
 const unsigned note=71;
 set_arp(v,ARP_F_ON,1);set_arp(v,ARP_F_DIVISION,1);set_arp(v,ARP_F_MODE,ARP_UP);
 set_arp(v,ARP_F_GATE,50);select_page_gate(v);
 key_midi(v,note,100);sample(1);expect_on(v,note);
 unsigned due=expected_deadline(v,50);CHECK(due==690);
 k3(-1);sample(1);expect_on(v,note);
 k3(-1);sample(1);expect_on(v,note);gate_number(48);
 for(unsigned t=3;t<due;++t){sample(1);expect_on(v,note);}
 sample(1);expect_off(v); /* 50% original, not 49% or 48%. */
 CHECK(sync_on(v,note));expect_on(v,note);
 /* Following 48% uses threshold relative to the existing grid with fractional residual. */
 due=expected_deadline(v,48);
 CHECK(due>=660 && due<=662);
 for(unsigned t=1;t<due;++t){sample(1);expect_on(v,note);}
 sample(1);expect_off(v);
 key_midi(v,note,0);sample(1);expect_off(v);
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 stage_juno(1);seq.bpm=120.f;render(2);CHECK(seq_tempo(&seq)==120.f);
 for(unsigned v=0;v<NBASS;++v) {
    verify_pair(v,0);verify_pair(v,1);multiple_pending(v);
 }
 printf("PASS both owners real active SuperJ6 K3 preserves dispatched deadline, later steps/zero, held + synced release/STOP (%u checks)\n",checks);
}
