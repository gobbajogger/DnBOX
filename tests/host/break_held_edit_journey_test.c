/* Observe UI enqueue values while forwarding every call to the real engine. */
static int last_break_pitch;
#define engine_set captured_engine_set
#define SMC_UI_MAIN inherited_fixture
#include "smc_ui_test.c"
#undef engine_set
void engine_set(int target,int voice,int index,int value);
void captured_engine_set(int target,int voice,int index,int value){
 if(target==T_BRK&&voice==0&&index==1)last_break_pitch=value;
 engine_set(target,voice,index,value);
}
static void tap_button(int b){held_btn=1u<<b;++now_ms;input();held_btn=0;++now_ms;input();}
int main(void){for(int route=0;route<5;route++){
 setup(V_SEQ,PART_BRK);cur_pat()->brk.steps=0xffff;
 for(int n=0;n<16;n++)audio_block();
 held_keys=1u<<WHITE_KEY[10];++now_ms;input();CHECK(ui.brk_held==10);
 tap_button(B_EDIT);CHECK(ui.view==V_PART);CHECK(lock_step()==10);
 pref_t pitch=PR(R_ENG,T_BRK,0,1);int before=proj.sound.v[T_BRK][0][1];last_break_pitch=-1;
 CHECK(pg.r[cur_page()][3].kind==pitch.kind && pg.r[cur_page()][3].a==pitch.a && pg.r[cur_page()][3].c==pitch.c);
 enc_acc[EN_K4]=1;++now_ms;input();CHECK(lock_of(pitch,10)>=0);CHECK(proj.sound.v[T_BRK][0][1]==before);
 CHECK(last_break_pitch==lock_of(pitch,10));CHECK(last_break_pitch!=before);audio_block();
 now_ms+=STEP_TAP_MS+1;
 if(route==1){tap_button(B_HOME);CHECK(ui.view==V_HOME);CHECK(ui.brk_held<0);}
 if(route==2){enc_acc[EN_ALGO]=-1;++now_ms;input();CHECK(ui.part!=PART_BRK);CHECK(ui.brk_held<0);}
 if(route==3){tap_button(B_SEQ);CHECK(ui.view==V_PART);CHECK(lock_step()==10);}
 if(route==4){smk_clip_enter_cleanup();CHECK(ui.brk_held<0);}
 held_keys=0;++now_ms;input();CHECK(lock_step()<0);audio_block();
 CHECK(last_break_pitch==before);CHECK(proj.sound.v[T_BRK][0][1]==before);
 CHECK((proj.pat[0].brk.steps&(1u<<10))!=0);
 if(route==3){tap_button(B_SEQ);CHECK(ui.view==V_SEQ);}
 }
 /* Untouched long hold is not a tap-delete. */
 setup(V_SEQ,PART_BRK);cur_pat()->brk.steps=1u<<10;
 held_keys=1u<<WHITE_KEY[10];++now_ms;input();tap_button(B_EDIT);
 now_ms+=STEP_TAP_MS+1;held_keys=0;input();CHECK(cur_pat()->brk.steps==(1u<<10));
 /* A genuine stopped cue changes the pattern: the old key cannot lock or erase it. */
 setup(V_SEQ,PART_BRK);proj.pat[0].brk.steps=proj.pat[1].brk.steps=1u<<10;
 held_keys=1u<<WHITE_KEY[10];++now_ms;input();tap_button(B_EDIT);
 seq_cue_part(&seq,PART_BRK,1);for(int n=0;n<16 && seq.ppat[PART_BRK]!=1;n++)audio_block();CHECK(seq.ppat[PART_BRK]==1);
 ++now_ms;input();CHECK(ui.brk_held<0 && lock_step()<0);
 held_keys=0;++now_ms;input();CHECK(proj.pat[0].brk.steps==(1u<<10));CHECK(proj.pat[1].brk.steps==(1u<<10));
 /* Cue adoption after input's entry guard: dispatch key-up directly before another input pass. */
 setup(V_SEQ,PART_BRK);proj.pat[0].brk.steps=proj.pat[1].brk.steps=1u<<10;
 held_keys=1u<<WHITE_KEY[10];++now_ms;input();
 seq_cue_part(&seq,PART_BRK,1);for(int n=0;n<16 && seq.ppat[PART_BRK]!=1;n++)audio_block();CHECK(seq.ppat[PART_BRK]==1);
 key_event(WHITE_KEY[10],0);CHECK(ui.brk_held<0);
 CHECK(proj.pat[0].brk.steps==(1u<<10));CHECK(proj.pat[1].brk.steps==(1u<<10));held_keys=0;
 /* A queued playing cue adopts a different pattern while the physical key stays down. */
 setup(V_SEQ,PART_BRK);proj.pat[0].brk.steps=proj.pat[1].brk.steps=1u<<10;
 held_keys=1u<<WHITE_KEY[10];++now_ms;input();tap_button(B_EDIT);
 seq_start(&seq);audio_block();seq_cue_part(&seq,PART_BRK,1);
 for(int blocks=0;blocks<4096 && seq.ppat[PART_BRK]!=1;blocks++)audio_block();
 CHECK(seq.ppat[PART_BRK]==1);++now_ms;input();CHECK(ui.brk_held<0 && lock_step()<0);
 held_keys=0;++now_ms;input();CHECK(proj.pat[0].brk.steps==(1u<<10));CHECK(proj.pat[1].brk.steps==(1u<<10));seq_stop(&seq);
 /* Playback owns restoration: key-up must not enqueue a stopped base restore. */
 setup(V_SEQ,PART_BRK);cur_pat()->brk.steps=1u<<10;
 held_keys=1u<<WHITE_KEY[10];++now_ms;input();tap_button(B_EDIT);
 enc_acc[EN_K4]=1;++now_ms;input();CHECK(ui.hold_used);
 seq_start(&seq);audio_block();CHECK(seq.playing);last_break_pitch=-1;now_ms+=STEP_TAP_MS+1;held_keys=0;input();
 CHECK(lock_step()<0);CHECK(last_break_pitch==-1);seq_stop(&seq);
 /* Full pool refusal cannot edit the base or accidentally delete the held hit. */
 setup(V_SEQ,PART_BRK);cur_pat()->brk.steps=1u<<10;
 for(int n=0;n<NLANE;n++)CHECK(motion_alloc(proj.arr.lane,n%NPAT,PART_909,T_909,n/NPAT,0)>=0);
 int base=proj.sound.v[T_BRK][0][1];held_keys=1u<<WHITE_KEY[10];++now_ms;input();tap_button(B_EDIT);
 enc_acc[EN_K4]=1;++now_ms;input();CHECK(motion_count(proj.arr.lane)==NLANE);CHECK(proj.sound.v[T_BRK][0][1]==base);CHECK(strstr(ui.msg,"MOTION FULL"));
 now_ms+=STEP_TAP_MS+1;held_keys=0;input();CHECK(cur_pat()->brk.steps==(1u<<10));
 /* SAVE clear scans the full lane and preserves other steps; last-value clear frees it. */
 setup(V_SEQ,PART_BRK);cur_pat()->brk.steps=1u<<10;
 held_keys=1u<<WHITE_KEY[10];++now_ms;input();tap_button(B_EDIT);enc_acc[EN_K4]=1;input();
 int lane=motion_find(proj.arr.lane,0,PART_BRK,T_BRK,0,1);CHECK(lane>=0);
 for(int n=0;n<NSTEPS;n++)proj.arr.lane[lane].val[n]=13;
 held_btn=1u<<B_SAVE;enc_acc[EN_K4]=1;++now_ms;input();CHECK(proj.arr.lane[lane].val[10]==MOT_NONE);CHECK(proj.arr.lane[lane].val[NSTEPS-1]==13);CHECK(strstr(ui.msg,"P-LOCK CLEARED"));
 for(int n=0;n<NSTEPS;n++)proj.arr.lane[lane].val[n]=MOT_NONE;
 proj.arr.lane[lane].val[10]=13;enc_acc[EN_K4]=1;input();audio_block();CHECK(!proj.arr.lane[lane].used);
 held_btn=held_keys=0;now_ms+=STEP_TAP_MS+1;input();CHECK(lock_step()<0);
 /* Real held SELECT chance edits and100% restoration; stale dispatch cannot index-1. */
 setup(V_SEQ,PART_BRK);proj.pat[0].brk.steps=proj.pat[1].brk.steps=1u<<10;
 held_keys=1u<<WHITE_KEY[10];++now_ms;input();
 enc_acc[EN_SELECT]=-1;++now_ms;input();
 int prob=motion_find(proj.arr.lane,0,PART_BRK,T_PROB,PART_BRK<<4,0);CHECK(prob>=0);CHECK(proj.arr.lane[prob].val[10]==95);
 enc_acc[EN_SELECT]=1;++now_ms;input();audio_block();CHECK(!proj.arr.lane[prob].used);
 int nextprob=motion_alloc(proj.arr.lane,1,PART_BRK,T_PROB,PART_BRK<<4,0);CHECK(nextprob>=0);proj.arr.lane[nextprob].val[10]=50;
 seq_cue_part(&seq,PART_BRK,1);for(int n=0;n<16&&seq.ppat[PART_BRK]!=1;n++)audio_block();CHECK(seq.ppat[PART_BRK]==1);
 static lane_t before_lanes[NLANE];memcpy(before_lanes,proj.arr.lane,sizeof before_lanes);
 prob_turn(-1);CHECK(!memcmp(before_lanes,proj.arr.lane,sizeof before_lanes));
 key_event(WHITE_KEY[10],0);CHECK(proj.pat[0].brk.steps==(1u<<10));CHECK(proj.pat[1].brk.steps==(1u<<10));held_keys=0;
 puts("BREAK SEQ hold EDIT lock release/navigation journey PASS");return 0;
}
