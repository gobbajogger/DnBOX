/* SPDX-License-Identifier: GPL-3.0-only */
#include "mb_join.h"
#include <assert.h>
#include <stdio.h>
#include <threads.h>
enum { RUNS=25000 };
static join_word ready,done,cancel,declined,retired;
static uint32_t reply,writes,epoch;
static void render(void){
    assert(!reply && !writes);
    reply=epoch^0x53257352u;
    ++writes;
}
static int core1(void *unused){
    (void)unused;
    for(uint32_t g=1;g<=RUNS;++g){
        while(atomic_load_explicit(&ready,memory_order_acquire)!=g) thrd_yield();
        assert(epoch==g);
        if(!mb_decline(&cancel,&declined,g)) render();
        atomic_store_explicit(&done,g,memory_order_release);
        while(atomic_load_explicit(&retired,memory_order_acquire)!=g) thrd_yield();
    }
    return 0;
}
int main(void){
    join_word pending_cancel=4, early_decline=0, previous_done=3;
    assert(mb_decline(&pending_cancel,&early_decline,4));
    assert(mb_poll(&previous_done,&early_decline,4)==HOLD_OWNER);
    atomic_store_explicit(&previous_done,4,memory_order_release);
    assert(mb_poll(&previous_done,&early_decline,4)==RENDER_LOCAL);
    thrd_t worker; int result;
    assert(thrd_create(&worker,core1,NULL)==thrd_success);
    unsigned canceled=0;
    for(uint32_t g=1;g<=RUNS;++g){
        epoch=g; reply=writes=0;
        assert(mb_poll(&done,&declined,g)==HOLD_OWNER);
        atomic_store_explicit(&ready,g,memory_order_release);
        if(g&1u) atomic_store_explicit(&cancel,g,memory_order_release);
        enum join_result outcome;
        while((outcome=mb_poll(&done,&declined,g))==HOLD_OWNER) thrd_yield();
        if(outcome==RENDER_LOCAL){assert(g&1u); ++canceled; render();}
        assert(reply==(g^0x53257352u) && writes==1);
        atomic_store_explicit(&retired,g,memory_order_release);
    }
    assert(thrd_join(worker,&result)==thrd_success && !result);
    printf("Existing-DONE contract: %u canceled + %u completed, once; decline without DONE is not admission\n",canceled,RUNS-canceled);
}
