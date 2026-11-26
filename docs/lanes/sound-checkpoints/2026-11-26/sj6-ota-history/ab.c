#include <stdint.h>
#include <string.h>
#include "replay.inc"
__attribute__((visibility("default"))) unsigned state_bytes(void){return sizeof(OTA);}
static OTA state[2];
__attribute__((visibility("default"))) void reset_owner(unsigned i) {memset(state+i,0,sizeof *state);}
__attribute__((visibility("default"))) float tick_owner(unsigned i,float x,const float*p,const float*c) {return physical_tick(state+i,x,p,c);}
