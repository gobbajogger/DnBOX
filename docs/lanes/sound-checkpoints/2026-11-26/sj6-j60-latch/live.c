#include "juno6.h"
#include "fastmath.h"
#include <string.h>
#include <math.h>
#include "wet.inc"
__attribute__((visibility("default"))) int live_guard(unsigned latched) {
    juno6_chorus_t c={0};
    c.sample_rate=96000.f;
    c.factory_mode=JUNO6_CHORUS_I;
    const uint8_t b[4]={1,0,0,128};
    juno6_j60_config(&c,b);
    sj6_j60_t *s=&c.j60;
    unsigned changes=0, driven=0;
    for(unsigned i=0;i<12000;i++) {
        if(i==4096)s->target[2]=.22f;
        if(i==8192)s->target[2]=.78f;
        const float in[2]={.45f*sinf(.030f*i),.32f*cosf(.019f*i)};
        const float mix=s->mix;
        const float prior[2]={s->post1[0],s->post1[1]};
        j60_core(&c,s,1,in);
        if(i>=4096 && i<8192)changes+=mix!=s->mix;
        for(unsigned ch=0;ch<2;ch++){
            const float expected=latched?1.17f*mix*prior[ch]:s->post1[ch];
            if(memcmp(&expected,&s->wet[ch],sizeof expected))return -1;
            if(i>=4096 && expected!=0.f && (latched?1.17f*s->mix*s->post1[ch]:prior[ch])!=expected)++driven;
        }
    }
    if(!changes||!driven)return -2;
    c.factory_mode=JUNO6_CHORUS_OFF;
    float left[2]={-.3f,.2f},right[2]={.7f,-.1f};
    juno6_j60_process(&c,left,right,2);
    if(left[0]!=-.3f||left[1]!=.2f||right[0]!=.7f||right[1]!=-.1f||!s->reset)return -3;
    return driven;
}
