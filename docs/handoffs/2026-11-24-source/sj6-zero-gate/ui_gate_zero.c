#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include "app/original_runtime.h"
#include "dsp/juno6.h"
static juno6_t *juno;
void __real_juno6_init(juno6_t *,float);
void __wrap_juno6_init(juno6_t *p,float hz){__real_juno6_init(p,hz);juno=p;}
static juno6_t *parts_juno[NBASS];
static unsigned gates(const juno6_t *p,unsigned note)
{
 unsigned n=0;
 for(unsigned i=0;i<DNBX_VOICES;++i)n+=p->core.voice[i].note==note && p->core.voice[i].gate;
 return n;
}
static unsigned all_gates(const juno6_t *p)
{
 unsigned n=0;
 for(unsigned i=0;i<DNBX_VOICES;++i)n+=p->core.voice[i].gate;
 return n;
}
static void key_midi(unsigned v,unsigned note,unsigned vel)
{
 unsigned ch=seq.ch[TRK_BASS0+v];
 host_midi_put((vel?9u:8u) | ((vel?0x90u:0x80u)|ch)<<8 | note<<16 | vel<<24);
}
static void stage_juno(int typed)
{
 project_reset_stage();
 if(typed) project_stage.set.snd_rev=REV137;
 for(unsigned v=0;v<NBASS;++v){
  CHECK(original_sound_init(project_stage.sound.v[T_303][v],ORIGINAL_SOUND_JUNO,0));
  engine_bass_sound_refresh(&project_stage.sound,v);
  if(typed){slot137(&project_stage.sound,v)[0]=137;slot137(&project_stage.sound,v)[1]=(uint8_t)v;}
 }
 CHECK(!project_validate_stage());project_commit_stage();
 stabilize();
 for(unsigned v=0;v<NBASS;v++) {
    juno=NULL;
    CHECK(!engine_bass_select_preset(v,ORIGINAL_SOUND_JUNO,0,&proj.sound));
    for(unsigned n=0;n<1024&&engine_bass_preset_pending(v);++n)render(1);
    CHECK(juno);parts_juno[v]=juno;
 }
 for(unsigned p=0;p<NPAT;++p)for(unsigned v=0;v<NBASS;++v)
    for(unsigned n=0;n<NSTEPS;++n)proj.pat[p].bass[v].step[n].flags=G_REST;
 CHECK(parts_juno[0]!=parts_juno[1]);
}
static engine_arp_settings arp_of(unsigned v)
{
 engine_arp_settings x;CHECK(engine_arp_settings_read(v,&x));return x;
}
static void select_page_gate(unsigned v)
{
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_SEQ);smc.view=V_SEQ;build_pages();
 visit("ARP SEQ");
 CHECK(arp_seq_page() && pg.r[cur_page()][2].kind==R_ARP && pg.r[cur_page()][2].a==ARP_F_GATE);
 smc_peek_cancel();smc_page_set(cur_page());
}
static void gate_number(int n)
{
 char num[16],unit[8],expected[16];
 snprintf(expected,sizeof expected,"%d",n);
 pref_value(pg.r[cur_page()][2],num,unit);
 CHECK(!strcmp(num,expected)&&!strcmp(unit,"%"));
}
static void set_arp(unsigned owner,unsigned field,unsigned value)
{
 CHECK(engine_arp_set(owner,arp_of(owner).revision,field,(int)value)==S1_EDIT_QUEUED);
 render(1);
}
static void sample(unsigned frames)
{
 int32_t lr[100]={0};
 CHECK(frames<=50);engine_render(lr,frames);
}
static void run_free(unsigned owner,unsigned mode)
{
 unsigned peer=1-owner, note=62+owner;
 set_arp(owner,ARP_F_MODE,mode);
 CHECK(!seq.playing && !all_gates(parts_juno[owner]));
 key_midi(owner,note,100);sample(1);CHECK(gates(parts_juno[owner],note)==1);
 sample(1);CHECK(!all_gates(parts_juno[owner])&&!all_gates(parts_juno[peer]));
 /* A zero gate is not a rest. It leaves the key held and fires next grid. */
 unsigned seen=0;
 for(unsigned n=0;n<1500;++n){
    sample(1);
    if(gates(parts_juno[owner],note)){
      ++seen;sample(1);CHECK(!all_gates(parts_juno[owner]));break;
    }
 }
 CHECK(seen==1);
 key_midi(owner,note,0);sample(2);CHECK(!all_gates(parts_juno[owner]));
}
static void run_sync(unsigned owner)
{
 unsigned note=50+owner;
 set_arp(owner,ARP_F_MODE,ARP_SEQ);
 seq_start(&seq);sample(50);key_midi(owner,note,100);sample(1);
 CHECK(!all_gates(parts_juno[owner]));
 unsigned seen=0;
 for(unsigned n=0;n<1500;++n){
    sample(1);
    if(gates(parts_juno[owner],note)){++seen;sample(1);CHECK(!all_gates(parts_juno[owner]));break;}
 }
 CHECK(seen==1);
 seq_stop(&seq);key_midi(owner,note,0);sample(4);CHECK(!all_gates(parts_juno[owner]));
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 stage_juno(0); select_page_gate(0);
 CHECK(pref_desc(pg.r[cur_page()][2])->max==99);
 CHECK(engine_arp_set(0,arp_of(0).revision,ARP_F_GATE,0)==S1_EDIT_REJECTED);
 stage_juno(1);
 for(unsigned v=0;v<NBASS;++v){
    const unsigned peer=1-v;
    set_arp(v,ARP_F_ON,1);set_arp(v,ARP_F_DIVISION,1);
    set_arp(v,ARP_F_GATE,1);
    engine_arp_steps steps;CHECK(engine_arp_steps_read(v,&steps));
    sound_t before=proj.sound;
    select_page_gate(v);gate_number(1);CHECK(pref_desc(pg.r[cur_page()][2])->max==100);
    undo_start();
    detent(2,-1);render(1);gate_number(0);CHECK(arp_of(v).gate==0);
    CHECK(read137(&proj.sound,v)[SJ6_ARP_ZERO_BYTE]==1);
    CHECK(proj.sound.v[T_303][ARP_BLOCK_VOICE0+v][5]==1);
    slot137(&before,v)[SJ6_ARP_ZERO_BYTE]=1;
    CHECK(!memcmp(&before,&proj.sound,sizeof before));
    engine_arp_steps after;CHECK(engine_arp_steps_read(v,&after)&&steps.on_mask==after.on_mask&&steps.oct_mask==after.oct_mask);
    CHECK(!memcmp(steps.offset,after.offset,sizeof after.offset));
    CHECK(arp_of(peer).gate==(v?0:100));
    CHECK(undo_commit_stable());physical_history(0);CHECK(arp_of(v).gate==1);
    physical_history(1);CHECK(arp_of(v).gate==0);
    run_free(v,ARP_UP);run_free(v,ARP_SEQ);run_sync(v);
 }
 project_capture_stage(); slot137(&project_stage.sound,0)[SJ6_ARP_ZERO_BYTE]=2;CHECK(project_validate_stage()<0);
 CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 for(unsigned v=0;v<NBASS;++v)CHECK(arp_of(v).gate==0);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1&&!fclose(f));
 select_page_gate(0);detent(2,1);render(1);gate_number(1);
 CHECK(arp_of(0).gate==1 && arp_of(1).gate==0 && !read137(&proj.sound,0)[SJ6_ARP_ZERO_BYTE]);
 printf("PASS Build9 GATE 0 on actual K3: both owners ARP/SEQ onset then next-sample off, held repeat, transport and cold recall (%u)\n",checks);
}
