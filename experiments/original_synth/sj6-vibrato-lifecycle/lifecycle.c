#include "lifecycle.h"
#include <assert.h>
unsigned note_or_tick(sj6_vib_lifecycle *s,unsigned held_before,double tick,unsigned note) {
    return note?sj6_vib_key(s,held_before):sj6_vib_elapsed(s,tick);
}
#ifdef TEST
int main(void){
 sj6_vib_lifecycle x={.delay=.8513021875710888,.elapsed=1.25,.finished=1,.enabled=1};
 assert(!sj6_vib_key(&x,1)&&x.finished==1&&x.elapsed==1.25);
 assert(sj6_vib_key(&x,0)==SJ6_VIB_ZERO_SENDS&&x.elapsed==0&&x.finished==0);
 assert(!sj6_vib_elapsed(&x,.5));
 x=(sj6_vib_lifecycle){.delay=.25,.enabled=1};
 assert(!sj6_vib_elapsed(&x,.25)&&!x.finished);
 assert(sj6_vib_elapsed(&x,.001)==SJ6_VIB_RESTART&&x.finished);
 assert(!sj6_vib_elapsed(&x,2));
 x=(sj6_vib_lifecycle){.delay=.19,.enabled=1};
 assert(!sj6_vib_elapsed(&x,.3)&&x.finished);
 x=(sj6_vib_lifecycle){.delay=.009,.enabled=1};
 assert(!sj6_vib_key(&x,0)&&!x.lfo1_retrigger);
 x=(sj6_vib_lifecycle){.delay=.01,.enabled=1};
 assert(sj6_vib_key(&x,0)==SJ6_VIB_ZERO_SENDS);
 x=(sj6_vib_lifecycle){.delay=.2,.enabled=1};
 assert(sj6_vib_elapsed(&x,.2001)==SJ6_VIB_RESTART);
 x=(sj6_vib_lifecycle){.delay=.8513,.elapsed=1.25,.finished=1};
 assert(!sj6_vib_key(&x,0)&&!sj6_vib_elapsed(&x,1.)&&x.elapsed==1.25&&x.finished);
 return 0;
}
#endif
