#include <stdint.h>
#include <string.h>
#include "replay.inc"
__attribute__((visibility("default"))) unsigned state_bytes(void){return sizeof(OTA);}
static OTA state[2];
__attribute__((visibility("default"))) void reset_owner(unsigned i) {memset(state+i,0,sizeof *state);}
__attribute__((visibility("default"))) float tick_owner(unsigned i,float x,const float*p,const float*c) {return physical_tick(state+i,x,p,c);}
__attribute__((visibility("default"))) void end_state(unsigned i,float *out){
#ifdef TEST_CALLS
 const OTA *s=state+i;
 out[0]=s->up.input;
#else
 const OTA *s=state+i;
 out[0]=s->up[0][0][0];
#endif
 for(unsigned j=0;j<4;j++)out[j+1]=s->pole[j];
 out[5]=s->condition;
}
