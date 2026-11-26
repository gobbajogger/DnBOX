#include "juno6.h"
#include <assert.h>
/* Calls exact compiled compact middle at 96k where its host-rate bridge
 * has ratio 1 and therefore consumes the entered PCM without interpolation. */
__attribute__((visibility("default"))) void witness_middle(float *l,float *r,unsigned n) {
    juno6_chorus_t c;
    juno6_chorus_init(&c,96000.f);
    c.factory_mode=JUNO6_CHORUS_I;
    uint8_t b[4]={1,0,0,128};
    juno6_j60_config(&c,b);
    for(unsigned i=0;i<n;i+=128)juno6_j60_process(&c,l+i,r+i,n-i>128?128:n-i);
}
