/* SPDX-License-Identifier: GPL-3.0-only */
#include "join.h"
#include <assert.h>
#include <stdio.h>
struct owner {
    join_word request, reply;
    unsigned writes, payload, cancel_during;
    uint32_t generation;
};
static void render_only(void *raw) {
    struct owner *o=raw;
    assert(join_poll(&o->reply,o->generation)==HOLD_OWNER);
    ++o->writes;
    if(o->cancel_during) {
        join_cancel(&o->request,o->generation);
        /* This is an acknowledged delay, not authority to duplicate. */
        assert(join_poll(&o->reply,o->generation)==HOLD_OWNER);
        assert(!join_issue(&o->request,&o->reply,o->generation+1));
    }
    o->payload=o->generation*37u;
}
int main(void){
    struct owner o={0};
    for(uint32_t i=1;i<=8000;++i){
        o.generation=i;
        o.writes=o.payload=0;
        o.cancel_during=i%4==1;
        assert(join_issue(&o.request,&o.reply,i));
        assert(join_poll(&o.reply,i)==HOLD_OWNER);
        assert(!join_issue(&o.request,&o.reply,i+1));
        join_work(&o.request,&o.reply,i-1,render_only,&o);
        assert(o.writes==0);
        if(i%4==2) join_cancel(&o.request,i);
        assert(join_poll(&o.reply,i)==HOLD_OWNER);
        join_work(&o.request,&o.reply,i,render_only,&o);
        if(i%4==3) join_cancel(&o.request,i);
        enum join_result outcome=join_poll(&o.reply,i);
        if(outcome==RENDER_LOCAL){
            assert(i%4==2 && o.writes==0);
            ++o.writes; /* local executes once only after no-mutation acknowledgment */
            o.payload=i*37u;
        } else assert(outcome==USE_REMOTE && i%4!=2);
        join_work(&o.request,&o.reply,i,render_only,&o);
        assert(o.writes==1 && o.payload==i*37u);
        assert(join_poll(&o.reply,i+1)==HOLD_OWNER);
    }
    assert(!join_issue(&o.request,&o.reply,1u<<30));
    puts("8000 dispatches: stale, denied, canceled-before, canceled-during, canceled-after, and no duplicate render: PASS");
}
