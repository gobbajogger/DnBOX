#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
static const s1c *watch;
static unsigned route,ft0,taps,offkr,held,attack,release,distinct;
static float prior;
static int zero_depth;
int fixture_s1_env(unsigned v,int pos)
{
 if(v>=NBASS||!parts[v]||!live_s1_mod(&proj.sound,v))return 0;
 s1c *e=&parts[v]->e;
 if(pos>=0){
   engine_project_hold(1);
   CHECK(s1c_front_set(e,56,(float)pos));
   engine_project_hold(0);
 }
 return (int)e->t[56];
}
static void walk(unsigned owner,unsigned choice)
{
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 visit("FILTER MODE");smc_peek_cancel();smc_page_set(cur_page());
 pref_t r=pg.r[cur_page()][3];CHECK(r.kind==R_ENV_TEACH&&r.b==owner);
 CHECK(pg.r[cur_page()][2].c==S1_CUTOFF_LFO);
 for(unsigned i=0;i<4&&fixture_s1_env(owner,-1)!=(int)choice;i++)
  detent(3,fixture_s1_env(owner,-1)<(int)choice?1:-1);
 static const char *const names[]={"OFF","LFO 1","LFO 2","BOTH"};
 char value[16],unit[8];pref_value(r,value,unit);
 CHECK(!strcmp(value,names[choice])&&!*unit);
}
void host_env_tap(const s1c *s,const s1c_voice *v,unsigned ft,float base,float env,float l1,float l2,float hz)
{
 if(s!=watch)return;
 CHECK(ft==ft0);
 float scale=route==1?l1:route==2?l2:.5f*(l1+l2);
 float expected=base-base*(env*scale)*(1.f-v->fenv);
 if(expected<64.f)expected=64.f;if(expected>22050.f)expected=22050.f;
 CHECK(hz==expected);
 if(v->kc!=S1C_KR){offkr++;if(hz!=prior)distinct++;}
 if(v->gate)held++;
 if(v->ef.mode==S1U_E_ATK)attack++;
 if(v->ef.mode==S1U_E_REL)release++;
 prior=hz;taps++;
}
static void arm(unsigned owner,unsigned ft,int wet,int amount){
 s1_dry_part*p=parts[owner];CHECK(s1_part_program(p,S1_PART_INIT));
 CHECK(s1c_front_set(&p->e,115,(float)ft));
 CHECK(s1c_front_set(&p->e,18,wet?1.f:0.f));
 CHECK(s1c_front_set(&p->e,19,amount?.67f:0.f));
 CHECK(s1c_front_set(&p->e,139,0.f));
 CHECK(s1c_front_set(&p->e,16,6807.f));
 CHECK(s1c_front_set(&p->e,14,zero_depth?0.f:.96f)&&s1c_front_set(&p->e,46,zero_depth?0.f:.67f));
 CHECK(s1c_front_set(&p->e,15,22.f)&&s1c_front_set(&p->e,47,71.f));
 CHECK(s1c_front_set(&p->e,13,1.f)&&s1c_front_set(&p->e,45,2.f));
 CHECK(s1c_front_set(&p->e,22,.045f)&&s1c_front_set(&p->e,25,.22f));
}
static void take(unsigned owner,unsigned ft,unsigned r,int wet,int amount,float pcm[2][4096]){
 walk(owner,r);
 /* Pure part sampling starts at an independent state with e.c initially
  * zero on smoothed entries. Keep exact Off filter/zero amount controls. */
 arm(owner,ft,wet,amount);
 CHECK(fixture_s1_env(owner,(int)r)==(int)r);
 s1_dry_part*p=parts[owner];
 /* Settle front-end frequency/attack smoothing without an input note so
    an initially zero attack current does not skip the real attack stage. */
 float wl[256],wr[256];for(unsigned i=0;i<32;i++)s1_part_render(p,wl,wr,256);
 CHECK(s1_part_note(p,57,101));
 route=r;ft0=ft;watch=&p->e;taps=offkr=held=attack=release=distinct=0;prior=0.f;
 for(unsigned f=0;f<16;f++){
  if(f==8)CHECK(s1_part_note(p,57,0));
  s1_part_render(p,pcm[0]+256*f,pcm[1]+256*f,256);
 }
 watch=NULL;
 if(r&&amount&&!zero_depth){
  fprintf(stderr,"owner=%u filter=%u route=%u offKR=%u changed=%u attack=%u held=%u release=%u wet=%d\n",owner,ft,r,offkr,distinct,attack,held,release,wet);
  CHECK(taps==4096&&offkr==3584&&held==2048&&attack>500&&release==2048&&distinct>2000);
 } else CHECK(!taps);
}
static float old[2][4096],next[2][4096];
int main(void){
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;select_patch(0);set_view(V_PART);build_pages();
 CHECK(page_named("FILTER MODE")>=0&&pg.r[page_named("FILTER MODE")][3].kind!=R_ENV_TEACH);fresh();
 for(unsigned i=0;i<NBASS;i++){current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 stabilize();
 const char *dump=getenv("S1_ENV_UNTOUCHED");
 if(dump){engine_bass_on(0,60,0,0);engine_bass_on(1,67,0,0);record(reference);
   FILE*f=fopen(dump,"wb");CHECK(f);CHECK(fwrite(reference,sizeof reference,1,f)==1&&!fclose(f));return 0;}
 const sound_t orig=proj.sound;unsigned w=store_writes;
 for(unsigned owner=0;owner<NBASS;owner++)for(unsigned ft=0;ft<3;ft++){
  unsigned peer=1-owner;float untouched=parts[peer]->e.t[56];
  take(owner,ft,0,1,1,old);
  uint32_t hash[3];
  for(unsigned r=1;r<=3;r++){
   take(owner,ft,r,1,1,next);CHECK(memcmp(next,old,sizeof next));
   uint32_t h=2166136261u;const uint8_t *bytes=(const uint8_t*)next;
   for(unsigned i=0;i<sizeof next;i++)h=(h^bytes[i])*16777619u;
   hash[r-1]=h;
  }
  CHECK(hash[0]!=hash[1]&&hash[1]!=hash[2]&&hash[0]!=hash[2]);
  take(owner,ft,0,0,1,old);take(owner,ft,3,0,1,next);CHECK(!memcmp(next,old,sizeof next));
  take(owner,ft,0,1,0,old);take(owner,ft,3,1,0,next);CHECK(!memcmp(next,old,sizeof next));
  zero_depth=1;
  take(owner,ft,0,1,1,old);take(owner,ft,3,1,1,next);CHECK(!memcmp(next,old,sizeof next));
  zero_depth=0;
  CHECK(parts[peer]->e.t[56]==untouched);
 }
 CHECK(!memcmp(&orig,&proj.sound,sizeof orig)&&w==store_writes);
 printf("PASS scoped ID56 actual filter cadence, 3 routes x 3 topologies x 2 owners, wet audio plus filter-off/zero-amount/zero-depth null and teaching K4, no stored writes: %u checks\n",checks);
}
