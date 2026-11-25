/* SPDX-License-Identifier: GPL-3.0-only */
#define SJ6_LAYER1_FIXTURE_MAIN retained_layer_main
#include "ui_superj6_layer1_tuning_test.c"
#include "app/original_runtime.h"
static original_runtime_part *seen[2];
int __real_original_runtime_render(original_runtime_part*,original_runtime_workspace*,float*,float*,unsigned);
int __wrap_original_runtime_render(original_runtime_part *p,original_runtime_workspace *w,float*l,float*r,unsigned n)
{
    int rc=__real_original_runtime_render(p,w,l,r,n);
    if(p->kind==ORIGINAL_SOUND_JUNO)
      for(unsigned v=0;v<2;v++)if(parts[v]==&p->dsp.juno.synth)seen[v]=p;
    return rc;
}
static unsigned on(unsigned v)
{
 unsigned n=0;for(unsigned i=0;i<DNBX_VOICES;i++)n+=!!parts[v]->core.voice[i].gate;return n;
}
static engine_arp_settings conf(unsigned v)
{
 engine_arp_settings p;CHECK(engine_arp_settings_read(v,&p));return p;
}
static void change(unsigned v,unsigned param,int value)
{
 CHECK(engine_arp_set(v,conf(v).revision,param,value)==S1_EDIT_QUEUED);render(1);
}
static void samp(void)
{
 int32_t lr[2];engine_render(lr,1);
}
static void key(unsigned v,unsigned pitch,unsigned velocity)
{
 unsigned ch=seq.ch[TRK_BASS0+v];
 host_midi_put((velocity?9u:8u)|(((velocity?0x90u:0x80u)|ch)<<8)|(pitch<<16)|(velocity<<24));
}
static void k3(int e)
{
 now_ms+=180;enc_acc[EN_K3]=e;input();samp();
}
static void route(unsigned v)
{
 setup_tuning(v);seen[v]=0;
 seq.bpm=120.f;seq.ext=0;render(1);
 change(v,ARP_F_ON,1);change(v,ARP_F_MODE,ARP_UP);
 change(v,ARP_F_DIVISION,1);change(v,ARP_F_GATE,50);
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_SEQ);visit("ARP SEQ");
 CHECK(pg.r[cur_page()][2].kind==R_ARP&&pg.r[cur_page()][2].a==ARP_F_GATE);
 CHECK(engine_arp_set(v,conf(v).revision,ARP_F_GATE,0)==S1_EDIT_REJECTED);
 key(v,64+v,100);samp();CHECK(seen[v]&&on(v)==1&&!on(1-v));
 unsigned due=(seen[v]->arp.prm.gateTicks-seen[v]->arp.seq.mBeatTime+255u)/256u;
 CHECK(due==690);
 k3(-1);CHECK(on(v)==1);
 k3(-1);CHECK(on(v)==1 && conf(v).gate==48);
 for(unsigned i=3;i<due;++i){samp();CHECK(on(v)==1);}
 samp();CHECK(!on(v));
 unsigned count=0;while(!on(v)&&count++<1400)samp();
 CHECK(on(v)==1 && !on(1-v));
 due=(seen[v]->arp.prm.gateTicks-seen[v]->arp.seq.mBeatTime+255u)/256u;
 CHECK(due>=660&&due<=662);
 for(unsigned i=1;i<due;++i){samp();CHECK(on(v)==1);}
 samp();CHECK(!on(v));
 key(v,64+v,0);samp();CHECK(!on(v)&&!on(1-v));
 CHECK(proj.sound.v[T_303][ARP_BLOCK_VOICE0+v][5]==48);
 CHECK(project_save()!=0); /* native trial cannot persist */
 CHECK(proj.sound.v[T_303][ARP_BLOCK_VOICE0+v][5]==48);
}
int main(void)
{
 route(0);route(1);
 printf("PASS real K3 positive live Gate keeps own existing sample deadline then applies next; legacy bytes unchanged, save refused and zero rejection (%u checks)\n",checks);
}
