#include "engine.h"
#include "original_sound.h"
#include "s1_native_phaser.h"
#define CANONICAL137_IMPL 1
#include "sj6_137_host_witness.h"
#include "converter.h"
#include <assert.h>
#include <stdio.h>
unsigned original_sound_s1_program(const uint8_t b[ORIGINAL_SOUND_BYTES]){(void)b; return 0;}
int juno6_aux_destination_supported(unsigned id){return id==0;}
const float x0x_tanh_raw[1026]={0};
static sound_t s;
int main(void){
 for(unsigned i=0;i<2;i++){s.v[T_303][i][29]=ORIGINAL_SOUND_JUNO;slot137(&s,i)[0]=137;slot137(&s,i)[1]=i;}
 assert(canonical137(&s));
 for(unsigned owner=0;owner<2;owner++){
  for(unsigned q=0;q<256;q++){
    slot137(&s,owner)[31]=(uint8_t)q;
#ifdef SJ6_RATE_CARRIER_WITNESS
    assert(canonical137(&s)==(q<=128));
    if(q && q<=128){float percent;assert(sj6_crush_percent(109,q-1u,unit_control(q-1u),&percent));}
#else
    assert(canonical137(&s)==(q==0));
#endif
    slot137(&s,owner)[31]=0;
    s.v[T_MST][3u+owner][26]=(uint8_t)q;
    assert(canonical137(&s)); /* actual opaque byte collision */
    s.v[T_MST][3u+owner][26]=0;
  }
  s.v[T_303][owner][29]=ORIGINAL_SOUND_SYNTHONE;
  slot137(&s,owner)[31]=1;
  assert(!canonical137(&s));
  slot137(&s,owner)[31]=0;
  s.v[T_303][owner][29]=ORIGINAL_SOUND_JUNO;
 }
 puts("actual canonical137: opaque master collision; both typed owner bytes and old/new boundaries pass");
}
