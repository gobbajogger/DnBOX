#ifndef DNBOX_DEMO_JOB_PROTOCOL_H
#define DNBOX_DEMO_JOB_PROTOCOL_H
#include <stdint.h>
enum owner_job { JOB_909, JOB_808, JOB_A, JOB_B, JOB_BREAK, JOBS };
enum job_state { QUEUED=1, STARTED, DONE };
enum mode { STALE_SLOT, DEADLINE_UNCERTAIN, LOCAL_FALLBACK, USE_RESULT };
struct job { volatile uint32_t tag, voice; };
extern struct job dnbx_demo_jobs[JOBS];
int dnbx_demo_start(struct job *,uint32_t);
uint32_t dnbx_demo_event(uint32_t,uint32_t,uint32_t,uint32_t);
int dnbx_demo_finish(struct job *,uint32_t,uint32_t);
enum mode dnbx_demo_classify(const struct job *,uint32_t,int);
uint32_t dnbx_demo_join_voice(uint32_t *,uint32_t,unsigned,unsigned,const struct job[JOBS],uint32_t,unsigned);
#endif
