#include "firmware/src/app/original_runtime.h"
#include "experiments/original_synth/chicago_fx2_compact.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define NBASS 2
static original_runtime_part original_bass[NBASS];
static original_chicago_fx2_compact original_fx[NBASS];
static unsigned original_fx_ready=1,project_hold,reset_hits,locked;
static unsigned plat_cq_lock(void){assert(!locked); return ++locked;}
static void plat_cq_unlock(unsigned i){assert(locked==i); --locked;}

/* Test the reset call and exactly which owner was passed, not the real reset. */
int original_chicago_fx2_compact_init(original_chicago_fx2_compact *p){
 assert(p==&original_fx[0]); ++reset_hits; memset(p,0,sizeof *p); return 1;
}
int original_chicago_backend_prepare(original_chicago_backend *x){(void)x;abort();}
int original_chicago_backend_tempo(original_chicago_backend *x,float b){(void)x;(void)b;abort();}
int original_miami_backend_prepare(original_miami_backend *x){(void)x;abort();}
int original_miami_backend_set_tempo(original_miami_backend *x,float b){(void)x;(void)b;abort();}
int original_chicago_backend_retire(original_chicago_backend *x){(void)x;abort();}
#include "firmware/src/app/lab_delay_engine.inc"
/* Non-Juno paths are deliberately not invoked. No active voices in this
 * history-only All Off witness; synth-voice teardown is tested elsewhere. */
int original_chicago_backend_all_off(original_chicago_backend *x){(void)x;abort();}
int original_miami_backend_all_off(original_miami_backend *x){(void)x;abort();}
void s1_part_all_off(s1_dry_part *x){(void)x;abort();}
void juno6_all_off(juno6_t *x){assert(x==&original_bass[0].dsp.juno.synth);}
void juno6_delay24_process(juno6_t *,float *,float *,unsigned);
static void juno_fixture_init(void){
 original_runtime_part *p=&original_bass[0]; memset(p,0,sizeof *p);
 p->kind=ORIGINAL_SOUND_JUNO; p->status=ORIGINAL_RUNTIME_OK;
 p->dsp.juno.synth.typed=1; p->dsp.juno.synth.sample_rate=44100.f;
}
int main(void){
 const uint8_t q[]={3,42,63,64};
 memset(&original_fx[1],99,sizeof original_fx[1]);
 original_bass[1].kind=ORIGINAL_SOUND_CHICAGO;
 original_bass[1].status=ORIGINAL_RUNTIME_OK;
 juno_fixture_init();
 assert(engine_lab_delay_epoch(0)==0 && !engine_lab_delay_publish(0,0,1,q,4));
 assert(lab_delay_owner_retire(0)==ORIGINAL_RUNTIME_OK);
 juno_fixture_init();
 assert(engine_lab_delay_epoch(0)==0);
 lab_delay_bind_current();
 unsigned e=engine_lab_delay_epoch(0);
 assert(e==1 && !engine_lab_delay_publish(1,e,1,q,4));
 assert(engine_lab_delay_publish(0,e,1,q,4));
 assert(!original_bass[0].dsp.juno.synth.delay24.tap);
 assert(original_runtime_prepare(&original_bass[0],120)==ORIGINAL_RUNTIME_OK);
 assert(original_bass[0].dsp.juno.synth.delay24.tap==original_fx[0].arena.history);
 assert(original_runtime_delay_result(&original_bass[0])==2);
 juno6_t *synth=&original_bass[0].dsp.juno.synth;
 memset(original_fx[0].arena.history,0x51,4096);
 synth->delay24.filled=1024; synth->delay24.pos=64;
 synth->delay24.sum[0]=0.5f; synth->delay24.sum[1]=-0.5f;
 assert(original_runtime_all_off(&original_bass[0])==ORIGINAL_RUNTIME_OK);
 assert(synth->delay24.tap==original_fx[0].arena.history && !synth->delay24.filled);
 assert(!memcmp(original_bass[0].dsp.juno.lab_delay.applied,q,4));
 float a[256]={0},b[256]={0};
 juno6_delay24_process(synth,a,b,256);
 for(unsigned i=0;i<256;++i) assert(a[i]==0.f && b[i]==0.f);

 assert(engine_lab_delay_publish(0,e,1,q,4));
 assert(lab_delay_owner_retire(0)==ORIGINAL_RUNTIME_OK && reset_hits==1);
 juno_fixture_init(); lab_delay_bind_current();
 assert(!original_bass[0].dsp.juno.synth.delay24.tap);
 assert(engine_lab_delay_epoch(0)==e+1 && !engine_lab_delay_publish(0,e,1,q,4));
 project_hold=1; assert(!engine_lab_delay_publish(0,e+1,1,q,4)); project_hold=0;
 lab_delay_epoch[0]=UINT32_MAX-1;
 assert(lab_delay_owner_retire(0)==ORIGINAL_RUNTIME_OK); juno_fixture_init();
 assert(engine_lab_delay_epoch(0)==0 && !engine_lab_delay_publish(0,UINT32_MAX,1,q,4));
 for(unsigned i=0;i<sizeof original_fx[1];i++) assert(((const unsigned char *)&original_fx[1])[i]==99);
 printf("actual owner binding, direct runtime publish, All Off history safety, retire; alternate Chicago %zu bytes unchanged\n",sizeof original_fx[1]);
}
