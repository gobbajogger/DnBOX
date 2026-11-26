#include "event_commit.c"
#include "cases.h"
#include "retargets.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
 for(unsigned i=0;i<sizeof cases/sizeof *cases;i++){
    sj6_event_clock s={0};assert(!sj6_event_retarget(&s,55));
    assert(sj6_event_offer(&s,cases[i].anchor,27,55));
    for(unsigned f=cases[i].begin;f<cases[i].end;f+=16)assert(sj6_event_prep(&s)==-1);
    assert(sj6_event_prep(&s)==55 && sj6_event_prep(&s)==-1);
    assert(sj6_event_offer(&s,cases[i].anchor,27,55));sj6_event_cancel(&s);
    assert(sj6_event_prep(&s)==-1);
 }
 sj6_event_clock s={0};
 assert(sj6_event_offer(&s,reference_retarget[0].y,27,48));
 unsigned i=1;
 for(unsigned f=33075u;f<=33667u;f+=16){
    if(i<3&&f==reference_retarget[i].f){
        assert(sj6_event_retarget(&s,i==1?52:55));
        assert(s.start==reference_retarget[i].y);
        ++i;
    }
    assert(sj6_event_prep(&s)==(f==33667?55:-1));
 }
 assert(i==3);
 puts("Exact all four native crossings; two actual staggered retarget anchors reconstructed bitwise, no wrong-owner emission; prep integer only.");
}
