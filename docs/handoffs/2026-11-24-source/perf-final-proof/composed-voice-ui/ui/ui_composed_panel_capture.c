#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"

static void visit(const char *name){
 for(int i=0;i<pg.n;i++){
   if(!strcmp(name,pg.title[cur_page()])) return;
   now_ms+=200; enc_acc[EN_SELECT]=1;input();
 }
 fprintf(stderr,"Missing page %s\n",name); exit(1);
}
static void rec(const char *prefix, const char *tag, int k){
 char path[128],num[32]={0},unit[16]={0};
 snprintf(path,sizeof path,"%s-%s",prefix,tag);
 ui_frame();shot(path);
 if(k>=0){
  pref_t r=pg.r[cur_page()][k]; int v=matrix_value(r);
  pref_value_of(r,v,num,unit);
  printf("%s K%d %d %s%s touched=%d\n",path,k+1,v,num,unit,ui.touched);
 } else printf("%s page %d/%d\n",path,cur_page()+1,pg.n);
}
static void panel(const char *token,const char *name,unsigned owner){
 char prefix[96], tag[24];
 snprintf(prefix,sizeof prefix,"%c-%s",'A'+owner,token);
 visit(name); now_ms+=3000; rec(prefix,"default",-1);
 for(int k=0;k<4;k++){
  if(pg.r[cur_page()][k].kind==R_NONE) continue;
  for(int side=0;side<2;side++){
   for(int loop=0;loop<160;loop++){now_ms+=160;turn(k,side?1:-1);}
   snprintf(tag,sizeof tag,"K%d-%s",k+1,side?"high":"low");rec(prefix,tag,k);
  }
 }
}
int main(void){
 for(unsigned owner=0;owner<2;owner++){
  for(int is_j=0;is_j<2;is_j++){
   int bank=is_j?ORIGINAL_SOUND_JUNO:ORIGINAL_SOUND_SYNTHONE;
   setup(V_PART,PART_303A+owner);proj.set.bt|=PROJ_BT_CLK_IN;
   CHECK(!engine_bass_select_preset(owner,bank,0,&proj.sound));
   for(int n=0;n<1024&&engine_bass_preset_pending(owner);n++)render(1);
   CHECK(engine_bass_bank(owner)==bank);build_pages();
   if(is_j){panel("SJ6-LFO-RATE","LFO RATE",owner);panel("SJ6-TUNING","TUNING",owner);panel("SJ6-ARP","ARPEGGIATOR",owner);}
   else {panel("S1-EFFECT","EFFECT",owner);panel("S1-OUTPUT","OUTPUT",owner);}
  }
 }
 return 0;
}
