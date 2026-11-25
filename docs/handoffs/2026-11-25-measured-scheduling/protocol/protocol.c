#include "protocol.h"
#ifndef DNBX_MODEL_SYNC
#if defined(DNBX_PI32)
#define DNBX_MODEL_SYNC() __asm__ volatile("csync" ::: "memory")
#else
#define DNBX_MODEL_SYNC() __asm__ volatile("" ::: "memory")
#endif
#endif
#define TAG(n,p) (((n) << 2) | (p))
#define PAYLOAD 0x1ffffu
#define STAMP (255u << 17)
#define VALID (1u << 25)
struct job dnbx_demo_jobs[JOBS];
_Static_assert(sizeof(struct job)==8 && sizeof dnbx_demo_jobs==40,"fixed model footprint");
/* Single designated worker, main never claims a worker's slot while it is
 * runnable. Publish STARTED before the first mutable engine operation. */
int dnbx_demo_start(struct job *j,uint32_t n)
{
    if(j->tag!=TAG(n,QUEUED)) return 0;
    j->tag=TAG(n,STARTED);
    DNBX_MODEL_SYNC();
    return 1;
}
uint32_t dnbx_demo_event(uint32_t a,uint32_t part,uint32_t note,uint32_t velocity)
{
    return VALID | ((a+(1u<<17))&STAMP) | (velocity<<10) | (note<<3) | part;
}
int dnbx_demo_finish(struct job *j,uint32_t n,uint32_t result)
{
    if(j->tag!=TAG(n,STARTED)) return 0;
    j->voice=result;
    DNBX_MODEL_SYNC();
    j->tag=TAG(n,DONE);
    return 1;
}
enum mode dnbx_demo_classify(const struct job *j,uint32_t n,int stopped)
{
    if(stopped) DNBX_MODEL_SYNC();
    uint32_t tag=j->tag;
    if((tag>>2)!=n) return STALE_SLOT;
    if((tag&3u)==DONE) return USE_RESULT;
    if((tag&3u)==QUEUED && stopped) return LOCAL_FALLBACK;
    return DEADLINE_UNCERTAIN;
}
/* Merge only in the original synchronous mix order. It retains last payload
 * even when an exact multiple of 256 events makes the stamp delta zero. */
uint32_t dnbx_demo_join_voice(uint32_t *last_committed, uint32_t previous,unsigned captured,unsigned current,
                             const struct job jobs[JOBS],uint32_t n,unsigned expected)
{
    if(n<=*last_committed) return previous;
    for(unsigned i=0;i<JOBS;++i)
        if ((expected>>i&1u) && jobs[i].tag!=TAG(n,DONE)) return previous;
    *last_committed=n;
    if(!captured || current!=captured) return previous;
    for(unsigned i=0;i<JOBS;++i){
        if(!(expected>>i&1u)) continue;
        DNBX_MODEL_SYNC();
        uint32_t a=jobs[i].voice;
        if(a&VALID) previous=(captured<<25) | (((previous&STAMP)+(a&STAMP))&STAMP) | (a&PAYLOAD);
    }
    return previous;
}
