#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
enum {CASES=256};
static drum909_t refalt, live;
static float a[4][256],b[4][256];
static terminal_future f;
static unsigned n,token;
static pthread_barrier_t go, done;
static void *worker(void*unused){
 (void)unused;
 for(unsigned i=0;i<CASES;i++){
  pthread_barrier_wait(&go);terminal_create(&f,n,token);pthread_barrier_wait(&done);
 }
 return NULL;
}
int main(void){
 drum909_init(&refalt);
 live=refalt;
 pthread_t w;assert(!pthread_barrier_init(&go,NULL,2));assert(!pthread_barrier_init(&done,NULL,2));
 assert(!pthread_create(&w,NULL,worker,NULL));unsigned samples=0,ready_hits=0,empty_hits=0,stale_hits=0;
 for(unsigned i=0;i<CASES;i++){
  static const unsigned sizes[]={256,31,128,1,17,229,64,79};
  n=sizes[i%8];token=terminal_token(0x7ffff0u+i,n);
  for(unsigned j=0;j<DR_NUM;j++){
   float pan=(i%5==0)?1.f:.07f+j*.09f;
   refalt.pan_l[j]=live.pan_l[j]=pan;refalt.pan_r[j]=live.pan_r[j]=1.2f-pan;
   refalt.send_dly[j]=live.send_dly[j]=j*.066f;refalt.send_rev[j]=live.send_rev[j]=j*.034f;
   if(i%7==0||(i+j*11)%31==0){drum909_trigger(&live,j,.54f+(j%4)*.12f);drum909_trigger(&refalt,j,.54f+(j%4)*.12f);}
  }
  unsigned mask=(i%3)?0x7ffu:(i&1)?0x5a5u:0;
  memset(a,0,sizeof a);memset(b,0,sizeof b);
  drum909_render_st_cleared(&refalt,a[0],a[1],a[2],a[3],n,mask);
  terminal_begin(&f,&live,token);
  if(i&1){
   pthread_barrier_wait(&go);pthread_barrier_wait(&done);
   assert(atomic_load(&f.ready)==(token|1u));ready_hits++;
   d9_render_st_masked(&live,b[0],b[1],b[2],b[3],n,mask,1,&f,token);
  }else{
   /* Deterministic late worker: central fallback completes before the copied
      worker; it still sees only its assigned immutable preimage and private out. */
   assert(atomic_load(&f.ready)==token);empty_hits++;
   if(i%4==0){
     /* Inject a stale ready tag, deliberately mismatch generation or span. */
     atomic_store(&f.ready,(token^(i&4?2u:512u))|1u);stale_hits++;
   }
   d9_render_st_masked(&live,b[0],b[1],b[2],b[3],n,mask,1,&f,token);
   pthread_barrier_wait(&go);pthread_barrier_wait(&done);
  }
  assert(!memcmp(a,b,sizeof a));
  if(memcmp(&refalt,&live,sizeof live)){
    fprintf(stderr,"owner state delta span=%u\n",i);return 2;
  }
  /* C0 retired and C1 joined. Simulate completely overwriting A's union
     after release: neither old pointer nor old frame tag can be consumed. */
  memset(f.voices,0xA5,sizeof f.voices);memset(f.mono,0xA5,sizeof f.mono);
  atomic_store(&f.ready,(token^512u)|1);
  samples+=n;
 }
 assert(!pthread_join(w,NULL));
 printf("%u exact spans, %u frames, %u ready and %u forced-private-late (%u intentionally stale stamps); %zu raw owner bytes and %zu four-bus bytes compared\n",CASES,samples,ready_hits,empty_hits,stale_hits,CASES*sizeof live,CASES*sizeof a);
}
