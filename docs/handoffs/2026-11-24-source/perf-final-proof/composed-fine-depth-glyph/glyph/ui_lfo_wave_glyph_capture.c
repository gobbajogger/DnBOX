#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
static void walk_to(const char *title){
 for(int n=0;n<pg.n;n++){
  if(!strcmp(pg.title[cur_page()], title)) return;
  now_ms+=180; enc_acc[EN_SELECT]=1; input(); ui_frame();
 }
 CHECK(0 && "missing LFO RATE");
}
int main(void){
 for(unsigned owner=0;owner<2;owner++){
  setup(V_PART,PART_303A+owner);proj.set.bt|=PROJ_BT_CLK_IN;
  CHECK(!engine_bass_select_preset(owner,ORIGINAL_SOUND_JUNO,0,&proj.sound));
  for(int i=0;i<1024&&engine_bass_preset_pending(owner);i++)render(1);
  CHECK(engine_bass_bank(owner)==ORIGINAL_SOUND_JUNO);
  build_pages();walk_to("LFO RATE");
  pref_t shape=pg.r[cur_page()][3];
  CHECK(shape.kind==R_SJ6KNOB && shape.c==SJ6_LFO2_SHAPE && shape.b==owner);
  for(unsigned pass=0;pass<2;pass++){
   for(unsigned i=0;i<5;i++){
    unsigned expect=pass?4-i:i;
    for(int n=0;n<6&&matrix_value(shape)!=(int)expect;n++){
     now_ms+=180;turn(3,matrix_value(shape)<(int)expect?1:-1);
    }
    CHECK(engine_superj6_knob_position(owner,SJ6_LFO2_SHAPE)==(int)expect);
    char file[64],num[20]={0},unit[12]={0};
    pref_value_of(shape,expect,num,unit);
    now_ms+=1000;ui_frame();
    snprintf(file,sizeof file,"%c-%u-%u",'A'+owner,pass,expect);shot(file);
    printf("PASS %s actual shape=%u %s\n",file,expect,num);
    if(i==2){
      now_ms+=200;enc_acc[EN_SELECT]=1;input();ui_frame();
      snprintf(file,sizeof file,"%c-%u-side",'A'+owner,pass);shot(file);
      walk_to("LFO RATE");
    }
   }
  }
  walk_to("TUNING");ui_frame();shot(owner?"B-unrelated":"A-unrelated");
 }
 return 0;
}
