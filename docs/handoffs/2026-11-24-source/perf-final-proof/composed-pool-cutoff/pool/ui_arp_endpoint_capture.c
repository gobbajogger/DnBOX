#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
static void state(unsigned o,unsigned field,int high,const char *state) {
 char name[64];
 snprintf(name,sizeof name,"%c-%s-%s-%s",'A'+o,field==0?"STEPS":"GATE",high?"hi":"lo",state);
 ui_frame(); shot(name);
}
int main(void) {
 for(unsigned o=0;o<2;o++) {
  setup(V_SEQ,PART_303A+o); proj.set.bt|=PROJ_BT_CLK_IN;
  CHECK(!engine_bass_select_preset(o,ORIGINAL_SOUND_JUNO,0,&proj.sound));
  for(int n=0;n<1024&&engine_bass_preset_pending(o);n++)render(1);
  CHECK(engine_bass_bank(o)==ORIGINAL_SOUND_JUNO); build_pages();
  for(int n=0;n<3;n++){now_ms+=200;enc_acc[EN_SELECT]=1;input();}
  CHECK(arp_seq_page()); CHECK(pg.r[cur_page()][0].a==ARP_F_LENGTH && pg.r[cur_page()][2].a==ARP_F_GATE);
  for(unsigned field=0;field<2;field++) {
   unsigned k=field*2u, key=field?ARP_F_GATE:ARP_F_LENGTH;
   int max=field?99:15;
   for(int high=0;high<2;high++) {
    for(int n=0;n<=max && arp_get(o,key)!=(high?max:0);n++) {
      now_ms+=200; turn(k,high?1:-1);
    }
    CHECK(arp_get(o,key)==(high?max:0)); now_ms+=1500;
    now_ms+=200;turn(k,high?1:-1);
    state(o,field,high,"limit");
    CHECK(arp_get(o,key)==(high?max:0));
    /* Exactly same bound via real input after cue expiration. */
    now_ms+=1500;state(o,field,high,"stable");
    now_ms+=200;turn(k,high?-1:1);
    state(o,field,high,"edit");
    CHECK((high?arp_get(o,key)<max:arp_get(o,key)>0));
    printf("PASS %c %s %s bound exact, inward edit %d\n",'A'+o,field?"GATE":"STEPS",high?"high":"low",arp_get(o,key));
    for(int n=0;n<=max && arp_get(o,key)!=(high?max:0);n++) {
      now_ms+=200; turn(k,high?1:-1);
    }
    CHECK(arp_get(o,key)==(high?max:0));
    state(o,field,high,"arrive");
   }
  }
 }
 return 0;
}
