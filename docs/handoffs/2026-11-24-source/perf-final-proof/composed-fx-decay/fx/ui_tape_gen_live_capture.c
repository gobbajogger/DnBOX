#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"

static FILE *pcm_out;
static uint64_t pcm=UINT64_C(14695981039346656037);
static void blocks(unsigned n){
 for(unsigned i=0;i<n;i++){
  int32_t d[512]; engine_render(d,256);
  CHECK(fwrite(d,sizeof d,1,pcm_out)==1);
  for(unsigned j=0;j<512;j++){ pcm^=(uint32_t)d[j]; pcm*=UINT64_C(1099511628211); }
  now_ms+=6;
 }
}
static void shot_named(const char *group,int theme,const char *state){
 char name[80];snprintf(name,sizeof name,"%s-%d-%s",group,theme,state);ui_frame();shot(name);
}
static void hit(void){
 engine_drum(0,0,1.f);blocks(4);
 CHECK(engine_tape_active());
}
static void input_button(int b){
 now_ms+=200;held_btn=1u<<b;input();
 now_ms+=40;held_btn=0;input();ui_frame();
}
static void set_fx(int address,int v){
 pref_set(PR(R_ENG,T_FX,0,address),v);
}
static void fx(int theme){
 setup(V_HOME,PART_909);palette_set(theme?5:0);
 input_button(B_FX);CHECK(ui.view==V_FX);
 input_button(B_FX);CHECK(cur_page()==1);
 set_fx(FX_DL_TIME,6); set_fx(FX_DL_FDBK,96);set_fx(FX_RV_DECAY,37);
 pref_t taperef=pg.r[1][3];CHECK(taperef.kind==R_TAPE);
 pref_set(taperef,15);blocks(2);CHECK(!engine_tape_active());
 CHECK(pref_get(taperef)==15 && proj.sound.v[T_FX][0][FX_DL_TYPE]==0);
 shot_named("fx",theme,"digi");
 now_ms+=180;enc_acc[EN_K4]=1;input();
 CHECK(pref_get(taperef)==16 && proj.sound.v[T_FX][0][FX_DL_TYPE]==1);
 blocks(2);CHECK(!engine_tape_active());
 now_ms+=1000;shot_named("fx",theme,"tape-idle");
 unsigned tape_phase=tape_glyph_phase;
 now_ms+=170;ui_frame();CHECK(tape_glyph_phase==tape_phase);
 pref_set(PR(R_ENG,T_MIX,PART_909,MX_DLY),127);
 pref_set(find_ref(T_909,0,"Dly"),127);
 set_fx(FX_DL_LEVEL,96);
 hit();shot_named("fx",theme,"tape-live-a");
 tape_phase=tape_glyph_phase;now_ms+=100;
 hit();shot_named("fx",theme,"tape-live-b");CHECK(tape_glyph_phase!=tape_phase);
 pref_set(taperef,127);
 set_fx(FX_DL_TIME,11);set_fx(FX_DL_FDBK,61);set_fx(FX_RV_DECAY,88);
 hit();shot_named("fx",theme,"tape-high");
 char n[16],u[8];pref_value(PR(R_ENG,T_FX,0,FX_DL_TIME),n,u);
 printf("PASS FX palette %d actual delay division %s wear %u phase %u\n",theme,n,proj.sound.v[T_FX][0][FX_DL_WEAR],tape_glyph_phase);
 input_button(B_FX);CHECK(cur_page()==0);
 shot_named("fx",theme,"reverb");
}
static void gen(int owner){
 setup(V_SEQ,PART_303A+owner);proj.set.bt|=PROJ_BT_CLK_IN;
 int bank=owner?ORIGINAL_SOUND_MIAMI:ORIGINAL_SOUND_CHICAGO;
 CHECK(!engine_bass_select_preset(owner,bank,0,&proj.sound));
 for(int n=0;n<1024&&engine_bass_preset_pending(owner);n++)blocks(1);
 CHECK(engine_bass_bank(owner)==bank);
 bpart_t *line=&cur_pat()->bass[owner];line->len=16;
 for(unsigned j=0;j<16;j++){
   line->step[j]=(bstep_t){.note=(uint8_t)(48+(j*5)%13),.flags=(j%3==0?G_NOTE:G_REST),.velocity=96};
 }
 bpart_t saved=*line;
 input_button(B_ARP);CHECK(ui.view==V_GEN && !strcmp(pg.title[cur_page()],"GENERATE"));
 shot_named("gen",owner,"generate-idle");
 input_button(B_PLAY);blocks(1);CHECK(seq.playing);
 blocks(25);shot_named("gen",owner,"generate-play-a");
 blocks(42);shot_named("gen",owner,"generate-play-b");
 input_button(B_ARP);CHECK(!strcmp(pg.title[cur_page()],"SCALE"));
 blocks(8);shot_named("gen",owner,"scale-playing");
 CHECK(!memcmp(line,&saved,sizeof saved));
 printf("PASS GEN owner %d actual bank %d, legitimate playback grid and SCALE\n",owner,bank);
}
int main(void){
 char path[1024];
 snprintf(path,sizeof path,"%s/fixture.pcm",getenv("SMC_UI_SHOTS"));
 pcm_out=fopen(path,"wb");CHECK(pcm_out);
 fx(0);fx(1);gen(0);gen(1);
 CHECK(!fclose(pcm_out));
 printf("PCM %016llx\n",(unsigned long long)pcm);
}
