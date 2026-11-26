/* SPDX-License-Identifier: GPL-3.0-only */
#include "join.h"
#include <assert.h>
#include <stdio.h>
#include <threads.h>
enum { RUNS=25000 };
static join_word request,reply,retired,drained;
static uint32_t epoch,payload,writes;
static void render_only(void *ignored){
    (void)ignored;
    assert(payload==0 && writes==0);
    ++writes; payload=epoch*431u;
}
static int core1(void *unused){
    (void)unused;
    for(uint32_t n=1;n<=RUNS;++n){
        while((atomic_load_explicit(&request,memory_order_acquire)>>2)!=n)
            thrd_yield();
        assert(epoch==n);
        join_work(&request,&reply,n,render_only,NULL);
        atomic_store_explicit(&drained,n,memory_order_release); /* model platform DONE */
        while(atomic_load_explicit(&retired,memory_order_acquire)!=n)
            thrd_yield();
    }
    return 0;
}
int main(void){
    thrd_t worker; int result;
    assert(thrd_create(&worker,core1,NULL)==thrd_success);
    unsigned declined=0,completed=0;
    for(uint32_t n=1;n<=RUNS;++n){
        epoch=n; payload=writes=0;
        assert(join_issue(&request,&reply,n));
        if(n&1u) join_cancel(&request,n);
        enum join_result outcome;
        while((outcome=join_poll(&reply,n))==HOLD_OWNER) thrd_yield();
        if(outcome==RENDER_LOCAL){
            assert(n&1u);
            ++declined; render_only(NULL);
        } else { assert(outcome==USE_REMOTE); ++completed; }
        assert(payload==n*431u && writes==1);
        while(atomic_load_explicit(&drained,memory_order_acquire)!=n)
            thrd_yield();
        atomic_store_explicit(&retired,n,memory_order_release);
    }
    assert(thrd_join(worker,&result)==thrd_success && !result);
    assert(declined+completed==RUNS && completed>=RUNS/2);
    printf("Concurrent cancellation and completion: %u completed + %u declined, all exactly once\n",completed,declined);
}
