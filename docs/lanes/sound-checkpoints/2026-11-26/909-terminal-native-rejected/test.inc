#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
enum {JB_909,JB_808,JB_A,JB_B,JB_BRK,JB_N,N=256};
union workspace { terminal_future terminal; float active_terminal_reuse[1280]; };
static union workspace original_workspace;
static drum909_t reference_owner, candidate_owner;
void ref_drum909_init(drum909_t *);
void ref_drum909_trigger(drum909_t *,int,float);
void ref_drum909_render_st_cleared(drum909_t *,float *,float *,float *,float *,int,uint16_t);
static int same909(const drum909_t *a,const drum909_t *b){
 drum909_t temp=*a;
 for(unsigned i=0;i<4;i++){
  assert(a->smp[i].len==b->smp[i].len);
  assert(!memcmp(a->smp[i].adpcm,b->smp[i].adpcm,128));
  temp.smp[i].adpcm=b->smp[i].adpcm;
 }
 return !memcmp(&temp,b,sizeof temp);
}
static drum808_t baseline808,candidate808;
static float a[4][N],b[4][N],a8[4][N],b8[4][N],m8[4][N];
static _Atomic unsigned eng_stage,eng_808_body,eng_c1_next,eng_terminal_claim,eng_terminal_fault;
static unsigned eng_terminal_token,eng_808_split,eng_c1_mask,eng_job_arg,eng_c1_waited,brk_midi_count,eng_brk_mark;
static _Atomic unsigned worker_ready,active_a_count, before_publish, allow_publish;
static unsigned publish_held;
static unsigned stageA_consumed, order[5],count;
static pthread_barrier_t permit;
static jmp_buf trap;
static int fail_mode;
#define __builtin_trap() longjmp(trap,1)
void plat_cpu2_sync(void){atomic_thread_fence(memory_order_seq_cst);}
#define eng_sync() plat_cpu2_sync()
static int plat_cpu2_wait(void){return 0;}
static int stage_wait(unsigned s,int core){
 assert(core);
 if(s==1u)atomic_store(&worker_ready,1);
 while(atomic_load(&eng_stage)<s) {eng_sync();}
 return 0;
}
static void prefix808(drum808_t *d,float buf[4][N]){
 memset(buf,0,sizeof b8);
 drum808_render_st_lanes(d,buf[0],buf[1],buf[2],buf[3],N,0x7ff,D8_LANES_BODY);
}
/* Inserted at the producer's outgoing release boundary, not engine state. */
static void test_pre_publish(void){
 if(publish_held){before_publish=1;while(!allow_publish)eng_sync();}
}
static void d808_body_run(void){prefix808(&candidate808,b8);eng_808_body=1;}
static void finish808(drum808_t *d,float buf[4][N]){
 drum808_metal_prepare(d,m8,N,0x7ff);
 drum808_finish_rest(d,(const float (*)[256])m8,buf[0],buf[1],buf[2],buf[3],N,0x7ff);
}
static void put(unsigned stage){
 assert(count<5&&(!count||order[count-1]==stage-1u));order[count++]=stage;atomic_store(&eng_stage,stage);
}
static int run_job(int j,int core){
 assert(j==JB_A&&(core||fail_mode));
 if(!fail_mode){
  /* Adversarial active A takes every byte as soon as real worker gate allows.
     It cannot do this while C0's terminal consumer can still read it. */
  assert(eng_stage>=1&&stageA_consumed);
  memset(&original_workspace,0x89,sizeof original_workspace);
  atomic_fetch_add(&active_a_count,1);
  while(atomic_load(&eng_stage)<2)eng_sync();
  put(3);
 }else active_a_count++;
 return 1;
}
#include "scheduler.inc"
static void *worker(void *p){
 (void)p;
 pthread_barrier_wait(&permit);
 core2_jobs(0u);
 return 0;
}
static void begin(unsigned token){
 eng_terminal_token=token;eng_terminal_claim=eng_terminal_fault=0;
 eng_c1_next=eng_stage=eng_808_body=worker_ready=0;eng_808_split=1;eng_c1_mask=1u<<JB_A;eng_job_arg=0;
 eng_c1_waited=0;stageA_consumed=count=0;
 drum909_terminal_begin(&original_workspace.terminal,&candidate_owner,token);
}
int main(void){
 ref_drum909_init(&reference_owner);drum909_init(&candidate_owner);
 drum808_init(&baseline808);candidate808=baseline808;
 assert(!pthread_barrier_init(&permit,NULL,2));
 for(unsigned i=0;i<80;i++){
  unsigned ready=i&1;
  publish_held=!(i&1u) && !(i&2u);before_publish=allow_publish=0;
  if(!(i%3)){
   ref_drum909_trigger(&reference_owner,DR_CH,.8f);drum909_trigger(&candidate_owner,DR_CH,.8f);
   ref_drum909_trigger(&reference_owner,DR_RD,.9f);drum909_trigger(&candidate_owner,DR_RD,.9f);
   ref_drum909_trigger(&reference_owner,DR_BD,.7f);drum909_trigger(&candidate_owner,DR_BD,.7f);
   drum808_trigger(&baseline808,D8_CH,.8f);drum808_trigger(&candidate808,D8_CH,.8f);
   drum808_trigger(&baseline808,D8_BD,.8f);drum808_trigger(&candidate808,D8_BD,.8f);
  }
  memset(a,0,sizeof a);memset(b,0,sizeof b);
  ref_drum909_render_st_cleared(&reference_owner,a[0],a[1],a[2],a[3],N,0x7ff);
  prefix808(&baseline808,a8);finish808(&baseline808,a8);
  begin(0x800001feu|(i<<9)); pthread_t w; assert(!pthread_create(&w,NULL,worker,NULL));
  if(ready||publish_held){
    pthread_barrier_wait(&permit);
    if(publish_held){while(!before_publish)eng_sync();}
    else while(!atomic_load(&worker_ready))eng_sync();
    assert(eng_stage==0&&stageA_consumed==0);
  }
  /* For the forced late case, this must finish before the worker is even
     released. An accidental wait for producer here deadlocks/timeout. */
  drum909_render_st_terminal(&candidate_owner,b[0],b[1],b[2],b[3],N,0x7ff,&original_workspace.terminal,eng_terminal_token);
  assert(!memcmp(a,b,sizeof a)&&same909(&reference_owner,&candidate_owner));
  stageA_consumed=1;put(1);
  if(publish_held)allow_publish=1;
  else if(!ready)pthread_barrier_wait(&permit);
  while(!eng_808_body)eng_sync();
  finish808(&candidate808,b8);assert(!memcmp(a8,b8,sizeof a8)&&!memcmp(&baseline808,&candidate808,sizeof baseline808));
  for(unsigned j=0;j<4;j++) for(unsigned k=0;k<N;k++) a[j][k]+=a8[j][k],b[j][k]+=b8[j][k];
  put(2);assert(!pthread_join(w,0));assert(eng_stage==3);put(4);put(5);
  assert(!memcmp(a,b,sizeof a)&&count==5&&eng_terminal_fault==0);
 }
 assert(active_a_count==80);
 fail_mode=1;
 for(unsigned claim=0;claim<3;claim++)for(unsigned stage=2;stage<=3;stage++)for(unsigned body=0;body<=1;body++){
  eng_terminal_token=0x800001feu;eng_c1_mask=1<<JB_A;eng_terminal_claim=claim;eng_stage=stage;eng_808_body=body;
  eng_c1_waited=0;eng_c1_next=JB_A;eng_terminal_fault=0;active_a_count=0;
  if(!setjmp(trap))c1_take_over(6u);
  unsigned should_trap=(claim==1&&!body)||(claim==2&&stage<3);
  assert(eng_terminal_fault==(should_trap?claim:0));
  assert(active_a_count==(!should_trap&&stage<3?1:0));
 }
 puts("80 real moved 909 + 808-prefix owner-state/bus spans; 80 active A scratch reuses after stage1; 40 proven nonwaiting private late (20 paused after real private render before ready release), 12 actual guard cases including completed/no replay");
}
