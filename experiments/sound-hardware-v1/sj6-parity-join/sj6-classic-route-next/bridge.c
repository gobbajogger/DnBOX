#include <string.h>
#include <stdlib.h>
#include "juno6.h"
#include "fastmath.h"
#define TWO_PI 6.28318530717958647692f
#include "juno6_classic_native.inc"
juno6_chorus_t *create(unsigned mode){
 juno6_chorus_t *c=calloc(1,sizeof *c);
 c->sample_rate=44100.0f;
 c->factory_mode=mode;
 const uint8_t code[3]={0};
 juno6_classic_native_config(c,code);
 return c;
}
