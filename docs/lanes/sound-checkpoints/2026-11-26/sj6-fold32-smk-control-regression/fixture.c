#define main old_ui_arp_main
#include "ui_arp_engine_test.c"
#undef main
extern unsigned fold_arp_admit(unsigned);
extern unsigned fold_arp_state(unsigned,unsigned);
extern unsigned sj6_fold_test_samples;
static struct {uint32_t frames;int32_t wheel[2];uint16_t wheel_mod_seq;} fm1_in;
static int32_t clamp(int32_t x,int32_t l,int32_t h){return x<l?l:x>h?h:x;}
#include "actual_smk_wheels.inc"
static void physical_wheels(int p,int m){
 fm1_in.frames=340;fm1_in.wheel[0]=p;fm1_in.wheel[1]=m;++fm1_in.wheel_mod_seq;smk_wheels();
}

static void source_midi(unsigned t,unsigned d1,unsigned d2)
{
 unsigned st=t|(seq.ch[PART_303A]&15u);
 min_q[mi_w++%MQ]=(st>>4)|(st<<8)|(d1<<16)|(d2<<24);
}
static void registered(void)
{
  setup(V_SEQ,PART_303A);
  EXPECT(!engine_bass_select_preset(0,ORIGINAL_SOUND_JUNO,0,&proj.sound));
  for(unsigned n=0;n<1024&&engine_bass_preset_pending(0);n++)render(1);
  EXPECT(engine_bass_bank(0)==ORIGINAL_SOUND_JUNO);
  EXPECT(engine_superj6_pre_note_wave(0,0,0.f,0));
  render(2);
  EXPECT(engine_superj6_pre_note_wave(0,1,1.f/3.f,0));
  render(2);
  EXPECT(fold_arp_admit(0));
  engine_arp_cfg c;engine_arp_cfg_default(&c);
  c.on=1;c.mode=ARP_SEQ;c.length=16;c.speed=ARP_SPEED_1_16;c.gate=49;c.range=1;c.on_mask=0xffff;
#if X0X_SJ6_ARP_ADAPTER_NATIVE
  c.seq_on=1;
#endif
  for(unsigned j=0;j<16;j++)c.offset[j]=(signed char)j-8;
  EXPECT(engine_arp_preset(0,&c));render(4);
  build_pages();int page=page_named("ARP SEQ");EXPECT(page>=0);
  if(page<0)return;
  *page_of(V_SEQ)=page;build_pages();ui_frame();
  EXPECT(arp_seq_page());
  engine_arp_settings q;EXPECT(engine_arp_settings_read(0,&q)&&q.length==16&&q.gate==49);
  engine_arp_steps v;EXPECT(engine_arp_steps_read(0,&v)&&v.on_mask==0xffff);
  for(unsigned j=0;j<16;j++)EXPECT(v.offset[j]==(int)j-8);
  seq.playing=1;engine_bass_on(0,60,0,0);
  unsigned visits=0,opened=0,closed=0;
  for(unsigned j=0;j<740;j++){
    if(j==16)source_midi(0xe0,0,90);
    if(j==32)source_midi(0xb0,1,112);
    if(j==60)source_midi(0xe0,0,64);
    if(j==80)source_midi(0xb0,1,0);
    if(j==109)physical_wheels(550,615);
    if(j==112)physical_wheels(630,485);
    if(j==136)physical_wheels(550,615);
    if(j==385){
       /* UI edits: physical white step pad then encoder while held. */
       pad_down(5);turn(0,3);turn(2,1);pad_up();
       EXPECT(engine_arp_steps_read(0,&v)&&v.offset[5]==0&&(v.oct_mask&(1u<<5)));
       EXPECT(engine_arp_block_decode(proj.sound.v[T_303][ARP_BLOCK_VOICE0],&c)&&c.offset[5]==0);
       printf("STEP_EDIT %04x %04x %d\n",v.on_mask,v.oct_mask,v.offset[5]);
    }
    render(1);
    unsigned x=fold_arp_state(0,j*256);
    if(j<384){visits|=x&65535;opened+=!!(x>>16);closed+=!(x>>16);}
  }
  EXPECT(visits==65535);EXPECT(opened&&closed);
  engine_bass_off(0);render(64);seq.playing=0;
  printf("COVER %04x %u %u\n",visits,opened,closed);
#if SJ6_WAVE_PROBE
  EXPECT(sj6_fold_test_samples>0);
#else
  EXPECT(sj6_fold_test_samples==0);
#endif
  fprintf(stderr,"typed_fold_samples=%u\n",sj6_fold_test_samples);
}
static void stock_modes(void)
{
  setup(V_SEQ,PART_303A);
  EXPECT(!engine_bass_select_preset(0,ORIGINAL_SOUND_JUNO,0,&proj.sound));
  for(unsigned j=0;j<1024&&engine_bass_preset_pending(0);j++)render(1);
  EXPECT(engine_bass_bank(0)==ORIGINAL_SOUND_JUNO);
  EXPECT(engine_superj6_pre_note_wave(0,0,2.f/3.f,0));
  render(2);
  const engine_superj6_pwm pwm={.amount=.4f,.enable=1.f};
  EXPECT(engine_superj6_pre_note_wave(0,1,1.f,&pwm));
  render(2);
  unsigned admitted=sj6_fold_test_samples;
  for(unsigned mode=ARP_UP;mode<=ARP_UPDN;mode++){
    engine_arp_cfg c;engine_arp_cfg_default(&c);
    c.on=1;c.mode=mode;c.speed=ARP_SPEED_1_16;c.gate=62;c.range=1;
    EXPECT(engine_arp_preset(0,&c));render(4);
    engine_arp_settings q;EXPECT(engine_arp_settings_read(0,&q)&&q.mode==mode&&q.on);
    seq.playing=1;
    engine_bass_on(0,60,0,0);engine_bass_on(0,64,0,0);engine_bass_on(0,67,0,0);
    uint32_t h=2166136261u;unsigned visits=0;
    for(unsigned j=0;j<170;j++){
      int32_t a[512];engine_render(a,256);
      for(unsigned i=0;i<512;i++)h=(h^(uint32_t)a[i])*16777619u;
      unsigned pos,age;
      if(engine_arp_view(0,&pos,&age))visits|=1u<<(pos&15);
    }
    EXPECT(visits);
    engine_bass_off(0);render(64);seq.playing=0;
    printf("STOCK_MODE %u %04x %08x\n",mode,visits,h);
  }
  EXPECT(sj6_fold_test_samples==admitted);
}
int main(void){
 registered();
 stock_modes();
 if(failures)return 1;
 printf("PASS fold arp/wheel %u\n",expectations);return 0;
}
