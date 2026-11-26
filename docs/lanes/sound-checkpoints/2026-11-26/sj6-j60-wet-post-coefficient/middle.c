#include "juno6.h"
#include "fastmath.h"
#include <math.h>
#include <string.h>
#include "wet.inc"
__attribute__((visibility("default"))) void witness_middle(float *l,float *r,unsigned n) {
    juno6_chorus_t c={0};
    c.sample_rate=96000.f;
    c.factory_mode=JUNO6_CHORUS_I;
    uint8_t b[4]={1,0,0,128};
    juno6_j60_config(&c,b);
    sj6_j60_t *s=&c.j60;
    for(unsigned i=0;i<n;i++){
        const float x[2]={l[i],r[i]};
        j60_core(&c,s,1,x);
        l[i]=(float)(1.17*(double)s->wet[0]);
        r[i]=(float)(1.17*(double)s->wet[1]);
    }
}
/* Real public compact rate bridge, with the same caller block chronology. */
__attribute__((visibility("default"))) void witness_connected(float *l,float *r,const unsigned *parts,unsigned n) {
    juno6_chorus_t c={0};
    c.sample_rate=44100.f;
    c.factory_mode=JUNO6_CHORUS_I;
    uint8_t b[4]={1,0,0,128};
    juno6_j60_config(&c,b);
    for(unsigned i=0,at=0;i<n;at+=parts[i++])
      juno6_j60_process(&c,l+at,r+at,parts[i]);
}
