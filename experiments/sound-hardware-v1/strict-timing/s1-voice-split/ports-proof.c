#define X0X_IMA_TABLES_DEFINE 1
#include "s1c.c"
#include "compact-ports.inc"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stddef.h>
static s1c reference,split;
static s1_voice_view views[2];
enum { TILE=16 };
typedef struct { unsigned n,active; float prefix[TILE], suffix[TILE][3]; } tile;
static tile z;
static s1_voice_port_span plan;
static s1_voice_port_view plan_view;static unsigned spans;
static unsigned long long updates;static unsigned max_active,min_active=46;

static uint32_t rng=0x8672131aU;
static uint32_t roll(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static void front(s1c *s){
 float c2=s->port_c2,c1=1.f-c2;
 for(unsigned j=0;j<sizeof s1c_smid;j++)s->c[j]=c1*s->t[s1c_smid[j]]+c2*s->c[j];
 s->monos=(1.f-s->glide_c2)*s->monof+s->glide_c2*s->monos;
 s1u_lfo_tick(&s->lfo[0],P(L1R),(int)P(L1I),P(L1A),s->lfo_c2,s->sr);
 s1u_lfo_tick(&s->lfo[1],P(L2R),(int)P(L2I),P(L2A),s->lfo_c2,s->sr);
}
static void *worker(void *arg){
 unsigned k=*(unsigned*)arg;
 s1_voice_tile(&views[k],split.v,z.active,z.n,3,k,k?&z.suffix[0][0]:z.prefix,&plan_view);
 return NULL;
}
static unsigned active(const s1c *s){unsigned m=0;for(unsigned v=0;v<S1C_VOICES;v++)if(s->v[v].root!=-1&&s->v[v].stage)m|=1u<<v;return m;}
static uint32_t h=2166136261u;
static void hash(const void*value,size_t n){const unsigned char*p=value; for(size_t i=0;i<n;i++)h=(h^p[i])*16777619u;}
int main(void){
 uint64_t frames=0;unsigned active_samples=0,panels=0;
 for(unsigned run=0;run<9;run++){
  s1c_init(&reference,44100,0);
  s1c *s=&reference;
  const float c=run%3; const unsigned mode[] = {0,1,2};
  s1c_set(s,MONO,0.f);s1c_set(s,FTYPE,(float)mode[run%3]);s1c_set(s,NOISEV,run%2?.29f:0.f);
  s1c_set(s,I1,.18f);s1c_set(s,I2,.73f);s1c_set(s,SUBV,.50f);s1c_set(s,LPITCH,1.f);s1c_set(s,L1A,.76f);
  s1c_set(s,SEMI2,7);s1c_set(s,FMV,.28f);s1c_set(s,LFENV,1);s1c_set(s,FENV,c*.28f);
  split=reference;
  for(unsigned key=0;key<6;key++){s1c_start(&reference,36+4*key,86+key);s1c_start(&split,36+4*key,86+key);}
  for(unsigned blk=0;blk<384;blk++){
    unsigned n=1u+roll()%TILE;float out[TILE];
    if(blk%7==0){s1c_stop(&reference,40);s1c_stop(&split,40);}
    if(blk%11==0){s1c_start(&reference,40,112);s1c_start(&split,40,112);}
    if(blk%13==0){float q=run==0?0.f:(roll()&127u)/127.f;s1c_set(&reference,NOISEV,q);s1c_set(&split,NOISEV,q);panels++;}
    if(blk%17==0){float q=64.f+(roll()&4095u);s1c_set(&reference,CUT,q);s1c_set(&split,CUT,q);panels++;}
    z.n=n;z.active=active(&split);
    assert(z.active==active(&reference));
    for(unsigned f=0;f<n;f++){
       front(&reference);float y=0.f;
       for(unsigned v=0;v<S1C_VOICES;v++)if(z.active&(1u<<v))y+=voice_run_kr(&reference,&reference.v[v]);
       out[f]=y;assert(isfinite(y));
    }
    active_samples+=n*__builtin_popcount(z.active);
    s1_voice_capture(&views[0],&split);views[1]=views[0];
    if(blk%4==0||blk%13==0||blk%17==0||blk%7==0||blk%11==0){
       s1_voice_port_plan(&plan,&views[0],0,0);spans++;
       plan_view=(s1_voice_port_view){plan.target,plan.slot,plan.count};
    }
    updates+=(unsigned)n*plan.count;
    if(plan.count>max_active)max_active=plan.count;if(plan.count<min_active)min_active=plan.count;
    unsigned ways[]={0,1};pthread_t peer;
    assert(!pthread_create(&peer,NULL,worker,&ways[0]));worker(&ways[1]);assert(!pthread_join(peer,NULL));
    assert(!memcmp(views[0].c,views[1].c,sizeof split.c));
    assert(!memcmp(views[0].lfo,views[1].lfo,sizeof split.lfo));
    assert(views[0].rnd==views[1].rnd);
    memcpy(split.c,views[1].c,sizeof split.c);memcpy(split.lfo,views[1].lfo,sizeof split.lfo);split.rnd=views[1].rnd;
    for(unsigned f=0;f<n;f++)split.monos=(1.f-split.glide_c2)*split.monof+split.glide_c2*split.monos;
    for(unsigned f=0;f<n;f++){
       float y=z.prefix[f];
       for(unsigned v=3;v<S1C_VOICES;v++)if(z.active&(1u<<v))y+=z.suffix[f][v-3];
       assert(!memcmp(&out[f],&y,4));hash(&y,sizeof y);
    }
    assert(!memcmp(&reference,&split,sizeof split));
    frames+=n;
  }
 }
 printf("exact %llu frames, %u voice samples, %u live edits, PCM %08x; shared returned prefix PCM=%zu, full test tile=%zu\n",(unsigned long long)frames,active_samples,panels,h,sizeof(z.prefix),sizeof(tile));
 printf("active lane min/max=%u/%u; per-side sparse/full updates=%llu/%llu; header=%zu view=%zu spans=%u\n",min_active,max_active,updates,frames*46,sizeof plan,sizeof(s1_voice_view),spans);
}
