/* Fresh whitelisted host driver. No guest-memory, device, DSP or writable external API. */
#include "native-build/theme.c"
#include "physical-transport.h"
extern unsigned engine_live_key_probe(int voice,int field);
static uint32_t driver_keys,driver_pads,driver_pad_held;static int pad_key[16];
static const char *fixture_names[]={"909-sequence","909-step1-held","909-step1-release","808-sequence","chicago-sound","chicago-sequence","chicago-generator","miami-sound","juno-sound","drone-sound","break-sequence","home-patterns","song","fx","mixer","mixer-master","settings-index","settings-section","sequence-button-help","performance"};
static int grid_available(void){return ui.view==V_HOME || ui.view==V_SONG || drum_mix_page()>=0 || (ui.btn&(1u<<B_HOME|1u<<B_SAVE)) || !(is_303()&&ui.rec);}
static void driver_apply(void){fm1_in.pnotes=driver_pads;fm1_in.notes=fm1_in.knotes|fm1_in.pnotes;held_keys=fm1_in.notes;for(unsigned k=0;k<NKEYS;k++)key_velocity[k]=fm1_in.key_vel[k+5];}
static int driver_release(void){
 for(unsigned p=0;p<37;p++)smk__key_event(p,0);
 driver_keys=driver_pads=driver_pad_held=held_btn=0;
 for(int i=0;i<16;i++)pad_key[i]=-1;driver_apply();
 /* Actual host pipeline: input queues and 256-frame/44100Hz engine blocks.
  * Advance at least two blocks, then inspect physical-live cleanup only.
  * This is a bounded native-host acknowledgement, not device/audio latency. */
 run_ms(12);
 for(unsigned drain=0;drain<24 && (live_keys[0].n||live_keys[1].n||engine_live_key_probe(0,2)||engine_live_key_probe(1,2));drain++)run_ms(1);
 return !fm1_in.keys&&!driver_pad_held&&!live_keys[0].n&&!live_keys[1].n&&!engine_live_key_probe(0,2)&&!engine_live_key_probe(1,2);
}
static int driver_choose(unsigned voice,unsigned kind,unsigned factory){
 /* The inherited screenshot helper waits only12 audio blocks then exits.
  * Interactive fixture replay must honor real release-tail/preset admission.
  * Advance actual host input/audio, bounded to60s of simulated host time. */
 if(engine_bass_select_preset((int)voice,(int)kind,(int)factory,&proj.sound))return 0;
 for(unsigned wait=0;wait<6000 && engine_bass_preset_pending((int)voice);wait++)run_ms(10);
 return !engine_bass_preset_pending((int)voice)&&engine_bass_status((int)voice)==1&&engine_bass_bank((int)voice)==(int)kind&&engine_bass_preset_index((int)voice)==(int)factory;
}
static int seed(int n){
 if(!driver_release())return 0;ui.help=0;ui.overlay=O_NONE;ui.rec=0;ui.held_step=-1;seq.playing=0;
 if(!driver_choose(0,ORIGINAL_SOUND_CHICAGO,0)||!driver_choose(1,ORIGINAL_SOUND_MIAMI,0))return 0;
 switch(n){
 case 0:case 1:case 2:screen(V_SEQ,PART_909,0);if(n){driver_pads=driver_pad_held=1;pad_key[0]=0;driver_apply();run_ms(350);if(n==2){driver_pads=driver_pad_held=0;pad_key[0]=-1;driver_apply();run_ms(4);}}break;
 case 3:screen(V_SEQ,PART_808,0);break;
 case 4:screen(V_PART,PART_303A,0);break;case 5:screen(V_SEQ,PART_303A,0);break;case 6:screen(V_GEN,PART_303A,0);break;
 case 7:screen(V_PART,PART_303B,0);break;
 case 8:if(!driver_choose(0,ORIGINAL_SOUND_JUNO,0))return 0;screen(V_PART,PART_303A,0);break;
 case 9:if(!driver_choose(0,ORIGINAL_SOUND_DRONE,0))return 0;screen(V_PART,PART_303A,0);break;
 case 10:screen(V_SEQ,PART_BRK,0);break;case 11:screen(V_HOME,PART_909,0);break;case 12:screen(V_SONG,PART_909,0);break;
 case 13:screen(V_FX,PART_909,0);break;case 14:screen(V_MIX,PART_909,0);break;case 15:screen(V_MIX,PART_909,3);break;
 case 16:case 17:screen(V_SEQ,PART_909,0);held_btn=1u<<B_GLO;run_ms(4);held_btn=0;run_ms(4);if(n==17){held_btn=1u<<B_SEL;run_ms(4);held_btn=0;run_ms(4);}break;
 case 18:screen(V_SEQ,PART_909,0);held_btn=1u<<B_SEQ;run_ms(360);break;case 19:screen(V_PERF,PART_909,0);break;
 }
 run_ms(4); /* A real host scan arms source edge baselines after fixture selection. */
 return 1;
}
static void response(void){
 tr_reset();tr_enabled=1;memset(blit_hash,0,sizeof blit_hash);draw_header();draw_main();draw_knobs();leds();tr_enabled=0;
 printf("{\"source\":\"3c41ef4bedf9701f549ce2752b73306e2fab1eb8\",\"authority\":\"actual C native host; not live WASM or device\",\"state\":{\"view\":%u,\"part\":%u,\"page\":%u,\"overlay\":%u,\"rec\":%u,\"playing\":%u,\"keys\":%u,\"buttons\":%u,\"gridPadAvailable\":%s,\"nativeMs\":%u},\"leds\":{\"buttons\":%u,\"keys\":%u,\"dimKeys\":%u},",ui.view,ui.part,cur_page(),ui.overlay,ui.rec,seq.playing,held_keys,held_btn,grid_available()?"true":"false",now_ms,lit_btn,lit_keys,dim_keys);
 printf("\"physical\":{\"held\":%llu,\"irqDepth\":%u,\"padHeld\":%u},\"live\":[",(unsigned long long)fm1_in.keys,irq_depth,driver_pad_held);
 for(int v=0;v<2;v++) {uint64_t owned=0;for(unsigned p=0;p<37;p++)if(live_keys[v].down[p])owned|=(uint64_t)1u<<p;
 printf("%s{\"token\":%u,\"pitch\":%u,\"gate\":%u,\"uiHeld\":%llu,\"uiHeldCount\":%u,\"uiToken\":%u,\"uiPitch\":%u}",v?",":"",engine_live_key_probe(v,0),engine_live_key_probe(v,1),engine_live_key_probe(v,2),(unsigned long long)owned,live_keys[v].n,kbd_held_key[v],kbd_held_note[v]);}printf("],");
 printf("\"break\":{\"page\":%u,\"target\":%u,\"count\":[%d,%d]},",ui.brk_page,ui.brk_count_target,pref_get(PR(R_BRKSLICE,0,0,0)),pref_get(PR(R_BRKSLICE,1,0,0)));
 tr_json_sections();printf(",\"native_rgb565\":\"");for(int i=0;i<57600;i++)printf("%04x",swap16(fb[i]));printf("\"}\n");fflush(stdout);
}
int main(void){
 load_force=0;boot();settle();for(int i=0;i<16;i++)pad_key[i]=-1;if(!seed(0)){fputs("native physical cleanup failed at startup\n",stderr);return 1;}
 char line[120],op[24],tail;int a,b,n;
 while(fgets(line,sizeof line,stdin)){
  a=b=0;n=sscanf(line,"%23s %d %d %c",op,&a,&b,&tail);int valid=0;
  if(n<1){puts("{\"error\":\"empty input\"}");fflush(stdout);continue;}
  if(!strcmp(op,"snapshot")&&n==1)valid=1;
  else if(!strcmp(op,"release")&&n==1){if(!driver_release()){puts("{\"error\":\"native physical cleanup did not finish within bounded host drain\"}");fflush(stdout);continue;}valid=1;}
  else if(!strcmp(op,"fixture")&&n==2&&a>=0&&a<20){if(!seed(a)){puts("{\"error\":\"native fixture cleanup or preset admission did not finish within bounded host wait\"}");fflush(stdout);continue;}valid=1;}
  else if(!strcmp(op,"tick")&&n==2&&a>=0&&a<=500){run_ms(a);valid=1;}
  else if(!strcmp(op,"button")&&n==3&&a>=0&&a<NB&&(b==0||b==1)){if(b)held_btn|=1u<<a;else held_btn&=~(1u<<a);run_ms(4);valid=1;}
  else if(!strcmp(op,"key")&&n==3&&a>=0&&a<NKEYS&&(b==0||b==1)){smk__key_event((unsigned)a+5,b?100:0);driver_apply();run_ms(4);valid=1;}
  else if(!strcmp(op,"physical")&&n==3&&a>=0&&a<37&&b>=0&&b<=127){smk__key_event((unsigned)a,(unsigned)b);driver_apply();run_ms(8);valid=1;}
  else if(!strcmp(op,"pad")&&n==3&&a>=0&&a<16&&(b==0||b==1)){
   if(!b){driver_pad_held&=~(1u<<a);if(pad_key[a]>=0)driver_pads&=~(1u<<pad_key[a]);pad_key[a]=-1;valid=1;}
   else if(grid_available()){driver_pad_held|=1u<<a;if(pad_key[a]<0){pad_key[a]=WHITE_K[a];driver_pads|=1u<<pad_key[a];}valid=1;}
   if(valid){driver_apply();run_ms(4);}
  }
  else if(!strcmp(op,"encoder")&&n==3&&a>=0&&a<NE&&b>=-8&&b<=8){enc_acc[a]+=b;run_ms(4);valid=1;}
  if(valid)response();else{puts("{\"error\":\"unsupported input or source-host mode\"}");fflush(stdout);}
 }
 driver_release();tr_reset();return 0;
}
